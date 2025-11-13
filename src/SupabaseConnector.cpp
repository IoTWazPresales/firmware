
#include "SupabaseConnector.h"
#include "SensorManager.h"
#include "Connector_Task.h"

SupabaseConnector::SupabaseConnector() {
    _supabaseUrl = "https://hpywapfxlbbcjjhzcyqm.supabase.co";
    _lastSync = 0;
    _lastPoll = 0;
    _lastStatusUpdate = 0;
    _sensorManager = nullptr;
    _isRequestInProgress = false;
}

void SupabaseConnector::begin() {
    _prefs.begin("supabase", false);
    _deviceApiKey = _prefs.getString("api_key", "");
    _deviceId = _prefs.getString("device_id", "");
    
    // Always skip cert validation for edge functions
    _secureClient.setInsecure();
    
    if (_deviceApiKey.length() > 0 && _deviceId.length() > 0) {
        Serial.println("✅ Device already registered and configured");
        Serial.printf("🔑 Using API key: %s...\n", _deviceApiKey.substring(0, 10).c_str());
        Serial.printf("🆔 Device ID: %s\n", _deviceId.c_str());
    } else {
        Serial.println("⚠️ Device not registered - waiting for API key");
        Serial.println("💡 Use the /connect endpoint to register with your API key");
    }
}

void SupabaseConnector::setSensorManager(SensorManager* manager) {
    _sensorManager = manager;
    Serial.println("✅ SensorManager linked to SupabaseConnector");
}

void SupabaseConnector::loop() {
    if (!isConnected() || _isRequestInProgress) {
        // Update status to offline when not connected
        if (_deviceId.length() > 0 && WiFi.status() != WL_CONNECTED) {
            updateDeviceStatusOffline();
        }
        return;
    }
    
    unsigned long currentTime = millis();
    
    // Stagger operations to prevent simultaneous SSL connections
    // Only do one operation per loop cycle
    
    // Priority 1: Update device status every 2 minutes (less frequent)
    if (currentTime - _lastStatusUpdate >= 120000) { // 2 minutes
        updateDeviceStatus();
        _lastStatusUpdate = currentTime;
        return; // Exit to prevent other operations this cycle
    }
    
    // Priority 2: Sync sensor data every 30 seconds (less frequent)
    if (currentTime - _lastSync >= 30000) { // 30 seconds
        collectAndSyncSensorData();
        _lastSync = currentTime;
        return; // Exit to prevent other operations this cycle
    }
    
    // Priority 3: Poll for relay commands every 15 seconds (less frequent)
    if (currentTime - _lastPoll >= 15000) { // 15 seconds
        pollRelayCommands();
        _lastPoll = currentTime;
        return; // Exit to prevent other operations this cycle
    }
}

void SupabaseConnector::collectAndSyncSensorData() {
    if (!_sensorManager || _isRequestInProgress) {
        Serial.println("⚠️ No SensorManager available or request in progress");
        return;
    }
    
    Serial.println("🔄 Collecting sensor data for Supabase sync...");
    
    // Create sensor data JSON
    DynamicJsonDocument jsonDoc(2048);
    JsonObject root = jsonDoc.to<JsonObject>();
    _sensorManager->fillNumericJson(root);

    if (root.size() == 0) {
        Serial.println("⚠️ No numeric sensor data available for Supabase sync");
        return;
    }

    if (!enqueueSensorSyncEvent(jsonDoc)) {
        Serial.println("⚠️ Failed to enqueue sensor sync event");
    }
}

bool SupabaseConnector::registerWithApiKey(const String& apiKey) {
    if (_isRequestInProgress) {
        Serial.println("⚠️ Request already in progress, skipping registration");
        return false;
    }
    
    Serial.println("🔄 Validating API key: " + apiKey.substring(0, 15) + "...");
    
    // First validate the API key
    if (!validateApiKey(apiKey)) {
        Serial.println("❌ API key validation failed");
        return false;
    }
    
    Serial.println("✅ API key validated successfully!");
    
    // Store the API key and mark as registered
    _deviceApiKey = apiKey;
    _prefs.putString("api_key", apiKey);
    
    // The device_id will be determined from the API key validation
    Serial.println("✅ Device registered with API key");
    updateDeviceStatus();
    
    return true;
}

