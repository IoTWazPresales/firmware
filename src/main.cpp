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
#include "TelemetryService.h"
#include "DriverPackageManager.h"
#include "DriverPackageService.h"
#include "ManifestService.h"
#include "AuthMiddleware.h"
#include "SensorDriverRegistry.h"
#include "Logger.h"
#include "WebSocketService.h"
#include "DeviceScanner.h"
#include "RateLimiter.h"
#include "RateLimitMiddleware.h"
#include "ErrorRecovery.h"
#include "NetworkResilience.h"
#include "SensorSubmissionService.h"
#include "WirelessSensorManager.h"


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
// dataCollector will be initialized in setup() after sensorManager.begin()
SensorDataCollector*   dataCollector = nullptr;
DataLogger*            dataLogger = nullptr;
OTAHandler             otaHandler;
TelemetryService       telemetryService(&server);
DriverPackageManager   driverPackageManager(&LittleFS);
DriverPackageService   driverPackageService(&server, &driverPackageManager, &LittleFS);
ManifestService        manifestService(&server, &sensorManager);
WebSocketService       webSocketService(&server, &sensorManager);
SensorSubmissionService sensorSubmissionService(&server);

// Rate limiting (60 requests per minute per IP)
RateLimiter            apiRateLimiter(60, 60000);
RateLimiter            sensorRateLimiter(120, 60000); // More lenient for sensor endpoints


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
    delay(2000);  // Give Serial time to initialize
    Serial.println("\n\n=== NEUROGROW BOOT ===");
    Serial.println("Serial initialized");
    Serial.flush();
    
    Serial.println("Starting Logger...");
    Logger::setLevel(LogLevel::INFO);
    Logger::info("🚀 Firmware starting...");
    Serial.flush();
    
    Serial.println("Restoring critical state...");
    ErrorRecovery::restoreCriticalState();
    Serial.println("Critical state restored");
    Serial.flush();
    
    Serial.println("Saving initial state...");
    ErrorRecovery::saveCriticalState();
    Serial.println("Initial state saved");
    Serial.flush();
    
    Serial.println("Initializing ESP32React...");
    esp32React.begin();
    Serial.println("ESP32React initialized");
    Serial.flush();
    

    // ─── TASK WATCHDOG ──────────────────────────────────────────────
    // Disable panic mode temporarily to debug boot loop
    // 30-second timeout, panic=false (just warn, don't restart)
    Serial.println("Initializing watchdog (non-panic mode for debugging)...");
    esp_task_wdt_init(30, /* panic */ false);
    Serial.println("Watchdog initialized");
    Serial.flush();
    
    // ─── Mount LittleFS ───────────────────────────────────────────
    Serial.println("Mounting LittleFS...");
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
        
        // Ensure /manifests directory exists
        if (!LittleFS.exists("/manifests")) {
            Serial.println("📁 Creating /manifests directory");
            // Create by opening a file in the directory (LittleFS creates parent dirs)
            File test = LittleFS.open("/manifests/.keep", "w");
            if (test) {
                test.close();
                LittleFS.remove("/manifests/.keep");
                Serial.println("✅ /manifests directory created");
            } else {
                Serial.println("⚠️ Failed to create /manifests directory");
            }
        }
        
        // Ensure /drivers directory exists
        if (!LittleFS.exists("/drivers")) {
            Serial.println("📁 Creating /drivers directory");
            File test = LittleFS.open("/drivers/.keep", "w");
            if (test) {
                test.close();
                LittleFS.remove("/drivers/.keep");
                Serial.println("✅ /drivers directory created");
            }
        }
        
        // Ensure /submissions directory exists
        if (!LittleFS.exists("/submissions")) {
            Serial.println("📁 Creating /submissions directory");
            File test = LittleFS.open("/submissions/.keep", "w");
            if (test) {
                test.close();
                LittleFS.remove("/submissions/.keep");
                Serial.println("✅ /submissions directory created");
            }
        }
        
        // 1) Serve only your JS/CSS folders statically:
        server.serveStatic("/static/js/",  LittleFS, "/static/js/");
        server.serveStatic("/static/css/", LittleFS, "/static/css/");

        if (!LittleFS.exists("/index.html")) {
            Serial.println("⚠️ index.html missing from filesystem");
        }
    }
      
     
      delay(1000);
       
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization, x-device-api-key, X-Device-Api-Key");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");

    // Cloud Registration Endpoint
    server.on("/connect", HTTP_POST, [](AsyncWebServerRequest *request) {
        String apiKey;
        if (request->hasArg("api_key")) {
            apiKey = request->arg("api_key");
        } else if (request->hasParam("api_key", true)) {
            apiKey = request->getParam("api_key", true)->value();
        }

        apiKey.trim();

        Serial.println("=== DEVICE CONNECTION REQUEST ===");
        if (apiKey.length() > 0) {
            Serial.println("API key received: " + apiKey.substring(0, 15) + "...");
        }

        if (apiKey.isEmpty()) {
            Serial.println("❌ Connection failed: Missing API key");
            request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing API key\"}");
            return;
        }

        if (!enqueueHybridConnection(apiKey)) {
            Serial.println("❌ Unable to queue hybrid connection request");
            request->send(503, "application/json", "{\"status\":\"error\",\"message\":\"Device busy, retry\"}");
            return;
        }

        request->send(202, "application/json", "{\"status\":\"queued\",\"message\":\"Connection request accepted\"}");
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

  
   
    Serial.println("Starting TaskManager...");
    Serial.flush();
    taskManager.begin();
    Serial.println("TaskManager started");
    Serial.flush();
    
    // Initialize SensorDataCollector and DataLogger after sensors are initialized
    Serial.println("Initializing SensorDataCollector...");
    Serial.flush();
    dataCollector = new SensorDataCollector(
        sensorManager.getFirstPH(),
        sensorManager.getFirstTemperatureSensor(),
        sensorManager.getFirstDHT11(),
        sensorManager.getFirstMoistureSensor(),
        sensorManager.getFirstRTC(),
        sensorManager.getFirstTDS(),
        sensorManager.getFirstAtmosphereSensor()
    );
    Serial.println("SensorDataCollector initialized");
    Serial.flush();
    
    Serial.println("Initializing DataLogger...");
    Serial.flush();
    dataLogger = new DataLogger(&fileSystem, dataCollector, &server);
    Serial.println("DataLogger initialized");
    Serial.flush();
    
    // Auto-detect sensors if config is empty
    DynamicJsonDocument cfg(512);
    File f = LittleFS.open("/config.json", "r");
    bool configEmpty = true;
    if (f && f.size() > 0) {
        deserializeJson(cfg, f);
        f.close();
        configEmpty = cfg.size() == 0;
    }
    
    if (configEmpty) {
        Serial.println("🔍 Config empty, attempting auto-detection...");
        try {
            sensorManager.autoDetectSensors(&scanner);
            sensorManager.loadConfig(); // Reload after auto-detection
        } catch (...) {
            ErrorRecovery::logError(ErrorSeverity::WARNING, "SensorManager", "Auto-detection failed, continuing with empty config");
        }
    }
    
    Serial.println("=== HYBRID DEVICE READY ===");
    Serial.println("Device MAC: " + WiFi.macAddress());
    Serial.println("Local IP: " + WiFi.localIP().toString());
    Serial.println("💡 Communication modes:");
    Serial.println("   📡 HTTP: Device registration & fallback");
    Serial.println("   🔗 MQTT: Real-time sensor data & relay control");
    Serial.println("   🌉 Bridge: MQTT ↔ Supabase synchronization");
    Serial.println("   ⚡ WebSocket: Real-time updates on /ws");
    
    Serial.println("Starting WebSocket service...");
    webSocketService.begin();
    Serial.println("WebSocket started");
    
    Serial.println("Starting Sensor Submission service...");
    sensorSubmissionService.begin();
    Serial.println("Sensor Submission started");
    
    Serial.println("Starting Wireless Sensor Manager...");
    WirelessSensorManager::begin();
    Serial.println("Wireless Sensor Manager started");
    
    Serial.println("\n=== SETUP COMPLETE ===");
    Serial.flush();
}

void loop() {
    esp32React.loop();
    webSocketService.loop();
    WirelessSensorManager::loop();
    
    // Periodic state save (every 5 minutes)
    static unsigned long lastStateSave = 0;
    if (millis() - lastStateSave >= 300000) {
        ErrorRecovery::saveCriticalState();
        lastStateSave = millis();
    }
    
    vTaskDelay(pdMS_TO_TICKS(25));
}
