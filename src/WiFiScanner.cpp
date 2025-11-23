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

void WiFiScanner::begin() {
  // Store AP credentials for restarting after scan
  _apSSID = WiFi.softAPSSID();
  if (_apSSID.length() == 0) {
    _apSSID = "NeuroGrow-Setup";  // Default fallback
  }
  _apPassword = "setup12345678";  // Default AP password (matches main.cpp)
}

void WiFiScanner::preScanNetworks() {
  // Pre-scan networks when AP starts (before clients connect)
  // This allows users to see networks immediately without disconnecting
  Serial.println("Pre-scanning WiFi networks on AP startup...");
  
  // Ensure we're in AP_STA mode
  if (WiFi.getMode() != WIFI_AP_STA) {
    WiFi.mode(WIFI_AP_STA);
    delay(100);
  }
  
  // Start scan (async)
  int result = WiFi.scanNetworks(true, true);
  if (result >= 0) {
    Serial.printf("Pre-scan started, will cache %d networks when complete\n", result);
    _hasCachedResults = false;  // Will be set to true when scan completes
  } else {
    Serial.println("Pre-scan failed to start");
  }
}

void WiFiScanner::restartAP() {
  // Restart AP with same credentials to allow client reconnection
  // Only restart if we're in AP mode (not connected to WiFi)
  if (WiFi.status() == WL_CONNECTED) {
    // Connected to WiFi, don't restart AP
    return;
  }
  
  Serial.println("Restarting AP after scan...");
  
  // Get current AP settings before disconnecting
  String currentSSID = WiFi.softAPSSID();
  if (currentSSID.length() == 0) {
    currentSSID = _apSSID.length() > 0 ? _apSSID : "NeuroGrow-Setup";
  }
  int currentChannel = WiFi.channel();
  if (currentChannel == 0) {
    currentChannel = 1;  // Default channel
  }
  
  // Disconnect AP
  WiFi.softAPdisconnect(true);
  delay(200);
  
  // Restart AP with same settings
  WiFi.mode(WIFI_AP_STA);
  delay(100);
  
  const char* password = (_apPassword.length() > 0) ? _apPassword.c_str() : "setup12345678";
  if (!WiFi.softAP(currentSSID.c_str(), password, currentChannel, 0, 4)) {
    Serial.println("ERROR: Failed to restart AP!");
    return;
  }
  
  // Restore AP IP config
  IPAddress apIP(192, 168, 4, 1);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  delay(500);
  
  Serial.print("AP restarted: ");
  Serial.println(WiFi.softAPIP());
}

void WiFiScanner::scanNetworks(AsyncWebServerRequest* request) {
  // Ensure WiFi is in a mode that supports scanning (AP_STA or STA)
  wifi_mode_t currentMode = WiFi.getMode();
  if (currentMode != WIFI_AP_STA && currentMode != WIFI_STA) {
    // Switch to AP_STA mode to allow scanning while keeping AP active
    WiFi.mode(WIFI_AP_STA);
    delay(100);  // Give WiFi time to switch modes
  }

  // Check if a scan is already in progress
  int scanResult = WiFi.scanComplete();
  if (scanResult == -1) {
    // Scan already in progress, return 202 (Accepted)
    request->send(202, "application/json", "{\"status\":\"scanning\",\"message\":\"Scan already in progress. AP will restart when complete - reconnect to see results.\"}");
    return;
  }

  // Clear previous scan results if any
  if (scanResult != -2) {
    WiFi.scanDelete();
  }

  // Start new scan (async, non-blocking)
  // Note: This will cause AP clients to disconnect temporarily
  // User clicked "Scan again" so they expect this
  _lastScanTime = millis();  // Mark that we started a new scan
  int networksFound = WiFi.scanNetworks(true, true);  // async=true, show_hidden=true
  
  if (networksFound == -1) {
    // Scan failed to start
    request->send(500, "application/json", "{\"status\":\"error\",\"message\":\"Failed to start WiFi scan\"}");
    return;
  }

  // Scan started - AP will restart when scan completes (handled in listNetworks)
  request->send(202, "application/json", "{\"status\":\"scanning\",\"message\":\"Scan started. You may be disconnected briefly. Reconnect to AP when scan completes.\"}");
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
    _hasCachedResults = true;
    _cachedNetworkCount = numNetworks;
    
    // Check if this scan just completed (within last 2 seconds of starting)
    // If so, restart AP to allow reconnection
    unsigned long timeSinceScan = millis() - _lastScanTime;
    if (timeSinceScan < 2000 && WiFi.getMode() == WIFI_AP_STA && WiFi.status() != WL_CONNECTED) {
      // Fresh scan just completed - restart AP
      restartAP();
    }
    
    response->setLength();
    request->send(response);
  } else if (numNetworks == -1) {
    // Scan in progress - return empty networks array so frontend can keep polling
    AsyncJsonResponse* response = new AsyncJsonResponse(false, 256);
    JsonObject root = response->getRoot();
    root["count"] = 0;
    root["scanning"] = true;
    JsonArray networks = root.createNestedArray("networks");
    response->setLength();
    request->send(response);
  } else if (numNetworks == 0) {
    // Scan completed but no networks found
    AsyncJsonResponse* response = new AsyncJsonResponse(false, 256);
    JsonObject root = response->getRoot();
    root["count"] = 0;
    JsonArray networks = root.createNestedArray("networks");
    _hasCachedResults = true;
    _cachedNetworkCount = 0;
    
    // Check if this scan just completed (within last 2 seconds of starting)
    unsigned long timeSinceScan = millis() - _lastScanTime;
    if (timeSinceScan < 2000 && WiFi.getMode() == WIFI_AP_STA && WiFi.status() != WL_CONNECTED) {
      // Fresh scan just completed - restart AP
      restartAP();
    }
    
    response->setLength();
    request->send(response);
  } else if (numNetworks == -2) {
    // No scan has been started yet or results were cleared
    AsyncJsonResponse* response = new AsyncJsonResponse(false, 256);
    JsonObject root = response->getRoot();
    root["count"] = 0;
    root["message"] = "No scan results available. Click 'Scan again' to scan networks.";
    JsonArray networks = root.createNestedArray("networks");
    response->setLength();
    request->send(response);
  } else {
    // Unknown error
    request->send(500, "application/json", "{\"status\":\"error\",\"message\":\"Scan failed\"}");
  }
}
