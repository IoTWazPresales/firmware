#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <map>
#include <vector>
#include "SensorHumidityDHT11.h"
// Add other sensor headers as needed (e.g., SensorPH.h, SensorMoisture.h)

class SensorManager {
public:
    SensorManager(AsyncWebServer* server);
    ~SensorManager();
    void begin(); // Initialize sensors based on config
    void loop();  // Call loop() on all sensors
    // Getters for sensor instances (for SensorEndpoints, SensorDataCollector)
    SensorHumidityDHT11* getFirstDHT11() const;
    // Add getters for other sensor types as needed

private:
    void loadConfig(); // Load /config.json
    AsyncWebServer* _server;
    // Pin label to GPIO mapping
    const std::map<String, uint8_t> _pinToGpio = {
        {"D2", 25},
        {"D3", 26},
        {"D5", 0},
        {"D6", 14},
        {"D7", 13},
        {"D9", 2}
    };
    // Sensor vectors
    std::vector<SensorHumidityDHT11*> _dht11Sensors;
    // Add vectors for other sensor types as needed
};

#endif