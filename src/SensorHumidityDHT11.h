#ifndef SensorHumidityDHT11_h
#define SensorHumidityDHT11_h

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <ArduinoJson.h>


class S_H_DHT11State {
 public:
  float airtemperature;
  float airhumidity;

  /*static void read(S_H_DHT11State& settings, JsonObject& root) {
    root["airtemperature"] = settings.airtemperature;
    root["airhumidity"] = settings.airhumidity;
  }*/
};

class SensorHumidityDHT11{
 public:
  SensorHumidityDHT11(uint8_t pin, uint8_t type) : _dht(pin, type) {}

  void begin();
  void loop();
  void readSensor();

 private:
  DHT _dht;

  unsigned long _lastReading = 0;

  void registerConfig();
  void onConfigUpdated();
};

#endif
