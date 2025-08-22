#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include "MQTTConnector.h"
#include <SPIFFS.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <LittleFS.h>
#include <esp_task_wdt.h>
#include "SupabaseConnector.h"
#include <ESPmDNS.h>
#include "SensorManager.h"
#include "DeviceScanner.h"
#include "ScannerEndpoints.h"
#include "SensorEndpoints.h"
#include "RelayControl.h"
#include "RelayControlEndpoint.h"
#include "WiFiStatus.h"
#include "WiFiScanner.h"
#include "WiFiSettingsService.h"
#include "SystemStatus.h"
#include "OTAHandler.h"
#include "SensorDataCollector.h"
#include "FileSystem.h"
#include "FactoryResetService.h"
#include "RestartService.h"
#include "DataLogger.h"
#include "ESP32React.h"
#include "SupabaseSync.h"
#include <Preferences.h>
#include "TaskManager.h"
#include "Connector_Task.h"


SupabaseConnector supabaseConnector;
MQTTConnector mqttConnector;
static ConnectorHandles connectorHandles = {
  .supabase = &supabaseConnector,
  .mqtt     = &mqttConnector
};
AsyncWebServer server(80);
ESP32React   esp32React(&server);
DeviceScanner       scanner(&server);
ScannerEndpoints    scannerEndpoints(&server, &scanner);
SensorManager       sensorManager(&server);
RelayControl        relayControl(&server, &sensorManager);
RelayControlEndpoint relayEndpoints(&server, &relayControl, &supabaseConnector);
TaskManager taskManager(&server, &sensorManager, &scanner, &relayControl, &connectorHandles);


// Other Services
WiFiSettingsService    wifiSettingsService(&server);
FileSystem             fileSystem(&server);
WiFiStatus             wifiStatus(&server);
WiFiScanner            wifiScanner(&server);
SystemStatus           systemStatus(&server);
FactoryResetService    factoryResetService(&server, &LittleFS);
SensorEndpoints        endpoints(&server, &sensorManager,&supabaseConnector);
SensorDataCollector    dataCollector(
    sensorManager.getFirstPH(),
    sensorManager.getFirstTemperatureSensor(),
    sensorManager.getFirstDHT11(),
    sensorManager.getFirstMoistureSensor(),
    sensorManager.getFirstRTC(),
    sensorManager.getFirstTDS(),
    sensorManager.getFirstAtmosphereSensor()
);
DataLogger             dataLogger(&fileSystem, &dataCollector, &server);
OTAHandler             otaHandler;


const char* otaPassword = "0611401627";


void clearSupabaseCredentials() {
  Preferences prefs;
  prefs.begin("supabase", false);
  prefs.remove("device_id");
  prefs.remove("api_key");
  prefs.end();
  Serial.println("🚮 Supabase credentials cleared!");
}
void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    delay(1000);
      // ─── TASK WATCHDOG ──────────────────────────────────────────────
    // 10-second timeout, panic=true will abort() on timeout
    esp_task_wdt_init(10, /* panic */ true);
    esp32React.begin();
    

    // ─── Mount LittleFS ───────────────────────────────────────────
    bool littlefsMounted = LittleFS.begin(true);
    if (!littlefsMounted) {
        Serial.println("LittleFS Mount Failed");
    } else {
        Serial.println("LittleFS Mounted Successfully");
        
        if (!LittleFS.exists("/config.json")) {
            fileSystem.writeFile(LittleFS, "/config.json", "{}");
            
        }
        if (!LittleFS.exists("/relays.json")) {
            fileSystem.writeFile(LittleFS, "/relays.json", "{\"relays\":[]}");
        }
        if (!LittleFS.exists("/thresholds.json")) {
            fileSystem.writeFile(LittleFS, "/thresholds.json", "{}");
        }
        if (!LittleFS.exists("/sensor_data.json")) {
            fileSystem.writeFile(LittleFS, "/sensor_data.json", "{}");
        }
        
        // 1) Serve only your JS/CSS folders statically:
        server.serveStatic("/static/js/",  LittleFS, "/static/js/");
        server.serveStatic("/static/css/", LittleFS, "/static/css/");

        // 2) Serve index.html at the root path
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(LittleFS, "/index.html", "text/html");
        });
    }
      
     
      delay(1000);
       
