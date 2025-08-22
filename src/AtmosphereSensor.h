#ifndef ATMOSPHERE_SENSOR_H
#define ATMOSPHERE_SENSOR_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include "SparkFun_ENS160.h"

class AtmosphereSensor {
public:
    AtmosphereSensor(AsyncWebServer* server, int sdaPin = -1, int sclPin = -1);
    void begin();
    void loop();
    float getAirQuality() const;
    float getTVOC() const;
    float getCO2() const;

private:
    struct {
        float airquality;
        float TVOC;
        float CO2;
    } _state;
    SparkFun_ENS160 _ens;
    AsyncWebServer* _server;
    bool _sensorAvailable;
    int _sdaPin;
    int _sclPin;
    unsigned long _lastReading;
    const unsigned long _ensInterval = 5000;
};

#endif