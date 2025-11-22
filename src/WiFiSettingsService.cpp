#include "WiFiSettingsService.h"
#include "WiFiCredentials.h"
#include "WiFiStatus.h"
#include <LittleFS.h>
WiFiSettingsService::WiFiSettingsService(AsyncWebServer* server) : _server(server), _lastConnectionAttempt(0) {
  // Disable WiFi persistence and auto-reconnect on startup
  WiFi.persistent(false);
  WiFi.setAutoReconnect(false);

  WiFi.mode(WIFI_MODE_NULL);
  WiFi.onEvent(onStationModeDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  WiFi.onEvent(onStationModeStop, ARDUINO_EVENT_WIFI_STA_STOP);

  // Set up HTTP route to handle WiFi settings requests
  _server->on(WIFI_SETTINGS_SERVICE_PATH, HTTP_GET, [this](AsyncWebServerRequest* request) {
    handleSettingsRequest(request);
  });

  // Don't load settings here - LittleFS might not be mounted yet
  // Load settings will be called in begin() after LittleFS is mounted
}

void WiFiSettingsService::begin() {
  // Load settings after LittleFS is mounted
  loadSettings();
  
  if (WiFi.status() != WL_CONNECTED) {
    reconfigureWiFiConnection();
  }
}

void WiFiSettingsService::loop() {

   if (_isScanning) return; 
  unsigned long currentMillis = millis();
  if (!_lastConnectionAttempt || (currentMillis - _lastConnectionAttempt) >= WIFI_RECONNECTION_DELAY) {
    _lastConnectionAttempt = currentMillis;
    manageSTA();
  }
  Serial.println("WiFi Status: " + String(WiFi.status()));
  delay(1000); // Add delay to avoid flooding Serial
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
  // Only try to load if LittleFS is mounted
  if (!LittleFS.begin(false)) {  // false = don't format if mount fails
    // LittleFS not mounted, use defaults
    _settings.ssid = "";
    _settings.password = "";
    _settings.hostname = "NeuroGrow";
    _settings.staticIPConfig = false;
    Serial.println("LittleFS not mounted - using defaults (AP mode)");
    return;
  }
  
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
  DynamicJsonDocument doc(1024);
  JsonObject root = doc.to<JsonObject>();
  _settings.toJson(root);
  
  File file = LittleFS.open(WIFI_SETTINGS_FILE, "w");
  if (file) {
    serializeJson(root, file);
    file.close();
    Serial.println("WiFi settings saved");
  } else {
    Serial.println("Failed to save WiFi settings");
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
    return;
  }
  
  // If SSID is empty or not configured, start AP mode for setup
  if (_settings.ssid.isEmpty()) {
    if (WiFi.getMode() != WIFI_AP && WiFi.getMode() != WIFI_AP_STA) {
      Serial.println("No WiFi credentials - Starting AP mode for setup");
      WiFi.mode(WIFI_AP_STA);
      
      String apSSID = _settings.hostname.isEmpty() ? "NeuroGrow-Setup" : _settings.hostname + "-Setup";
      bool apStarted = WiFi.softAP(apSSID.c_str(), "setup12345678", 1, 0, 4);  // SSID, password, channel, hidden, max_connections
      
      if (!apStarted) {
        Serial.println("ERROR: Failed to start AP mode!");
        return;
      }
      
      IPAddress apIP(192, 168, 4, 1);
      IPAddress gateway(192, 168, 4, 1);
      IPAddress subnet(255, 255, 255, 0);
      WiFi.softAPConfig(apIP, gateway, subnet);
      
      delay(100);  // Give AP time to initialize
      
      Serial.print("AP Mode started! IP: ");
      Serial.println(WiFi.softAPIP());
      Serial.print("AP SSID: ");
      Serial.println(apSSID);
      Serial.print("AP Password: setup12345678");
      Serial.println();
      Serial.println("Connect to this network and go to http://192.168.4.1");
      Serial.flush();
    }
    return;
  }

  // Try to connect to configured WiFi
  WiFi.mode(WIFI_AP_STA);  // Keep AP available as fallback
  Serial.println("Connecting to WiFi: " + _settings.ssid);
  WiFi.begin(_settings.ssid.c_str(), _settings.password.c_str());
  
  // Wait up to 10 seconds for connection
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < 10000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected! IP: ");
    Serial.println(WiFi.localIP());
    // Disable AP once connected
    WiFi.mode(WIFI_STA);
  } else {
    Serial.println("Connection failed - AP mode still available");
    // Keep AP mode active
  }
}

void WiFiSettingsService::handleSettingsRequest(AsyncWebServerRequest* request) {
  DynamicJsonDocument jsonDoc(1024);
  JsonObject root = jsonDoc.to<JsonObject>();
  _settings.toJson(root);
  String response;
  serializeJson(jsonDoc, response);
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
