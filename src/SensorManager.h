#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <map>
#include <math.h>
#include <functional>
#include <vector>

#include "SensorHumidityDHT11.h"
#include "TemperatureSensor.h"
#include "SensorTDS.h"
#include "SensorRTC.h"
#include "SensorPH.h"
#include "SensorMoisture.h"
#include "AtmosphereSensor.h"
#include "SpectralSensor.h"
#include "NPKSensor.h"
#include "SensorManifest.h"

enum class SensorValueKind {
    Numeric,
    Text
};

struct SensorSample {
    bool valid = false;
    float numeric = NAN;
    String text;
};

struct SensorCapability {
    String id;
    String label;
    String unit;
    SensorValueKind kind = SensorValueKind::Numeric;
    std::function<SensorSample()> supplier;
};

class SensorManager {
public:
    SensorManager(AsyncWebServer* server);
    ~SensorManager();

    void begin();
    void loop();
    void loadConfig();
    float getParameterValue(const String& name) const;

    void enumerateCapabilities(const std::function<void(const SensorCapability&, const SensorSample&)>& fn) const;
    void fillValuesJson(JsonObject& root) const;
    void fillNumericJson(JsonObject& root) const;
    void describeCapabilities(JsonArray& array) const;

    // Legacy getters — unchanged
    SensorHumidityDHT11* getFirstDHT11() const;
    TemperatureSensor*   getFirstTemperatureSensor() const;
    SensorTDS*           getFirstTDS() const;
    SensorRTC*           getFirstRTC() const;
    SensorPH*            getFirstPH() const;
    SensorMoisture*      getFirstMoistureSensor() const;
    AtmosphereSensor*    getFirstAtmosphereSensor() const;
    SpectralSensor*      getFirstSpectralSensor() const;
    NPKSensor*           getFirstNPKSensor() const;

private:
    AsyncWebServer* _server;

    // Direct-GPIO sensors
    std::vector<SensorHumidityDHT11*> _dht11Sensors;
    std::vector<TemperatureSensor*>   _tempSensors;
    std::vector<SensorTDS*>           _tdsSensors;
    SensorRTC*                        _rtcSensor      = nullptr;
    std::vector<SensorPH*>            _phSensors;
    std::vector<SensorMoisture*>      _moistureSensors;
    std::vector<AtmosphereSensor*>    _atmosphereSensors;
    std::vector<SpectralSensor*>      _spectralSensors;
    std::vector<NPKSensor*>           _npkSensors;

    // I²C & UART configs (still reserved for future use)
    struct I2CConfig { int sda, scl; String type; };
    struct UARTConfig{ int rx, tx, re;  String type; };
    std::vector<I2CConfig>  _i2cConfigs;
    std::vector<UARTConfig> _uartConfigs;

    std::vector<SensorCapability> _capabilities;
    std::map<String, SensorSample> _latestSamples;
    bool _needsCapabilityRefresh = true;
    std::vector<SensorManifest> _manifests;

    // GPIO lookup for direct pins
    const std::map<String,uint8_t> _pinToGpio = {
      {"D2",25},  {"D3",26}, {"D5",0},  {"D6",14},
      {"D7",13},  {"D9",2},  {"A0",36}, {"A1",39},
      {"A2",34},  {"A3",35}, {"A4",15}, {"IO21",21},
      {"IO22",22},{"IO16",16},{"IO17",17},{"IO4",4},
      {"IO12",12}
    };

    void registerCapabilities();
    void registerNumericCapability(const String& id,
                                   const String& label,
                                   const String& unit,
                                   std::function<float()> reader);
    void registerTextCapability(const String& id,
                                const String& label,
                                std::function<String()> reader);
    void updateSamples();
    void loadManifests();
    String manifestLabelFor(const String& capabilityId, const String& fallback) const;
    String manifestUnitFor(const String& capabilityId, const String& fallback) const;
};

#endif // SENSOR_MANAGER_H
