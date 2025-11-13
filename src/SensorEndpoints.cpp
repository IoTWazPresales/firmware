#include "SensorEndpoints.h"
#include "SensorDataCollector.h"
#include "SupabaseConnector.h"

SensorEndpoints::SensorEndpoints(AsyncWebServer* server, SensorManager* manager, SupabaseConnector* supabase)
    : _server(server), _manager(manager), _supabase(supabase) {
    handleSensorData();
}

void SensorEndpoints::handleSensorData() {
    _server->on("/api/sensor", HTTP_GET, [this](AsyncWebServerRequest *request) {
        DynamicJsonDocument jsonDoc(4096);
        JsonObject values = jsonDoc.createNestedObject("values");
        JsonArray meta = jsonDoc.createNestedArray("meta");

        if (_manager) {
            _manager->fillValuesJson(values);
            _manager->describeCapabilities(meta);
            for (JsonPair kv : values) {
                jsonDoc[kv.key()] = kv.value();
            }
        }

        if (_supabase && _supabase->isConnected() && _manager) {
            DynamicJsonDocument numericDoc(2048);
            JsonObject numeric = numericDoc.to<JsonObject>();
            _manager->fillNumericJson(numeric);
            _supabase->syncSensorData(numericDoc);
        }

        String response;
        serializeJson(jsonDoc, response);
        request->send(200, "application/json", response);
    });

    _server->on("/api/sensor", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
        request->send(204);
    });
}