bool SupabaseConnector::validateApiKey(const String& apiKey) {
    if (_isRequestInProgress) {
        return false;
    }
    
    _isRequestInProgress = true;
    
    _http.end();
    delay(100);
    
    _http.setTimeout(10000);
    _http.begin(_secureClient, _supabaseUrl + "/functions/v1/esp32-sensor-data");
    _http.addHeader("Content-Type", "application/json");
    // Send both headers that the edge function might be looking for
    _http.addHeader("x-device-api-key", apiKey);
    _http.addHeader("X-Device-Api-Key", apiKey);
    _http.addHeader("authorization", apiKey);
    _http.addHeader("Authorization", apiKey);
    
    // Send a test payload to validate the API key
    DynamicJsonDocument testDoc(256);
    testDoc["temperature"] = -999; // Test value
    testDoc["humidity"] = -999;
    testDoc["test_validation"] = true;
    
    String payload;
    serializeJson(testDoc, payload);
    
    int httpCode = _http.POST(payload);
    bool isValid = (httpCode >= 200 && httpCode < 300);
    
    if (isValid) {
        Serial.println("✅ API key is valid");
        
        // Try to get device info from the response or make another call
        String response = _http.getString();
        Serial.println("📥 Validation response: " + response);
        
        // For now, we'll generate a device ID based on MAC address
        // In a real implementation, you'd get this from the API response
        _deviceId = "dev_" + WiFi.macAddress();
        _deviceId.replace(":", "");
        _prefs.putString("device_id", _deviceId);
        
    } else {
        Serial.printf("❌ API key validation failed: %d\n", httpCode);
        if (httpCode > 0) {
            String errorResponse = _http.getString();
            Serial.println("📥 Error response: " + errorResponse);
        }
    }
    
    _http.end();
    _isRequestInProgress = false;
    
    return isValid;
}

bool SupabaseConnector::isConnected() {
    bool wifiConnected = WiFi.status() == WL_CONNECTED;
    bool hasApiKey = _deviceApiKey.length() > 0;
    bool hasDeviceId = _deviceId.length() > 0;
    
    if (!wifiConnected) {
        Serial.println("⚠️ WiFi not connected");
    }
    if (!hasApiKey) {
        Serial.println("⚠️ No device API key - device not registered");
    }
    if (!hasDeviceId) {
        Serial.println("⚠️ No device ID - device not registered");
    }
    
    return wifiConnected && hasApiKey && hasDeviceId;
}

void SupabaseConnector::setDeviceApiKey(const String& apiKey) {
    _deviceApiKey = apiKey;
    _prefs.putString("api_key", apiKey);
    Serial.println("🔑 Supabase API key updated");
    updateDeviceStatus();
}

void SupabaseConnector::storeCredentials(const String& deviceId, const String& apiKey) {
    _deviceId = deviceId;
    _deviceApiKey = apiKey;
    _prefs.putString("device_id", deviceId);
    _prefs.putString("api_key", apiKey);
    Serial.println("✅ Device credentials stored successfully");
}

void SupabaseConnector::syncSensorData(const DynamicJsonDocument& sensorData) {
    String payload = formatSensorDataForSupabase(sensorData);
    syncSensorData(payload);
}

void SupabaseConnector::syncSensorData(const String& payload) {
    if (!isConnected() || _isRequestInProgress) {
        Serial.println("❌ Cannot sync sensor data - not connected or request in progress");
        return;
    }

    Serial.println("🔄 Syncing sensor data to Supabase...");

    if (makeRequest("/functions/v1/esp32-sensor-data", "POST", payload)) {
        Serial.println("✅ Sensor data synced to Supabase successfully!");
    } else {
        Serial.println("❌ Failed to sync sensor data to Supabase");
    }
}

