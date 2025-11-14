#include "SensorManager.h"
#include <Wire.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <cmath>
#include <utility>
#include <cstring>

// I²C pins (unchanged)
static constexpr int I2C_SDA = 21;
static constexpr int I2C_SCL = 22;

namespace {
inline SensorSample makeNumericSample(float value) {
    SensorSample sample;
    if (std::isnan(value)) {
        sample.valid = false;
        sample.numeric = NAN;
    } else {
        sample.valid = true;
        sample.numeric = value;
    }
    return sample;
}

inline SensorSample makeTextSample(const String& value) {
    SensorSample sample;
    sample.valid = value.length() > 0;
    sample.text = value;
    return sample;
}
}

SensorManager::SensorManager(AsyncWebServer* server)
  : _server(server)
  , _rtcSensor(nullptr)
  , _needsCapabilityRefresh(true)
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
    DynamicJsonDocument cfg(2048);
    File f = LittleFS.open("/config.json","r");
    if (f && f.size()>0) {
        deserializeJson(cfg, f);
        f.close();
    } else {
        File w = LittleFS.open("/config.json","w");
        w.print("{}");
        w.close();
    }

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
    _latestSamples.clear();
    if (!cfg.is<JsonObject>()) return;

    for (auto kv : cfg.as<JsonObject>()) {
        String key  = kv.key().c_str();
        const char* type = kv.value().as<const char*>();

        // — I²C sockets J1–J4 —
        if (key.startsWith("J")) {
            if (strcmp(type, "spectral") == 0) {
                _spectralSensors.push_back(new SpectralSensor(_server, I2C_SDA, I2C_SCL));
            } 
            else if (strcmp(type, "airQuality") == 0) {
                _atmosphereSensors.push_back(new AtmosphereSensor(_server, I2C_SDA, I2C_SCL));
            } else if (strcmp(type, "npk") == 0) {
                _npkSensors.push_back(new NPKSensor(_server));
            }
            continue;
        }

        if (strcmp(type, "rtcSensor") == 0) {
            if (!_rtcSensor) _rtcSensor = new SensorRTC(_server);
            continue;
        }

        auto it = _pinToGpio.find(key);
        if (it == _pinToGpio.end()) continue;
        int gpio = it->second;

        if (strcmp(type, "airTempHumidity") == 0) {
            _dht11Sensors.push_back(new SensorHumidityDHT11(_server, gpio, DHT11));
        } 
        else if (strcmp(type, "temperatureSensor") == 0) {
            _tempSensors.push_back(new TemperatureSensor(_server, gpio));
        } 
        else if (strcmp(type, "tdsSensor") == 0) {
            _tdsSensors.push_back(new SensorTDS(_server, gpio));
        } 
        else if (strcmp(type, "phSensor") == 0) {
            _phSensors.push_back(new SensorPH(_server, gpio));
        } 
        else if (strcmp(type, "moistureSensor") == 0) {
            _moistureSensors.push_back(new SensorMoisture(_server, gpio));
        }
    }
    _needsCapabilityRefresh = true;
}

void SensorManager::begin() {
    Wire.begin(I2C_SDA, I2C_SCL);
    loadConfig();
    loadManifests();

    for (auto *p: _dht11Sensors)      p->begin();
    for (auto *p: _tempSensors)       p->begin();
    for (auto *p: _tdsSensors)        p->begin();
    if (_rtcSensor)                   _rtcSensor->begin();
    for (auto *p: _phSensors)         p->begin();
    for (auto *p: _moistureSensors)   p->begin();
    for (auto *p: _atmosphereSensors) p->begin();
    for (auto *p: _spectralSensors)   p->begin();
    for (auto *p: _npkSensors)        p->begin();

    registerCapabilities();
    updateSamples();
    _needsCapabilityRefresh = false;
}

void SensorManager::registerNumericCapability(const String& id,
                                              const String& label,
                                              const String& unit,
                                              std::function<float()> reader) {
    SensorCapability cap;
    cap.id = id;
    cap.label = manifestLabelFor(id, label);
    cap.unit = manifestUnitFor(id, unit);
    cap.kind = SensorValueKind::Numeric;
    cap.supplier = [reader]() -> SensorSample {
        if (!reader) {
            return makeNumericSample(NAN);
        }
        float value = reader();
        return makeNumericSample(value);
    };
    _capabilities.push_back(std::move(cap));
}

