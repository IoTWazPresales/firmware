#include <WiFiScanner.h>

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
  // Ensure WiFi is in a mode that supports scanning (AP_STA or STA)
  wifi_mode_t currentMode = WiFi.getMode();
  if (currentMode != WIFI_AP_STA && currentMode != WIFI_STA) {
    // Switch to AP_STA mode to allow scanning while keeping AP active
    WiFi.mode(WIFI_AP_STA);
    delay(100);  // Give WiFi time to switch modes
  }

  // Ensure STA is initialized (even if not connected)
  if (WiFi.status() == WL_NO_SSID_AVAIL || WiFi.status() == WL_IDLE_STATUS) {
    // STA is initialized but not connected - this is fine for scanning
  }

  // Check if a scan is already in progress
  int scanResult = WiFi.scanComplete();
  if (scanResult == -1) {
    // Scan already in progress, return 202 (Accepted)
    request->send(202, "application/json", "{\"status\":\"scanning\",\"message\":\"Scan already in progress\"}");
    return;
  }

  // Clear previous scan results if any
  if (scanResult != -2) {
    WiFi.scanDelete();
  }

  // Start new scan (async, non-blocking)
  int networksFound = WiFi.scanNetworks(true, true);  // async=true, show_hidden=true
  
  if (networksFound == -1) {
    // Scan failed to start
    _isScanning = false;
    request->send(500, "application/json", "{\"status\":\"error\",\"message\":\"Failed to start WiFi scan\"}");
    return;
  }

  _isScanning = false;
  request->send(202, "application/json", "{\"status\":\"scanning\",\"message\":\"Scan started\"}");
}

void WiFiScanner::listNetworks(AsyncWebServerRequest* request) {
  int numNetworks = WiFi.scanComplete();
  
  if (numNetworks > 0) {
    // Scan completed successfully, return results
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
    
    root["count"] = numNetworks;
    response->setLength();
    request->send(response);
  } else if (numNetworks == -1) {
    // Scan in progress
    request->send(202, "application/json", "{\"status\":\"scanning\",\"message\":\"Scan in progress\"}");
  } else if (numNetworks == 0) {
    // Scan completed but no networks found
    AsyncJsonResponse* response = new AsyncJsonResponse(false, 256);
    JsonObject root = response->getRoot();
    root["count"] = 0;
    JsonArray networks = root.createNestedArray("networks");
    response->setLength();
    request->send(response);
  } else if (numNetworks == -2) {
    // No scan has been started yet, trigger one
    scanNetworks(request);
  } else {
    // Unknown error
    request->send(500, "application/json", "{\"status\":\"error\",\"message\":\"Scan failed\"}");
  }
}
