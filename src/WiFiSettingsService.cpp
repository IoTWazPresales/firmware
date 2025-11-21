#include "WiFiSettingsService.h"
#include "WiFiCredentials.h"
#include "WiFiStatus.h"
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

  loadSettings();
}

void WiFiSettingsService::begin() {
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
  _settings.ssid = "9532828 [2Ghz]";    // Your network SSID
  _settings.password = "0611401627";    // Your network password
  _settings.hostname = "NeuroGrow";   // Device hostname
  _settings.staticIPConfig = false;      // Use DHCP
  Serial.println("Loaded SSID: " + _settings.ssid);
  Serial.println("Loaded Password: " + _settings.password);
  // Optional: Add static IP settings if needed later
  // _settings.localIP = IPAddress(192, 168, 1, 100);
  // _settings.gatewayIP = IPAddress(192, 168, 1, 1);
  // _settings.subnetMask = IPAddress(255, 255, 255, 0);
  // _settings.dnsIP1 = IPAddress(8, 8, 8, 8);
  // _settings.dnsIP2 = IPAddress(8, 8, 4, 4);  // Load WiFi settings from file (pseudo-code for file operations)
}

void WiFiSettingsService::saveSettings() {
  // Save WiFi settings to file (pseudo-code for file operations)
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
  Serial.println("manageSTA called");
  if (WiFi.isConnected()) {
    Serial.println("Already connected");
    return;
  }
  if (_settings.ssid.isEmpty()) {
    Serial.println("SSID is empty");
    return;
  }

  WiFi.mode(WIFI_STA);
  Serial.println("Set WiFi mode to STA");
  Serial.println("Connecting to WiFi: " + _settings.ssid);
  WiFi.begin(_settings.ssid.c_str(), _settings.password.c_str());
  Serial.println("WiFi.begin called");
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