void SensorManager::registerTextCapability(const String& id,
                                           const String& label,
                                           std::function<String()> reader) {
    SensorCapability cap;
    cap.id = id;
    cap.label = manifestLabelFor(id, label);
    cap.unit = manifestUnitFor(id, "");
    cap.kind = SensorValueKind::Text;
    cap.supplier = [reader]() -> SensorSample {
        if (!reader) {
            return makeTextSample(String());
        }
        return makeTextSample(reader());
    };
    _capabilities.push_back(std::move(cap));
}

void SensorManager::registerCapabilities() {
    _capabilities.clear();

    if (auto* s = getFirstTemperatureSensor()) {
        registerNumericCapability("temperature", "Water Temperature", "°C", [s]() {
            float v = s->getTemperature();
            return (v < -40.0f) ? NAN : v;
        });
    }
    if (auto* dht = getFirstDHT11()) {
        registerNumericCapability("airtemp", "Air Temperature", "°C", [dht]() {
            float v = dht->getAirTemperature();
            return (v < -40.0f) ? NAN : v;
        });
        registerNumericCapability("humidity", "Air Humidity", "%", [dht]() {
            float v = dht->getHumidity();
            return (v < 0.0f) ? NAN : v;
        });
    }
    if (auto* moisture = getFirstMoistureSensor()) {
        auto moistureReader = [moisture]() {
            float v = moisture->getMoisture();
            return (v < 0.0f) ? NAN : v;
        };
        registerNumericCapability("moisture", "Soil Moisture", "%", moistureReader);
        registerNumericCapability("soilMoisture", "Soil Moisture (alias)", "%", moistureReader);
    }
    if (auto* tds = getFirstTDS()) {
        registerNumericCapability("tdsSens", "TDS", "ppm", [tds]() {
            float v = tds->getTDS();
            return (v < 0.0f) ? NAN : v;
        });
    }
    if (auto* ph = getFirstPH()) {
        registerNumericCapability("ph", "Solution pH", "", [ph]() {
            float v = ph->getPH();
            return (v < 0.0f) ? NAN : v;
        });
    }
    if (auto* rtc = getFirstRTC()) {
        registerTextCapability("real", "RTC Timestamp", [rtc]() {
            return rtc->getRTC();
        });
    }
    if (auto* atmos = getFirstAtmosphereSensor()) {
        registerNumericCapability("CO2", "CO₂", "ppm", [atmos]() {
            float v = atmos->getCO2();
            return (v < 0.0f) ? NAN : v;
        });
        registerNumericCapability("TVOC", "TVOC", "ppb", [atmos]() {
            float v = atmos->getTVOC();
            return (v < 0.0f) ? NAN : v;
        });
        registerNumericCapability("airquality", "Air Quality Index", "", [atmos]() {
            float v = atmos->getAirQuality();
            return (v < 0.0f) ? NAN : v;
        });
    }
    if (auto* npk = getFirstNPKSensor()) {
        registerNumericCapability("nitro", "Nitrogen", "mg/kg", [npk]() {
            float v = npk->getNitrogen();
            return (v < 0.0f) ? NAN : v;
        });
        registerNumericCapability("potas", "Potassium", "mg/kg", [npk]() {
            float v = npk->getPotassium();
            return (v < 0.0f) ? NAN : v;
        });
        registerNumericCapability("phos", "Phosphorus", "mg/kg", [npk]() {
            float v = npk->getPhosphorus();
            return (v < 0.0f) ? NAN : v;
        });
        registerNumericCapability("soilph", "Soil pH", "", [npk]() {
            float v = npk->getPHSoil();
            return (v < 0.0f) ? NAN : v;
        });
    }
    if (auto* spect = getFirstSpectralSensor()) {
        registerNumericCapability("total", "PAR Total", "µmol/m²/s", [spect]() {
            float v = spect->getTotalLight();
            return (v < 0.0f) ? NAN : v;
        });
        registerNumericCapability("blue", "Blue Ratio", "", [spect]() {
            return spect->getBlueRatio();
        });
        registerNumericCapability("green", "Green Ratio", "", [spect]() {
            return spect->getGreenRatio();
        });
        registerNumericCapability("red", "Red Ratio", "", [spect]() {
            return spect->getRedRatio();
        });
        registerNumericCapability("farRed", "Far Red Ratio", "", [spect]() {
            return spect->getFarRedRatio();
        });
        registerNumericCapability("ChlorophyllIndexRedGreen", "Chlorophyll Index (R/G)", "", [spect]() {
            return spect->getChlorophyllIndexRedGreen();
        });
        registerNumericCapability("ChlorophyllIndexRedBlue", "Chlorophyll Index (R/B)", "", [spect]() {
            return spect->getChlorophyllIndexRedBlue();
        });
        registerNumericCapability("ndvi", "NDVI", "", [spect]() {
            return spect->getNDVI();
        });
        registerNumericCapability("greenIntensity", "Green Light Intensity", "lux", [spect]() {
            return spect->getGreenLightIntensity();
        });
        registerNumericCapability("lux", "Lux", "lux", [spect]() {
            return spect->getLux();
        });
    }
}

