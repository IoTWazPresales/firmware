#ifndef WiFiSettingsService_h
#define WiFiSettingsService_h

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <IPUtils.h>
#include <JsonUtils.h>

#define WIFI_SETTINGS_FILE "/config/wifiSettings.json"
#define WIFI_SETTINGS_SERVICE_PATH "/api/wifiSettings"
#define WIFI_RECONNECTION_DELAY 1000 * 30

class WiFiSettingsService {
 public:

  WiFiSettingsService(AsyncWebServer* server);
  void begin();
  void loop();

 private:
   bool _isScanning = false; // Add this line
  struct WiFiSettings {
    String ssid;
    String password;
    String hostname;
    bool staticIPConfig;
    IPAddress localIP;
    IPAddress gatewayIP;
    IPAddress subnetMask;
    IPAddress dnsIP1;
    IPAddress dnsIP2;

    // Load settings from JSON
    void fromJson(JsonObject& root);
    // Save settings to JSON
    void toJson(JsonObject& root) const;
  };

  WiFiSettings _settings;
  AsyncWebServer* _server;
  unsigned long _lastConnectionAttempt;
  bool _stopping;
  static void onStationModeDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
  static void onStationModeStop(WiFiEvent_t event, WiFiEventInfo_t info);
  void loadSettings();
  void saveSettings();
  void reconfigureWiFiConnection();
  void manageSTA();
  void handleSettingsRequest(AsyncWebServerRequest* request);
};

#endif  // end WiFiSettingsService_h
