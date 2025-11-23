#include "WiFiSettingsService.h"
#include "WiFiCredentials.h"
#include "WiFiStatus.h"
#include <LittleFS.h>
WiFiSettingsService::WiFiSettingsService(AsyncWebServer* server) : _server(server), _lastConnectionAttempt(0) {
  // Don't do anything in constructor - all initialization in begin()
  // Route registration moved to begin() to avoid issues during global construction
}

void WiFiSettingsService::begin() {
  // Set up HTTP route to handle WiFi settings requests
  _server->on(WIFI_SETTINGS_SERVICE_PATH, HTTP_GET, [this](AsyncWebServerRequest* request) {
    handleSettingsRequest(request);
  });
  
  // POST handler to save WiFi settings
  _server->on(WIFI_SETTINGS_SERVICE_PATH, HTTP_POST,
    [](AsyncWebServerRequest* request) {},  // No upload handler
    nullptr,
    [this](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
      handleSaveSettingsRequest(request, data, len);
    }
  );
  
  // CORS preflight
  _server->on(WIFI_SETTINGS_SERVICE_PATH, HTTP_OPTIONS, [](AsyncWebServerRequest* request) {
    request->send(204);
  });
  
  // WiFi is already initialized by main.cpp (AP mode)
  // Just load settings and set up event handlers
  WiFi.persistent(false);
  WiFi.setAutoReconnect(false);
  
  // Load settings after LittleFS is mounted
  loadSettings();
  
  // Set up WiFi event handlers
  WiFi.onEvent(onStationModeDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  WiFi.onEvent(onStationModeStop, ARDUINO_EVENT_WIFI_STA_STOP);
  
  // Initialize AP mode if no credentials, or start STA connection if credentials exist
  // manageSTA() will be called from loop() to handle connection
  if (!_settings.ssid.isEmpty()) {
    Serial.println("Saved WiFi credentials found: " + _settings.ssid);
    // Connection will be attempted in manageSTA() via loop()
  } else {
    Serial.println("No saved WiFi credentials - AP mode will start");
    // AP mode will be started in manageSTA() via loop()
  }
}

void WiFiSettingsService::loop() {
  if (_isScanning) return; 
  
  unsigned long currentMillis = millis();
  if (!_lastConnectionAttempt || (currentMillis - _lastConnectionAttempt) >= WIFI_RECONNECTION_DELAY) {
    _lastConnectionAttempt = currentMillis;
    manageSTA();
  }
  
  // Reduced Serial output - only log status changes
  static wl_status_t lastStatus = WL_NO_SHIELD;
  wl_status_t currentStatus = WiFi.status();
  if (currentStatus != lastStatus) {
    Serial.printf("WiFi Status: %d\n", currentStatus);
    lastStatus = currentStatus;
  }
  // Removed blocking delay(1000) - let RTOS handle scheduling
}

void WiFiSettingsService::WiFiSettings::fromJson(JsonObject& root) {
  ssid = root["ssid"] | ssid;
  password = root["password"] | password;
  hostname = root["hostname"] | WiFi.getHostname();
  staticIPConfig = root["static_ip_config"] | false;

  // Use JsonUtils to read IP addresses
  JsonUtils::readIP(root, "local_ip", localIP);
  JsonUtils::readIP(root, "gateway_ip", gatewayIP);
  JsonUtils::readIP(root, "subnet_mask", subnetMask);
  JsonUtils::readIP(root, "dns_ip_1", dnsIP1);
  JsonUtils::readIP(root, "dns_ip_2", dnsIP2);
}

void WiFiSettingsService::WiFiSettings::toJson(JsonObject& root) const {
  root["ssid"] = ssid;
  root["password"] = password;
  root["hostname"] = hostname;
  root["static_ip_config"] = staticIPConfig;
   JsonUtils::writeIP(root, "local_ip", localIP);
  JsonUtils::writeIP(root, "gateway_ip", gatewayIP);
  JsonUtils::writeIP(root, "subnet_mask", subnetMask);
  JsonUtils::writeIP(root, "dns_ip_1", dnsIP1);
  JsonUtils::writeIP(root, "dns_ip_2", dnsIP2);
}

void WiFiSettingsService::loadSettings() {
  // LittleFS should already be mounted by main.cpp, just check if file exists
  // Try to load from LittleFS
  if (LittleFS.exists(WIFI_SETTINGS_FILE)) {
    File file = LittleFS.open(WIFI_SETTINGS_FILE, "r");
    if (file && file.size() > 0) {
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, file);
      file.close();
      if (!error && doc.is<JsonObject>()) {
        JsonObject root = doc.as<JsonObject>();
        _settings.fromJson(root);
        Serial.println("WiFi settings loaded from file");
        return;
      }
    }
  }
  
  // Default: empty (will trigger AP mode)
  _settings.ssid = "";
  _settings.password = "";
  _settings.hostname = "NeuroGrow";
  _settings.staticIPConfig = false;
  Serial.println("No saved WiFi settings - AP mode will start");
}

void WiFiSettingsService::saveSettings() {
  // Ensure /config directory exists
  if (!LittleFS.exists("/config")) {
    File test = LittleFS.open("/config/.keep", "w");
    if (test) { test.close(); LittleFS.remove("/config/.keep"); }
  }
  
  DynamicJsonDocument doc(1024);
  JsonObject root = doc.to<JsonObject>();
  _settings.toJson(root);
  
  File file = LittleFS.open(WIFI_SETTINGS_FILE, "w");
  if (file) {
    serializeJson(root, file);
    file.close();
    Serial.println("WiFi settings saved");
  } else {
    Serial.println("Failed to save WiFi settings - file open failed");
  }
}