void SupabaseConnector::syncRelayStates(bool pumpState, bool intakeFanState, bool extractorFanState, bool lightsState) {
    if (!isConnected() || _isRequestInProgress) return;
    
    DynamicJsonDocument doc(512);
    doc["pumpState"] = pumpState;
    doc["intakeFanState"] = intakeFanState;
    doc["extractorFanState"] = extractorFanState;
    doc["lightsState"] = lightsState;
    
    String payload;
    serializeJson(doc, payload);
    
    if (makeRequest("/functions/v1/esp32-relay-states", "POST", payload)) {
        Serial.println("✅ Relay states synced to Supabase");
    }
}

bool SupabaseConnector::pollRelayCommands() {
    if (!isConnected() || _isRequestInProgress) {
        return false;
    }
    
    _isRequestInProgress = true;
    
    // Ensure clean connection state
    _http.end();
    delay(25);
    
    _http.setTimeout(5000); // Shorter timeout
    _http.begin(_secureClient, _supabaseUrl + "/functions/v1/esp32-relay-states");
    _http.addHeader("Content-Type", "application/json");
    // Send multiple header formats to ensure compatibility
    _http.addHeader("x-device-api-key", _deviceApiKey);
    _http.addHeader("X-Device-Api-Key", _deviceApiKey);
    _http.addHeader("authorization", _deviceApiKey);
    _http.addHeader("Authorization", _deviceApiKey);
    
    int httpCode = _http.GET();
    bool hasCommands = false;
    
    if (httpCode == 200) {
        String response = _http.getString();
        Serial.println("📥 Polled relay commands from Supabase: " + response);
        hasCommands = response.length() > 10; // Basic check for valid JSON
    } else if (httpCode != -11 && httpCode != -4) { // Don't log timeout/connection errors
        Serial.printf("❌ Failed to poll relay commands: %d\n", httpCode);
    }
    
    _http.end();
    _isRequestInProgress = false;
    return hasCommands;
}

void SupabaseConnector::updateDeviceStatus() {
    if (!isConnected() || _isRequestInProgress) return;
    
    Serial.println("🔄 Updating device status to ONLINE...");
    
    _isRequestInProgress = true;
    
    _http.end();
    delay(25);
    
    _http.setTimeout(8000);
    _http.begin(_secureClient, _supabaseUrl + "/functions/v1/esp32-device-status");
    _http.addHeader("Content-Type", "application/json");
    // Send multiple header formats to ensure compatibility
    _http.addHeader("x-device-api-key", _deviceApiKey);
    _http.addHeader("X-Device-Api-Key", _deviceApiKey);
    _http.addHeader("authorization", _deviceApiKey);
    _http.addHeader("Authorization", _deviceApiKey);
    
    DynamicJsonDocument doc(256);
    doc["status"] = "online";
    doc["local_ip"] = WiFi.localIP().toString();
    
    String payload;
    serializeJson(doc, payload);
    
    int httpCode = _http.POST(payload);
    if (httpCode >= 200 && httpCode < 300) {
        Serial.println("✅ Device status updated to ONLINE successfully!");
    } else if (httpCode != -11 && httpCode != -4) {
        String errorResponse = _http.getString();
        Serial.printf("❌ Failed to update device status: %d - %s\n", httpCode, errorResponse.c_str());
    }
    
    _http.end();
    _isRequestInProgress = false;
}

void SupabaseConnector::updateDeviceStatusOffline() {
    if (_deviceApiKey.length() == 0 || _isRequestInProgress) return;
    
    Serial.println("🔄 Updating device status to OFFLINE...");
    
    _isRequestInProgress = true;
    
    _http.setTimeout(3000);
    _http.begin(_secureClient, _supabaseUrl + "/functions/v1/esp32-device-status");
    _http.addHeader("Content-Type", "application/json");
    // Send multiple header formats to ensure compatibility
    _http.addHeader("x-device-api-key", _deviceApiKey);
    _http.addHeader("X-Device-Api-Key", _deviceApiKey);
    _http.addHeader("authorization", _deviceApiKey);
    _http.addHeader("Authorization", _deviceApiKey);
    
    DynamicJsonDocument doc(128);
    doc["status"] = "offline";
    
    String payload;
    serializeJson(doc, payload);
    
    int httpCode = _http.POST(payload);
    if (httpCode >= 200 && httpCode < 300) {
        Serial.println("✅ Device status updated to OFFLINE");
    }
    
    _http.end();
    _isRequestInProgress = false;
}

