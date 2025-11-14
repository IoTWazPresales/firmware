#include "SensorDriverRegistry.h"
#include <ESPAsyncWebServer.h>
#include "SensorHumidityDHT11.h"
#include "TemperatureSensor.h"
#include "SensorTDS.h"
#include "SensorPH.h"
#include "SensorMoisture.h"
#include "AtmosphereSensor.h"
#include "SpectralSensor.h"
#include "NPKSensor.h"
#include "SensorRTC.h"

SensorDriverRegistry& SensorDriverRegistry::getInstance() {
  static SensorDriverRegistry instance;
  return instance;
}

void SensorDriverRegistry::registerDriver(const String& driverId, FactoryFunction factory) {
  _factories[driverId] = factory;
}

void* SensorDriverRegistry::createSensor(const String& driverId, AsyncWebServer* server, int pin1, int pin2) {
  auto it = _factories.find(driverId);
  if (it != _factories.end()) {
    return it->second(server, pin1, pin2);
  }
  return nullptr;
}

bool SensorDriverRegistry::hasDriver(const String& driverId) const {
  return _factories.find(driverId) != _factories.end();
}

std::vector<String> SensorDriverRegistry::getRegisteredDrivers() const {
  std::vector<String> drivers;
  for (const auto& pair : _factories) {
    drivers.push_back(pair.first);
  }
  return drivers;
}

// Auto-register built-in drivers on first access
static struct AutoRegister {
  AutoRegister() {
    auto& reg = SensorDriverRegistry::getInstance();
    
    reg.registerDriver("dht11", [](AsyncWebServer* s, int p1, int p2) -> void* {
      return new SensorHumidityDHT11(s, p1, DHT11);
    });
    
    reg.registerDriver("temperature", [](AsyncWebServer* s, int p1, int p2) -> void* {
      return new TemperatureSensor(s, p1);
    });
    
    reg.registerDriver("tds", [](AsyncWebServer* s, int p1, int p2) -> void* {
      return new SensorTDS(s, p1);
    });
    
    reg.registerDriver("ph", [](AsyncWebServer* s, int p1, int p2) -> void* {
      return new SensorPH(s, p1);
    });
    
    reg.registerDriver("moisture", [](AsyncWebServer* s, int p1, int p2) -> void* {
      return new SensorMoisture(s, p1);
    });
    
    reg.registerDriver("atmosphere", [](AsyncWebServer* s, int p1, int p2) -> void* {
      return new AtmosphereSensor(s, 21, 22); // I2C pins
    });
    
    reg.registerDriver("spectral", [](AsyncWebServer* s, int p1, int p2) -> void* {
      return new SpectralSensor(s, 21, 22); // I2C pins
    });
    
    reg.registerDriver("npk", [](AsyncWebServer* s, int p1, int p2) -> void* {
      return new NPKSensor(s);
    });
    
    reg.registerDriver("rtc", [](AsyncWebServer* s, int p1, int p2) -> void* {
      return new SensorRTC(s);
    });
  }
} _autoRegister;

