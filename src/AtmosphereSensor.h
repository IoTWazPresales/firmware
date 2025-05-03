#ifndef AtmosphereSensor_h
#define AtmosphereSensor_h

#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "SparkFun_ENS160.h"  // Click here to get the library: http://librarymanager/All#SparkFun_ENS160
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>


class AtmosphereSensor{
 public:
  AtmosphereSensor(AsyncWebServer* server);
    void begin();
    void loop();
    void readSensor();
    float getAirQuality() const;      // Getter for humidity value
    float getTVOC() const;   // Getter for air temperature value
    float getCO2() const;
 private:
  
   struct SensorState {
        float airquality; // pH value
        float TVOC;
        float CO2; // pH value
        
    } _state;
    SparkFun_ENS160 _ens;
    AsyncWebServer* _server;
    bool _sensorAvailable; // Flag for sensor presence
    const unsigned long ENSInterval = 5000;  // Interval in milliseconds
    unsigned long _lastReading = 0;
  
};

#endif



