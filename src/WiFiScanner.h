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

 private:
  void scanNetworks(AsyncWebServerRequest* request);
  void listNetworks(AsyncWebServerRequest* request);
  bool _isScanning = false;
};

#endif  // end WiFiScanner_h