bool SupabaseConnector::makeRequest(const String& endpoint, const String& method, const String& payload) {
    if (!isConnected() || _isRequestInProgress) {
        Serial.println("❌ Cannot make request - not connected or request in progress");
        return false;
    }
    
    if (_deviceApiKey.length() == 0) {
        Serial.println("❌ No device API key available - cannot make authenticated request");
        return false;
    }
    
    _isRequestInProgress = true;
    
    _http.end();
    delay(25);
    
    if (endpoint.indexOf("sensor-data") >= 0) {
        _http.setTimeout(10000);
    } else {
        _http.setTimeout(5000);
    }
    
    _http.begin(_secureClient, _supabaseUrl + endpoint);
    _http.addHeader("Content-Type", "application/json");
    // Send multiple header formats to ensure compatibility with edge functions
    _http.addHeader("x-device-api-key", _deviceApiKey);
    _http.addHeader("X-Device-Api-Key", _deviceApiKey);
    _http.addHeader("authorization", _deviceApiKey);
    _http.addHeader("Authorization", _deviceApiKey);
    
    Serial.printf("🔑 Using API key for request: %s...\n", _deviceApiKey.substring(0, 10).c_str());
    
    int httpCode;
    if (method == "POST") {
        httpCode = _http.POST(payload);
    } else if (method == "GET") {
        httpCode = _http.GET();
    } else {
        _http.end();
        _isRequestInProgress = false;
        return false;
    }
    
    bool success = (httpCode >= 200 && httpCode < 300);
    if (!success) {
        if (httpCode == 401) {
            Serial.println("❌ 401 Unauthorized - API key may be invalid or expired");
            String errorResponse = _http.getString();
            Serial.println("📥 Error response: " + errorResponse);
        } else if (httpCode != -11 && httpCode != -4) {
            String errorResponse = _http.getString();
            Serial.printf("❌ Supabase request failed: %d - %s\n", httpCode, errorResponse.c_str());
        }
    } else {
        Serial.printf("✅ Request successful: %d\n", httpCode);
    }
    
    _http.end();
    _isRequestInProgress = false;
    return success;
}

String SupabaseConnector::formatSensorDataForSupabase(const DynamicJsonDocument& data) {
    DynamicJsonDocument formatted(512); // Reduced size
    
    // Map essential sensor data to Supabase expected format
    formatted["temperature"] = data["temperature"];
    formatted["humidity"] = data["humidity"];
    formatted["airtemp"] = data["airtemp"];
    formatted["ph"] = data["ph"];
    formatted["airquality"] = data["airquality"];
    formatted["TVOC"] = data["TVOC"];
    formatted["CO2"] = data["CO2"];
    formatted["tdsSens"] = data["tdsSens"];
    formatted["Lux"] = data["lux"];
    formatted["blue"] = data["blue"];
    formatted["green"] = data["green"];
    formatted["red"] = data["red"];
    formatted["farRed"] = data["farRed"];
    formatted["ChlorophyllIndexRedGreen"] = data["ChlorophyllIndexRedGreen"];
    formatted["ChlorophyllIndexRedBlue"] = data["ChlorophyllIndexRedBlue"];
    formatted["ndvi"] = data["ndvi"];
    formatted["greenIntensity"] = data["greenIntensity"];
    formatted["moisture"] = data["moisture"];
    formatted["soilMoisture"] = data["soilMoisture"];
    formatted["total"] = data["total"];
    formatted["nitro"] = data["nitro"];
    formatted["potas"] = data["potas"];
    formatted["phos"] = data["phos"];
    formatted["soilph"] = data["soilph"];
    
    String result;
    serializeJson(formatted, result);
    return result;
}

void SupabaseConnector::handleCloudEvent(const CloudEvent& event) {
    if (event.length == 0) {
        return;
    }
    switch (event.type) {
        case CloudEventType::SensorSync: {
            String payload(event.payload, event.length);
            syncSensorData(payload);
            break;
        }
        case CloudEventType::RelaySync:
            // TODO: implement queued relay syncs
            break;
        case CloudEventType::Status:
            // TODO: implement queued status heartbeats
            break;
    }
}
