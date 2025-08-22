// SensorPH.h
#ifndef SENSOR_PH_H
#define SENSOR_PH_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

class SensorPH {
public:
    SensorPH(AsyncWebServer* server, int pin = -1);
    void begin();
    void loop();
    float getPH() const;
    void setPin(int pin);
    int getPin() const;

private:
    bool isValidADCPin(int pin) const;
    struct {
        float ph;
    } _state;
    AsyncWebServer* _server;
    int _pin;
    unsigned long _lastReading;
    const unsigned long _phInterval = 5000;
};

#endif