Wire.begin(21, 22);
    // Initialize relay controrelayControl.begin();  // no more setSensorManager
    
     
    
   
   

        // Cloud Registration Endpoint
   // NEW: Direct API key registration endpoint
    server.on("/connect", HTTP_POST, [](AsyncWebServerRequest *request) {
        String apiKey = "";
        if (request->hasArg("api_key")) {
            apiKey = request->arg("api_key");
        }
        
        Serial.println("=== DEVICE CONNECTION REQUEST ===");
        Serial.println("API key received: " + apiKey.substring(0, 15) + "...");
        
        if (apiKey.length() == 0) {
            Serial.println("❌ Connection failed: Missing API key");
            request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing API key\"}");
            return;
        }
        
        // Respond immediately to avoid timeout
        request->send(200, "application/json", "{\"status\":\"processing\",\"message\":\"Validating API key...\"}");
        request->send(200, "application/json", "{\"status\":\"processing\",\"message\":\"Validating API key and setting up MQTT...\"}");

        // Register with API key in background
         Serial.println("🚀 Starting hybrid connection process...");
        bool httpSuccess = supabaseConnector.registerWithApiKey(apiKey);
        
        if (httpSuccess) {
            Serial.println("✅ HTTP registration successful!");
            
            // Now set up MQTT with the same credentials
            String deviceId = "dev_" + WiFi.macAddress();
            deviceId.replace(":", "");
            
            mqttConnector.setDeviceCredentials(deviceId, apiKey);
            Serial.println("✅ MQTT credentials configured!");
            Serial.println("🎉 Hybrid connection established (HTTP + MQTT)");
        } else {
           Serial.println("❌ Hybrid connection failed");
        }
    });
    server.on("/disconnect", HTTP_POST, [](AsyncWebServerRequest *request) {
   Serial.println("=== HYBRID DEVICE DISCONNECT REQUEST ===");        
        // Clear stored credentials
        Preferences prefs;
        prefs.begin("supabase", false);
        prefs.remove("api_key");
        prefs.remove("device_id");
        prefs.end();
        
         prefs.begin("mqtt", false);
        prefs.remove("api_key");
        prefs.remove("device_id");
        prefs.end();
        
        Serial.println("✅ All stored credentials cleared");
        Serial.println("🔄 Device disconnected from cloud (HTTP + MQTT)");  
        
        request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"Device disconnected and credentials cleared\"}");
    });
    // Fallback for 404: if it’s an API route, return 404; else serve index.html for SPA routing
    server.onNotFound([](AsyncWebServerRequest *request){
        String url = request->url();
        if (url.startsWith("/api/")) {
            request->send(404, "application/json", "{\"status\":\"error\",\"message\":\"Not found\"}");
        } else {
            // SPA fallback
            request->send(LittleFS, "/index.html", "text/html");
        }
    });

    // Enable CORS preflight handling
    server.on("/*", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
        request->send(200);
    });
  
   
    taskManager.begin();
    Serial.println("=== HYBRID DEVICE READY ===");
    Serial.println("Device MAC: " + WiFi.macAddress());
    Serial.println("Local IP: " + WiFi.localIP().toString());
    Serial.println("💡 Communication modes:");
    Serial.println("   📡 HTTP: Device registration & fallback");
    Serial.println("   🔗 MQTT: Real-time sensor data & relay control");
    Serial.println("   🌉 Bridge: MQTT ↔ Supabase synchronization");
}

void loop() {


    //esp32React.loop();
    //sensorManager.loop();
   
}
