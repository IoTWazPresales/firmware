
#ifndef SUPABASE_CONNECTOR_H
#define SUPABASE_CONNECTOR_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <Preferences.h>

// Forward declarations
class SensorManager;

class SupabaseConnector {
public:
    SupabaseConnector();
    void begin();
    void loop();
    void setSensorManager(SensorManager* manager);
    void setDeviceApiKey(const String& apiKey);
    void syncSensorData(const DynamicJsonDocument& sensorData);
    void syncRelayStates(bool pumpState, bool intakeFanState, bool extractorFanState, bool lightsState);
    bool pollRelayCommands();
    void updateDeviceStatus();
    void updateDeviceStatusOffline();
    bool isConnected();
    void storeCredentials(const String& deviceId, const String& apiKey);
    
    // NEW: Direct API key registration
    bool registerWithApiKey(const String& apiKey);
    bool validateApiKey(const String& apiKey);
    
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
    
    bool makeRequest(const String& endpoint, const String& method, const String& payload = "");
    String formatSensorDataForSupabase(const DynamicJsonDocument& data);
    void collectAndSyncSensorData();
};

#endif