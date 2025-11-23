#ifndef WiFiScanner_h
#define WiFiScanner_h

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>

#define SCAN_NETWORKS_SERVICE_PATH "/api/scanNetworks"
#define LIST_NETWORKS_SERVICE_PATH "/api/listNetworks"

#define MAX_WIFI_SCANNER_SIZE 1024

class WiFiScanner {
 public:
  WiFiScanner(AsyncWebServer* server);
  void begin();  // Call this after WiFi AP is started to store AP credentials
  void preScanNetworks();  // Pre-scan on startup (before clients connect)

 private:
  void scanNetworks(AsyncWebServerRequest* request);
  void listNetworks(AsyncWebServerRequest* request);
  void restartAP();  // Restart AP after scan to allow reconnection
  
  bool _hasCachedResults = false;
  int _cachedNetworkCount = 0;
  unsigned long _lastScanTime = 0;  // Track when scan was started
  String _apSSID;
  String _apPassword;
};

#endif  // end WiFiScanner_h
