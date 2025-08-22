// SensorManager.cpp

#include "SensorManager.h"
#include <Wire.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

// I²C pins (unchanged)
static constexpr int I2C_SDA = 21;
static constexpr int I2C_SCL = 22;

SensorManager::SensorManager(AsyncWebServer* server)
  : _server(server)
  , _rtcSensor(nullptr)
{}

SensorManager::~SensorManager() {
    for (auto *p : _dht11Sensors)      delete p;
    for (auto *p : _tempSensors)       delete p;
    for (auto *p : _tdsSensors)        delete p;
    delete _rtcSensor;
    for (auto *p : _phSensors)         delete p;
    for (auto *p : _moistureSensors)   delete p;
    for (auto *p : _atmosphereSensors) delete p;
    for (auto *p : _spectralSensors)   delete p;
    for (auto *p : _npkSensors)        delete p;
}

void SensorManager::loadConfig() {
    DynamicJsonDocument cfg(4096);
    File f = LittleFS.open("/config.json","r");
    if (f && f.size()>0) {
        deserializeJson(cfg, f);
        f.close();
    } else {
        // ensure file exists
        File w = LittleFS.open("/config.json","w");
        w.print("{}");
        w.close();
    }

    // clear out everything
    _dht11Sensors.clear();
    _tempSensors.clear();
    _tdsSensors.clear();
    delete _rtcSensor; _rtcSensor = nullptr;
    _phSensors.clear();
    _moistureSensors.clear();
    _atmosphereSensors.clear();
    _spectralSensors.clear();
    _npkSensors.clear();
    _i2cConfigs.clear();
    _uartConfigs.clear();

    if (!cfg.is<JsonObject>()) return;

    for (auto kv : cfg.as<JsonObject>()) {
        String key  = kv.key().c_str();
        String type = kv.value().as<String>();

        // — I²C sockets J1–J4 —
        if (key.startsWith("J")) {
            if (type == "spectral") {
                _spectralSensors.push_back(new SpectralSensor(_server, I2C_SDA, I2C_SCL));
            } 
            else if (type == "airQuality") {
                _atmosphereSensors.push_back(new AtmosphereSensor(_server, I2C_SDA, I2C_SCL));
            }
            continue;
        }

        // — RTC —
        if (type == "rtcSensor") {
            if (!_rtcSensor) _rtcSensor = new SensorRTC(_server);
            continue;
        }

        // — direct-GPIO pins —
        auto it = _pinToGpio.find(key);
        if (it == _pinToGpio.end()) continue;
        int gpio = it->second;

        if (type == "airTempHumidity") {
            _dht11Sensors.push_back(new SensorHumidityDHT11(_server, gpio, DHT11));
        } 
        else if (type == "temperatureSensor") {
            _tempSensors.push_back(new TemperatureSensor(_server, gpio));
        } 
        else if (type == "tdsSensor") {
            _tdsSensors.push_back(new SensorTDS(_server, gpio));
        } 
        else if (type == "phSensor") {
            _phSensors.push_back(new SensorPH(_server, gpio));
        } 
        else if (type == "moistureSensor") {
            _moistureSensors.push_back(new SensorMoisture(_server, gpio));
        }
    }
}

void SensorManager::begin() {
    // bring up I²C
    Wire.begin(I2C_SDA, I2C_SCL);

    // 1) load from config
    loadConfig();

    // 2) begin all direct-GPIO sensors
    for (auto *p: _dht11Sensors)      p->begin();
    for (auto *p: _tempSensors)       p->begin();
    for (auto *p: _tdsSensors)        p->begin();
    if (_rtcSensor)                   _rtcSensor->begin();
    for (auto *p: _phSensors)         p->begin();
    for (auto *p: _moistureSensors)   p->begin();
    for (auto *p: _atmosphereSensors) p->begin();
    for (auto *p: _spectralSensors)   p->begin();
    for (auto *p: _npkSensors)        p->begin();

    // 3) register readers for API
    registerReaders();
}

void SensorManager::registerReaders() {
    _readers["soilMoisture"] = [&](){
        auto* s = getFirstMoistureSensor();
        return s ? s->getMoisture() : -1.0f;
    };
    _readers["temperature"] = [&](){
        auto* s = getFirstDHT11();
        return s ? s->getAirTemperature() : -1.0f;
    };
    _readers["humidity"] = [&](){
        auto* s = getFirstDHT11();
        return s ? s->getHumidity() : -1.0f;
    };
    _readers["ph"] = [&](){
        auto* s = getFirstPH();
        return s ? s->getPH() : -1.0f;
    };
    _readers["co2"] = [&](){
        auto* s = getFirstAtmosphereSensor();
        return s ? s->getCO2() : -1.0f;
    };
    _readers["spectral"] = [&](){
        auto* s = getFirstSpectralSensor();
        return s ? s->getLux() : -1.0f;
    };
    _readers["airQuality"] = [&](){
        auto* s = getFirstAtmosphereSensor();
        return s ? s->getTVOC() : -1.0f;
    };
}

void SensorManager::loop() {
    // direct-GPIO sensors
    for (auto *p: _dht11Sensors)      p->loop();
    for (auto *p: _tempSensors)       p->loop();
    for (auto *p: _tdsSensors)        p->loop();
    if (_rtcSensor)                   _rtcSensor->loop();
    for (auto *p: _phSensors)         p->loop();
    for (auto *p: _moistureSensors)   p->loop();
    for (auto *p: _atmosphereSensors) p->loop();
    for (auto *p: _spectralSensors)   p->loop();
    for (auto *p: _npkSensors)        p->loop();
}

float SensorManager::getParameterValue(const String& name) const {
    auto it = _readers.find(name);
    return it != _readers.end() ? it->second() : -1.0f;
}
// ——— Legacy getters ———

SensorHumidityDHT11* SensorManager::getFirstDHT11() const {
    return _dht11Sensors.empty() ? nullptr : _dht11Sensors[0];
}

TemperatureSensor* SensorManager::getFirstTemperatureSensor() const {
    return _tempSensors.empty() ? nullptr : _tempSensors[0];
}

SensorTDS* SensorManager::getFirstTDS() const {
    return _tdsSensors.empty() ? nullptr : _tdsSensors[0];
}

SensorRTC* SensorManager::getFirstRTC() const {
    return _rtcSensor;
}

SensorPH* SensorManager::getFirstPH() const {
    return _phSensors.empty() ? nullptr : _phSensors[0];
}

SensorMoisture* SensorManager::getFirstMoistureSensor() const {
    return _moistureSensors.empty() ? nullptr : _moistureSensors[0];
}

AtmosphereSensor* SensorManager::getFirstAtmosphereSensor() const {
    return _atmosphereSensors.empty() ? nullptr : _atmosphereSensors[0];
}

SpectralSensor* SensorManager::getFirstSpectralSensor() const {
    return _spectralSensors.empty() ? nullptr : _spectralSensors[0];
}

NPKSensor* SensorManager::getFirstNPKSensor() const {
    return _npkSensors.empty() ? nullptr : _npkSensors[0];
}
