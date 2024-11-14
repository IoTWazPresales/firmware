#include <WiFiScanner.h>

  bool _isScanning = false; // Add this as a member variable
WiFiScanner::WiFiScanner(AsyncWebServer* server) {
  // Corrected lambda function syntax
  server->on(SCAN_NETWORKS_SERVICE_PATH, HTTP_GET, [this](AsyncWebServerRequest *request) {
    scanNetworks(request);
  });  // <-- closing parenthesis for this lambda function

  // Corrected lambda function syntax
  server->on(LIST_NETWORKS_SERVICE_PATH, HTTP_GET, [this](AsyncWebServerRequest *request) {
    listNetworks(request);
  });  // <-- closing parenthesis for this lambda function
}

void WiFiScanner::scanNetworks(AsyncWebServerRequest* request) {

_isScanning = true;

  if (WiFi.scanComplete() != -1) {
    WiFi.scanDelete();
    WiFi.scanNetworks(true);
    _isScanning = false;
  }
  request->send(202);
}

void WiFiScanner::listNetworks(AsyncWebServerRequest* request) {
  int numNetworks = WiFi.scanComplete();
  if (numNetworks > -1) {
    AsyncJsonResponse* response = new AsyncJsonResponse(false, MAX_WIFI_SCANNER_SIZE);
    JsonObject root = response->getRoot();
    JsonArray networks = root.createNestedArray("networks");
    for (int i = 0; i < numNetworks; i++) {
      JsonObject network = networks.createNestedObject();
      network["rssi"] = WiFi.RSSI(i);
      network["ssid"] = WiFi.SSID(i);
      network["bssid"] = WiFi.BSSIDstr(i);
      network["channel"] = WiFi.channel(i);
      network["encryption_type"] = (uint8_t)WiFi.encryptionType(i);
    }
    response->setLength();
    request->send(response);
  } else if (numNetworks == -1) {
    request->send(202);
  } else {
    scanNetworks(request);
  }
}
