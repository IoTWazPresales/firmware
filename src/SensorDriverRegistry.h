#pragma once

#include <Arduino.h>
#include <functional>
#include <map>
#include <vector>
#include "SensorManifest.h"

// Forward declaration
class AsyncWebServer;

class SensorDriverRegistry {
public:
  using FactoryFunction = std::function<void*(AsyncWebServer* server, int pin1, int pin2)>;
  
  static SensorDriverRegistry& getInstance();
  
  void registerDriver(const String& driverId, FactoryFunction factory);
  void* createSensor(const String& driverId, AsyncWebServer* server, int pin1 = 0, int pin2 = 0);
  bool hasDriver(const String& driverId) const;
  std::vector<String> getRegisteredDrivers() const;
  
private:
  SensorDriverRegistry() = default;
  std::map<String, FactoryFunction> _factories;
};