void SensorManager::updateSamples() {
    for (const auto& cap : _capabilities) {
        SensorSample sample = cap.supplier ? cap.supplier() : SensorSample{};
        _latestSamples[cap.id] = sample;
    }
}

void SensorManager::loop() {
    for (auto *p: _dht11Sensors)      p->loop();
    for (auto *p: _tempSensors)       p->loop();
    for (auto *p: _tdsSensors)        p->loop();
    if (_rtcSensor)                   _rtcSensor->loop();
    for (auto *p: _phSensors)         p->loop();
    for (auto *p: _moistureSensors)   p->loop();
    for (auto *p: _atmosphereSensors) p->loop();
    for (auto *p: _spectralSensors)   p->loop();
    for (auto *p: _npkSensors)        p->loop();

    if (_needsCapabilityRefresh) {
        registerCapabilities();
        _needsCapabilityRefresh = false;
    }

    updateSamples();
}

float SensorManager::getParameterValue(const String& name) const {
    auto it = _latestSamples.find(name);
    if (it == _latestSamples.end()) {
        return -1.0f;
    }
    const SensorSample& sample = it->second;
    if (!sample.valid || std::isnan(sample.numeric)) {
        return -1.0f;
    }
    return sample.numeric;
}

void SensorManager::enumerateCapabilities(const std::function<void(const SensorCapability&, const SensorSample&)>& fn) const {
    if (!fn) return;
    for (const auto& cap : _capabilities) {
        auto it = _latestSamples.find(cap.id);
        if (it != _latestSamples.end()) {
            fn(cap, it->second);
        } else {
            fn(cap, SensorSample{});
        }
    }
}

void SensorManager::fillValuesJson(JsonObject& root) const {
    enumerateCapabilities([&](const SensorCapability& cap, const SensorSample& sample){
        if (cap.kind == SensorValueKind::Numeric) {
            if (sample.valid && !std::isnan(sample.numeric)) {
                root[cap.id] = sample.numeric;
            } else {
                root[cap.id] = nullptr;
            }
        } else {
            if (sample.valid) {
                root[cap.id] = sample.text;
            } else {
                root[cap.id] = nullptr;
            }
        }
    });
}

void SensorManager::fillNumericJson(JsonObject& root) const {
    enumerateCapabilities([&](const SensorCapability& cap, const SensorSample& sample){
        if (cap.kind != SensorValueKind::Numeric) {
            return;
        }
        if (sample.valid && !std::isnan(sample.numeric)) {
            root[cap.id] = sample.numeric;
        }
    });
}

void SensorManager::describeCapabilities(JsonArray& array) const {
    for (const auto& cap : _capabilities) {
        JsonObject obj = array.createNestedObject();
        obj["id"] = cap.id;
        obj["label"] = cap.label;
        if (!cap.unit.isEmpty()) {
            obj["unit"] = cap.unit;
        } else {
            obj["unit"] = "";
        }
        obj["kind"] = (cap.kind == SensorValueKind::Numeric) ? "numeric" : "text";
    }
}

void SensorManager::loadManifests() {
    _manifests = SensorManifest::loadAll(LittleFS);
}

String SensorManager::manifestLabelFor(const String& capabilityId, const String& fallback) const {
    const SensorManifest* manifest = findManifestForCapability(_manifests, capabilityId);
    if (!manifest) {
        return fallback;
    }
    auto it = manifest->capabilityLabels.find(capabilityId);
    if (it != manifest->capabilityLabels.end() && it->second.length() > 0) {
        return it->second;
    }
    if (manifest->label.length() > 0) {
        return manifest->label;
    }
    return fallback;
}

String SensorManager::manifestUnitFor(const String& capabilityId, const String& fallback) const {
    const SensorManifest* manifest = findManifestForCapability(_manifests, capabilityId);
    if (!manifest) {
        return fallback;
    }
    auto it = manifest->capabilityUnits.find(capabilityId);
    if (it != manifest->capabilityUnits.end() && it->second.length() > 0) {
        return it->second;
    }
    if (manifest->unit.length() > 0) {
        return manifest->unit;
    }
    return fallback;
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
