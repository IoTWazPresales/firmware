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
    delay(2000);
    Serial.println("Boot start");
    Serial.flush();
    
    Logger::setLevel(LogLevel::INFO);
    Logger::info("Firmware starting");
    
    Serial.println("ErrorRecovery init");
    Serial.flush();
    ErrorRecovery::restoreCriticalState();
    ErrorRecovery::saveCriticalState();
    
    Serial.println("ESP32React init");
    Serial.flush();
    // esp32React.begin() is called later after LittleFS is mounted (Preferences needs it)
    
    Serial.println("Watchdog init");
    Serial.flush();
    esp_task_wdt_init(30, /* panic */ false);
    
    Serial.println("LittleFS mount");
    Serial.flush();
    bool littlefsMounted = LittleFS.begin(true);
    if (!littlefsMounted) {
        Serial.println("LittleFS failed");
        Serial.flush();
    } else {
        Serial.println("LittleFS OK");
        Serial.flush();
    }
        
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
        
        // Ensure directories exist
        if (!LittleFS.exists("/manifests")) {
            File test = LittleFS.open("/manifests/.keep", "w");
            if (test) { test.close(); LittleFS.remove("/manifests/.keep"); }
        }
        if (!LittleFS.exists("/drivers")) {
            File test = LittleFS.open("/drivers/.keep", "w");
            if (test) { test.close(); LittleFS.remove("/drivers/.keep"); }
        }
        if (!LittleFS.exists("/submissions")) {
            File test = LittleFS.open("/submissions/.keep", "w");
            if (test) { test.close(); LittleFS.remove("/submissions/.keep"); }
        }
        
        // Serve static files - map /static/* requests to root files
        // The build process puts files in root, but browser requests /static/*
        // Map /static/ to root of LittleFS so /static/main.b0b7b627.js -> /main.b0b7b627.js
        server.serveStatic("/static/", LittleFS, "/");
        
        // Serve root-level files directly
        server.serveStatic("/favicon.ico", LittleFS, "/favicon.ico");
        server.serveStatic("/manifest.json", LittleFS, "/manifest.json");
        server.serveStatic("/manifest12.json", LittleFS, "/manifest12.json");
        server.serveStatic("/robots.txt", LittleFS, "/robots.txt");
        server.serveStatic("/logo192.png", LittleFS, "/logo192.png");
        server.serveStatic("/FissionLogo_SkyBlue.png", LittleFS, "/FissionLogo_SkyBlue.png");
        
        // Fallback: serve JS/CSS files from root if requested from root (without /static/)
        server.serveStatic("/*.js", LittleFS, "/", "text/javascript");
        server.serveStatic("/*.css", LittleFS, "/", "text/css");
        server.serveStatic("/*.png", LittleFS, "/", "image/png");
        server.serveStatic("/*.jpg", LittleFS, "/", "image/jpeg");
        server.serveStatic("/*.ico", LittleFS, "/", "image/x-icon");

        if (!LittleFS.exists("/index.html")) {
            Serial.println("⚠️ index.html missing from filesystem");
        }
    
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
    // Fallback for 404: if it's an API route, return 404; else serve index.html for SPA routing
    server.onNotFound([](AsyncWebServerRequest *request){
        String url = request->url();
        if (url.startsWith("/api/")) {
            request->send(404, "application/json", "{\"status\":\"error\",\"message\":\"Not found\"}");
        } else {
            // SPA fallback
            request->send(LittleFS, "/index.html", "text/html");
        }
    });

    Serial.println("WiFi init");
    Serial.flush();
    WiFi.mode(WIFI_AP_STA);
    delay(100);
    WiFi.softAP("NeuroGrow-Setup", "setup12345678", 1, 0, 4);
    delay(500);
    IPAddress apIP(192, 168, 4, 1);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    delay(500);
    Serial.println("AP started");
    Serial.flush();

    Serial.println("WiFiScanner init");
    Serial.flush();
    wifiScanner.begin();
    wifiScanner.preScanNetworks();
    
    Serial.println("WiFiSettingsService init");
    Serial.flush();
    wifiSettingsService.begin();
    
    Serial.println("Server begin");
    Serial.flush();
    server.begin();
    Serial.println("Server started");
    Serial.flush();
   
    Serial.println("TaskManager begin");
    Serial.flush();
    taskManager.begin();
    Serial.println("TaskManager started");
    Serial.flush();
    
    Serial.println("Creating dataCollector");
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
    Serial.println("Creating dataLogger");
    Serial.flush();
    dataLogger = new DataLogger(&fileSystem, dataCollector, &server);
    
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
        try {
            sensorManager.autoDetectSensors(&scanner);
            sensorManager.loadConfig(); // Reload after auto-detection
        } catch (...) {
            ErrorRecovery::logError(ErrorSeverity::WARNING, "SensorManager", "Auto-detection failed, continuing with empty config");
        }
    }
    
    Serial.println("Device ready");
    
    webSocketService.begin();
    sensorSubmissionService.begin();
    driverPackageService.begin();  // Initialize after LittleFS is mounted
    WirelessSensorManager::begin();
    
    Serial.println("Setup complete");
    Serial.flush();
}

void loop() {
    esp32React.loop();
    webSocketService.loop();
    WirelessSensorManager::loop();
    wifiSettingsService.loop();  // Handle WiFi connection/AP mode
    
    // Periodic state save (every 5 minutes)
    static unsigned long lastStateSave = 0;
    if (millis() - lastStateSave >= 300000) {
        ErrorRecovery::saveCriticalState();
        lastStateSave = millis();
    }
    
    vTaskDelay(pdMS_TO_TICKS(25));
}
