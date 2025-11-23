// Auto-detection implementation for SensorManager
#include "SensorManager.h"
#include "SensorDriverRegistry.h"
#include "DeviceScanner.h"
#include "SpectralSensor.h"
#include "AtmosphereSensor.h"
#include "NPKSensor.h"
#include "SensorHumidityDHT11.h"
#include "TemperatureSensor.h"
#include "SensorTDS.h"
#include "SensorPH.h"
#include "SensorMoisture.h"
#include <ArduinoJson.h>
#include <LittleFS.h>

// I²C pins (must match SensorManager.cpp)
static constexpr int I2C_SDA = 21;
static constexpr int I2C_SCL = 22;

void SensorManager::autoDetectSensors(DeviceScanner* scanner) {
    if (!scanner) {
        Serial.println("⚠️ Auto-detection: No scanner available");
        return;
    }
    
    Serial.println("🔍 Starting auto-detection from manifests...");
    
    // Get scan results
    String scanJson = scanner->getScanResults();
    DynamicJsonDocument scanDoc(2048);
    if (deserializeJson(scanDoc, scanJson) != DeserializationError::Ok) {
        Serial.println("⚠️ Failed to parse scan results");
        return;
    }
    
    int detectedCount = 0;
    
    // Check manifests with autoDetect flag
    for (const auto& manifest : _manifests) {
        if (!manifest.autoDetect) {
            continue; // Skip non-auto-detectable sensors
        }
        
        Serial.printf("  🔎 Checking manifest: %s\n", manifest.driverId.c_str());
        
        // I2C sensors
        if (manifest.driverId == "spectral" || manifest.driverId == "atmosphere" || manifest.driverId == "npk") {
            JsonArray i2cDevices = scanDoc["i2cDevices"].as<JsonArray>();
            if (i2cDevices.size() > 0) {
                // Validate I2C pins (SDA=21, SCL=22 are standard ESP32 I2C pins)
                if (I2C_SDA < 0 || I2C_SDA > 39 || I2C_SCL < 0 || I2C_SCL > 39) {
                    Serial.printf("    ⚠️ Invalid I2C pins (SDA=%d, SCL=%d) for %s\n", I2C_SDA, I2C_SCL, manifest.driverId.c_str());
                    continue;
                }
                // Try to create sensor using registry
                auto& registry = SensorDriverRegistry::getInstance();
                if (registry.hasDriver(manifest.driverId)) {
                    void* sensor = registry.createSensor(manifest.driverId, _server, I2C_SDA, I2C_SCL);
                    if (sensor) {
                        if (manifest.driverId == "spectral") {
                            _spectralSensors.push_back(static_cast<SpectralSensor*>(sensor));
                        } else if (manifest.driverId == "atmosphere") {
                            _atmosphereSensors.push_back(static_cast<AtmosphereSensor*>(sensor));
                        } else if (manifest.driverId == "npk") {
                            _npkSensors.push_back(static_cast<NPKSensor*>(sensor));
                        }
                        Serial.printf("    ✅ Auto-detected %s on I2C\n", manifest.driverId.c_str());
                        detectedCount++;
                    }
                }
            }
        }
        
        // GPIO sensors (check digital pins)
        if (manifest.driverId == "dht11" || manifest.driverId == "temperature" || 
            manifest.driverId == "tds" || manifest.driverId == "ph" || manifest.driverId == "moisture") {
            JsonArray digitalPins = scanDoc["digitalPins"].as<JsonArray>();
            for (JsonObject pin : digitalPins) {
                if (pin["detected"].as<bool>()) {
                    int pinNum = pin["pin"].as<int>();
                    String key = pin["key"].as<String>();
                    
                    // Validate GPIO pin number (ESP32 valid range: 0-39)
                    if (pinNum < 0 || pinNum > 39) {
                        Serial.printf("    ⚠️ Skipping invalid GPIO pin %d for %s\n", pinNum, manifest.driverId.c_str());
                        continue;
                    }
                    
                    // Check if already configured
                    DynamicJsonDocument cfg(2048);
                    File f = LittleFS.open("/config.json", "r");
                    if (f && f.size() > 0) {
                        deserializeJson(cfg, f);
                        f.close();
                    }
                    
                    if (!cfg.containsKey(key.c_str())) {
                        // Auto-create sensor
                        auto& registry = SensorDriverRegistry::getInstance();
                        if (registry.hasDriver(manifest.driverId)) {
                            void* sensor = registry.createSensor(manifest.driverId, _server, pinNum, 0);
                            if (sensor) {
                                if (manifest.driverId == "dht11") {
                                    _dht11Sensors.push_back(static_cast<SensorHumidityDHT11*>(sensor));
                                } else if (manifest.driverId == "temperature") {
                                    _tempSensors.push_back(static_cast<TemperatureSensor*>(sensor));
                                } else if (manifest.driverId == "tds") {
                                    _tdsSensors.push_back(static_cast<SensorTDS*>(sensor));
                                } else if (manifest.driverId == "ph") {
                                    _phSensors.push_back(static_cast<SensorPH*>(sensor));
                                } else if (manifest.driverId == "moisture") {
                                    _moistureSensors.push_back(static_cast<SensorMoisture*>(sensor));
                                }
                                
                                // Update config
                                cfg[key] = manifest.driverId;
                                File w = LittleFS.open("/config.json", "w");
                                if (w) {
                                    serializeJson(cfg, w);
                                    w.close();
                                }
                                
                                Serial.printf("    ✅ Auto-detected %s on %s (GPIO %d)\n", 
                                            manifest.driverId.c_str(), key.c_str(), pinNum);
                                detectedCount++;
                            }
                        }
                    }
                }
            }
        }
    }
    
    Serial.printf("🎯 Auto-detection complete: %d sensor(s) detected\n", detectedCount);
    if (detectedCount > 0) {
        _needsCapabilityRefresh = true;
    }
}

