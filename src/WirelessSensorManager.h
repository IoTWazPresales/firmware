#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

// Unified sensor abstraction for all wireless protocols
class WirelessSensorManager {
public:
    static void begin();
    static void loop();
    
    // BLE scanning
    static void scanBLEDevices();
    static JsonArray getBLEDevices();
    
    // LoRaWAN management
    static void initLoRaWAN();
    static void processLoRaWANMessages();
    static bool sendLoRaWANCommand(const String& deviceEUI, const String& command);
    
    // Sigfox integration
    static void initSigfox();
    static void processSigfoxCallbacks();
    
    // Unified sensor registration
    static bool registerWirelessSensor(const String& protocol, const String& deviceId, const JsonObject& metadata);
    static JsonArray getAllWirelessSensors();
    
private:
    static bool _bleInitialized;
    static bool _loraInitialized;
    static bool _sigfoxInitialized;
    static StaticJsonDocument<2048> _wirelessSensors;
};

