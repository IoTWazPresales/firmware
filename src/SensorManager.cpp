#include "SensorManager.h"

SensorManager::SensorManager(AsyncWebServer* server) : _server(server) {
}

SensorManager::~SensorManager() {
    // Clean up dynamically allocated sensors
    for (auto dht : _dht11Sensors) {
        delete dht;
    }
    _dht11Sensors.clear();
}

void SensorManager::loadConfig() {
    DynamicJsonDocument config(512);
    File file = SPIFFS.open("/config.json", "r");
    if (file) {
        deserializeJson(config, file);
        file.close();
        Serial.println("Loaded config.json:");
        serializeJson(config, Serial);
        Serial.println();
    } else {
        Serial.println("No config.json found, using defaults");
    }

    // Clear existing sensors
    for (auto dht : _dht11Sensors) {
        delete dht;
    }
    _dht11Sensors.clear();

    // Create sensors based on config
    for (JsonPair pair : config.as<JsonObject>()) {
        String pinLabel = pair.key().c_str();
        String sensorType = pair.value().as<String>();
        if (sensorType == "airTempHumidity") {
            auto it = _pinToGpio.find(pinLabel);
            if (it != _pinToGpio.end()) {
                uint8_t gpio = it->second;
                SensorHumidityDHT11* dht = new SensorHumidityDHT11(_server, gpio, DHT11);
                _dht11Sensors.push_back(dht);
                Serial.printf("Assigned DHT11 to %s (GPIO %d)\n", pinLabel.c_str(), gpio);
            } else {
                Serial.printf("Invalid pin label: %s\n", pinLabel.c_str());
            }
        }
        // Add other sensor types here (e.g., soilMoisture, ph)
    }
}

void SensorManager::begin() {
    loadConfig();
    // Initialize all sensors
    for (auto dht : _dht11Sensors) {
        dht->begin();
    }
    // Add initialization for other sensor types
}

void SensorManager::loop() {
    for (auto dht : _dht11Sensors) {
        dht->loop();
    }
    // Add loop calls for other sensor types
}

SensorHumidityDHT11* SensorManager::getFirstDHT11() const {
    return _dht11Sensors.empty() ? nullptr : _dht11Sensors[0];
}