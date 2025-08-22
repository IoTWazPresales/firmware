#ifndef MQTT_CONNECTOR_H
#define MQTT_CONNECTOR_H

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <Preferences.h>

// Forward declarations
class SensorManager;

class MQTTConnector {
public:
    MQTTConnector();
    void begin();
    void loop();
    void setSensorManager(SensorManager* manager);
    void setDeviceCredentials(const String& deviceId, const String& apiKey);
    
    // MQTT Publishing methods
    void publishSensorData(const DynamicJsonDocument& sensorData);
    void publishRelayStates(bool pumpState, bool intakeFanState, bool extractorFanState, bool lightsState);
    void publishDeviceStatus(const String& status, const String& localIP = "");
    
    // Connection status
    bool isConnected();
    String getConnectionStatus();
    
private:
    WiFiClientSecure _wifiClient;
    PubSubClient _mqttClient;
    Preferences _prefs;
    SensorManager* _sensorManager;
    
    String _deviceId;
    String _deviceApiKey;
    String _mqttHost;
    int _mqttPort;
    String _mqttUsername;
    String _mqttPassword;
    
    unsigned long _lastReconnectAttempt;
    unsigned long _lastSensorPublish;
    unsigned long _lastStatusPublish;
    unsigned long _reconnectInterval;
    unsigned long _sensorPublishInterval;
    unsigned long _statusPublishInterval;
    
    bool _lastConnectionState;
    
    // Connection management
    bool connectToMQTT();
    void handleDisconnection();
    void onMqttMessage(char* topic, byte* payload, unsigned int length);
    
    // Topic helpers
    String getSensorTopic();
    String getRelayCommandTopic();
    String getRelayStatusTopic();
    String getDeviceStatusTopic();
    
    // Data collection
    void collectAndPublishSensorData();
    
    // Static callback wrapper
    static void mqttCallback(char* topic, byte* payload, unsigned int length);
    static MQTTConnector* instance;
};

#endif