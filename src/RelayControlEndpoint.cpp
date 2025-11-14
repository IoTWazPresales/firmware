#include "RelayControlEndpoint.h"
#include "RelayControl.h"
#include "SupabaseConnector.h"
#include "SensorManager.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include "AuthMiddleware.h"

extern SensorManager sensorManager;

RelayControlEndpoint::RelayControlEndpoint(AsyncWebServer* server, RelayControl* relayControl, SupabaseConnector* supabase)
  : _server(server), _relayControl(relayControl), _supabase(supabase)
{
  handleRelayData();
}

static int stringPinToGPIO(const String& pin) {
  // If you sent "D18", strip the "D" and return 18.
  // If you sent "18", just parse it.
  if (pin.startsWith("D")) {
    return pin.substring(1).toInt();
  }
  return pin.toInt();
}

void RelayControlEndpoint::syncRelayStateToSupabase() {
    if (_supabase && _supabase->isConnected()) {
        _supabase->syncRelayStates(
            _relayControl->getWaterPumpState(),
            _relayControl->getIntakeFanState(),
            _relayControl->getExhaustFanState(),
            _relayControl->getLightsState()
        );
    }
}

void RelayControlEndpoint::handleRelayData() {
  //
  // 1) List available sensor parameters
  //
  _server->on("/api/sensors", HTTP_GET, [this](AsyncWebServerRequest* req){
    AsyncJsonResponse* response = new AsyncJsonResponse(false, 1024);
    JsonObject root = response->getRoot();
    JsonArray arr = root.createNestedArray("parameters");
    sensorManager.enumerateCapabilities([&](const SensorCapability& cap, const SensorSample& sample){
      if (cap.kind == SensorValueKind::Numeric) {
        JsonObject obj = arr.createNestedObject();
        obj["id"] = cap.id;
        obj["label"] = cap.label.length() ? cap.label : cap.id;
        obj["unit"] = cap.unit;
        obj["kind"] = "numeric";
      }
    });
    response->setLength();
    req->send(response);
  });

  //
  // 2) Live device states with Supabase sync
  //
  auto deviceStateHandler = [this](AsyncWebServerRequest* req){
    AsyncJsonResponse* response = new AsyncJsonResponse(false, 256);
    JsonObject root = response->getRoot();
    root["pumpState"]        = _relayControl->getWaterPumpState();
    root["intakeFanState"]   = _relayControl->getIntakeFanState();
    root["extractorFanState"]= _relayControl->getExhaustFanState();
    root["lightsState"]      = _relayControl->getLightsState();
    
    // Sync to Supabase if connected
    syncRelayStateToSupabase();
    
    response->setLength();
    req->send(response);
  };
  _server->on("/api/relay/GetDeviceStates",  HTTP_GET, deviceStateHandler);
  _server->on("/api/relay/getDeviceStates",  HTTP_GET, deviceStateHandler);

  //
  // 3) GET thresholds.json
  //
  _server->on("/api/relay/getThresholds", HTTP_GET, [this](AsyncWebServerRequest* req){
    File f = LittleFS.open("/thresholds.json", "r");
    AsyncJsonResponse* response = new AsyncJsonResponse(false, 512);
    JsonObject root = response->getRoot();
    if (f && f.size()>0) {
      DynamicJsonDocument d(512);
      deserializeJson(d, f);
      f.close();
      for (JsonPair kv : d.as<JsonObject>()) {
        root[kv.key()] = kv.value();
      }
    }
    response->setLength();
    req->send(response);
  });

  //
  // 4) GET merged relay+threshold config
  //
  _server->on("/api/relay/config", HTTP_GET, [this](AsyncWebServerRequest* req){
    // load relays.json
    File rf = LittleFS.open("/relays.json","r");
    DynamicJsonDocument rd(2048);
    if (rf && rf.size()>0) {
      deserializeJson(rd, rf);
      rf.close();
    } else {
      rd.createNestedArray("relays");
    }

    // load thresholds.json
    File tf = LittleFS.open("/thresholds.json","r");
    DynamicJsonDocument td(1024);
    if (tf && tf.size()>0) {
      deserializeJson(td, tf);
      tf.close();
    }

    // merge
    AsyncJsonResponse* response = new AsyncJsonResponse(false, 1536);
    JsonObject root = response->getRoot();
    JsonArray arr = root.createNestedArray("relays");
    for (JsonObject r : rd["relays"].as<JsonArray>()) {
      JsonObject o = arr.createNestedObject();
      const char* id = r["id"].as<const char*>();
      int pin = r["pin"].as<int>();
      const char* parameter = r["parameter"].as<const char*>();

      o["id"] = id;
      o["pin"] = pin;
      o["parameter"] = parameter;
      if (td.containsKey(id)) {
        o["min"] = td[id]["min"].as<float>();
        o["max"] = td[id]["max"].as<float>();
      } else {
        o["min"] = 0.0f;
        o["max"] = 100.0f;
      }
    }

    response->setLength();
    req->send(response);
  });

  //
  // 5) POST saveConfig
  //
  _server->on("/api/relay/saveConfig", HTTP_POST,
    [](AsyncWebServerRequest* req){
      if (!AuthMiddleware::checkApiKey(req)) {
        req->send(401, "application/json", "{\"status\":\"error\",\"message\":\"Unauthorized\"}");
        return;
      }
    },
    nullptr,
    [this](AsyncWebServerRequest* req,uint8_t* data,size_t len,size_t,size_t){
      String body((char*)data,len);
      Serial.println("saveConfig body: "+body);

      DynamicJsonDocument in(2048);
      if (deserializeJson(in, body)
          || !in.containsKey("relays")
          || !in["relays"].is<JsonArray>())
      {
        req->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid payload\"}");
        return;
      }

      // write relays.json
      {
        DynamicJsonDocument rd(2048);
        auto arr = rd.createNestedArray("relays");
        for (JsonObject o : in["relays"].as<JsonArray>()) {
          String id  = o["id"].as<String>();
          String ps  = o["pin"].as<String>();
          String pr  = o["parameter"].as<String>();
          int    pn  = stringPinToGPIO(ps);
          if (pn < 0) {
            req->send(400,"application/json","{\"status\":\"error\",\"message\":\"Bad pin\"}");
            return;
          }
          JsonObject out = arr.createNestedObject();
          out["id"]        = id;
          out["pin"]       = pn;
          out["parameter"] = pr;
        }
        File f = LittleFS.open("/relays.json","w");
        if (!f) { req->send(500,"application/json","{\"status\":\"error\"}"); return; }
        serializeJson(rd, f);
        f.close();
      }

      // write thresholds.json
      {
        DynamicJsonDocument td(1024);
        for (JsonObject o : in["relays"].as<JsonArray>()) {
          String id = o["id"].as<String>();
          float  mi = o["min"].as<float>();
          float  ma = o["max"].as<float>();
          JsonObject n = td.createNestedObject(id);
          n["min"] = mi;
          n["max"] = ma;
        }
        File f = LittleFS.open("/thresholds.json","w");
        if (!f) { req->send(500,"application/json","{\"status\":\"error\"}"); return; }
        serializeJson(td, f);
        f.close();
      }

      _relayControl->begin();
      req->send(200,"application/json","{\"status\":\"success\"}");
    }
  );

  //
  // NEW: Supabase device registration endpoint
  //
  _server->on("/api/device/register", HTTP_POST,
    [](AsyncWebServerRequest*){},
    nullptr,
    [this](AsyncWebServerRequest* req, uint8_t* data, size_t len, size_t, size_t){
      String body((char*)data, len);
      DynamicJsonDocument doc(512);
      
      if (deserializeJson(doc, body) || !doc.containsKey("apiKey")) {
        req->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid payload\"}");
        return;
      }
      
      String apiKey = doc["apiKey"].as<String>();
      if (_supabase) {
        _supabase->setDeviceApiKey(apiKey);
        req->send(200, "application/json", "{\"status\":\"success\",\"message\":\"Device registered with Supabase\"}");
      } else {
        req->send(500, "application/json", "{\"status\":\"error\",\"message\":\"Supabase not available\"}");
      }
    }
  );

  //
  // 6) Optional legacy & debug endpoints...
  //
}