void WiFiSettingsService::reconfigureWiFiConnection() {
  // reset last connection attempt to force loop to reconnect immediately
  _lastConnectionAttempt = 0;

// disconnect and de-configure wifi
if (WiFi.status() == WL_CONNECTED) {
    if (WiFi.disconnect(true)) {
      _stopping = true;
      delay(1000);  // Add a small delay to avoid immediate reconnection attempts
    }
  }
}

void WiFiSettingsService::manageSTA() {
  if (WiFi.isConnected()) {
    // If connected, ensure AP is disabled to save power
    if (WiFi.getMode() != WIFI_STA) {
      WiFi.mode(WIFI_STA);
      Serial.println("WiFi connected - AP mode disabled");
    }
    return;
  }
  
  // If SSID is empty or not configured, start AP mode for setup
  if (_settings.ssid.isEmpty()) {
    if (WiFi.getMode() != WIFI_AP && WiFi.getMode() != WIFI_AP_STA) {
      Serial.println("No WiFi credentials - Starting AP mode for setup");
      WiFi.mode(WIFI_AP_STA);
      delay(100);
    }
    
    // Create AP name with MAC address identifier (last 4 digits)
    if (!WiFi.softAPgetStationNum()) {  // Only start if not already started
      String macAddr = WiFi.macAddress();
      macAddr.replace(":", "");
      String apSSID = "NeuroGrow-" + macAddr.substring(macAddr.length() - 4);
      
      // Start AP
      bool apStarted = WiFi.softAP(apSSID.c_str(), "setup12345678", 1, 0, 4);
      
      if (!apStarted) {
        Serial.println("ERROR: Failed to start AP mode!");
        return;
      }
      
      delay(200);  // Give AP time to initialize
      
      // Configure AP IP
      IPAddress apIP(192, 168, 4, 1);
      IPAddress gateway(192, 168, 4, 1);
      IPAddress subnet(255, 255, 255, 0);
      WiFi.softAPConfig(apIP, gateway, subnet);
      
      Serial.print("AP Mode started! IP: ");
      Serial.println(WiFi.softAPIP());
      Serial.print("AP SSID: ");
      Serial.println(apSSID);
      Serial.println("AP Password: setup12345678");
    }
    return;
  }

  // Try to connect to configured WiFi
  // Keep AP available as fallback during connection attempt
  if (WiFi.getMode() != WIFI_AP_STA) {
    WiFi.mode(WIFI_AP_STA);
    delay(100);
  }
  
  static unsigned long lastConnectAttempt = 0;
  unsigned long now = millis();
  if (now - lastConnectAttempt < 5000) {
    return;  // Don't attempt connection too frequently
  }
  lastConnectAttempt = now;
  
  Serial.println("Connecting to WiFi: " + _settings.ssid);
  WiFi.begin(_settings.ssid.c_str(), _settings.password.c_str());
  
  // Non-blocking connection check (will complete in loop)
  // Connection status will be checked on next loop iteration
}

void WiFiSettingsService::handleSettingsRequest(AsyncWebServerRequest* request) {
  DynamicJsonDocument jsonDoc(1024);
  JsonObject root = jsonDoc.to<JsonObject>();
  _settings.toJson(root);
  String response;
  serializeJson(jsonDoc, response);
  request->send(200, "application/json", response);
}

void WiFiSettingsService::handleSaveSettingsRequest(AsyncWebServerRequest* request, uint8_t* data, size_t len) {
  Serial.println("[WiFiSettingsService] Received POST request to save WiFi settings");
  
  DynamicJsonDocument jsonDoc(1024);
  DeserializationError error = deserializeJson(jsonDoc, (char*)data, len);
  
  if (error) {
    Serial.printf("[WiFiSettingsService] JSON parse error: %s\n", error.c_str());
    request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
    return;
  }
  
  if (!jsonDoc.is<JsonObject>()) {
    Serial.println("[WiFiSettingsService] Invalid JSON structure");
    request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON structure\"}");
    return;
  }
  
  JsonObject root = jsonDoc.as<JsonObject>();
  
  // Update settings from JSON
  _settings.fromJson(root);
  
  // Save to file
  saveSettings();
  
  // Reconfigure WiFi connection
  reconfigureWiFiConnection();
  
  // Force immediate connection attempt
  _lastConnectionAttempt = 0;
  
  Serial.println("[WiFiSettingsService] WiFi settings saved, reconnecting...");
  
  // Return updated settings
  DynamicJsonDocument responseDoc(1024);
  JsonObject responseRoot = responseDoc.to<JsonObject>();
  _settings.toJson(responseRoot);
  String response;
  serializeJson(responseDoc, response);
  request->send(200, "application/json", response);
}

#ifdef ESP32
void WiFiSettingsService::onStationModeDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  WiFi.disconnect(true);
}

void WiFiSettingsService::onStationModeStop(WiFiEvent_t event, WiFiEventInfo_t info) {
  
}
#elif defined(ESP8266)
void WiFiSettingsService::onStationModeDisconnected(const WiFiEventStationModeDisconnected& event) {
  WiFi.disconnect(true);
}
#endif
