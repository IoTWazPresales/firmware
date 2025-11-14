
#ifndef SUPABASE_CONNECTOR_H
#define SUPABASE_CONNECTOR_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include "NetworkResilience.h"

// Forward declarations
class SensorManager;
struct CloudEvent;

class SupabaseConnector {
public:
    SupabaseConnector();
    void begin();
    void loop();
    void setSensorManager(SensorManager* manager);
    void setDeviceApiKey(const String& apiKey);
    void syncSensorData(const DynamicJsonDocument& sensorData);
    void syncSensorData(const String& payload);
    void syncRelayStates(bool pumpState, bool intakeFanState, bool extractorFanState, bool lightsState);
    bool pollRelayCommands();
    bool updateDeviceStatus();
    void updateDeviceStatusOffline();
    bool isConnected();
    void storeCredentials(const String& deviceId, const String& apiKey);
    
    // NEW: Direct API key registration
    bool registerWithApiKey(const String& apiKey);
    bool validateApiKey(const String& apiKey);

    const String& getDeviceId() const { return _deviceId; }
    const String& getApiKey() const { return _deviceApiKey; }
    
    void handleCloudEvent(const CloudEvent& event);

private:
    String _deviceApiKey;
    String _deviceId;
    String _supabaseUrl;
    HTTPClient _http;
    WiFiClientSecure _secureClient;
    Preferences _prefs;
    SensorManager* _sensorManager;
    unsigned long _lastSync;
    unsigned long _lastPoll;
    unsigned long _lastStatusUpdate;
    bool _isRequestInProgress;
    NetworkResilience _networkResilience;
    
    bool makeRequest(const String& endpoint, const String& method, const String& payload = "");
    String formatSensorDataForSupabase(const DynamicJsonDocument& data);
    bool collectAndSyncSensorData();
};

#endif