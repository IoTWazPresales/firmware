// SensorTDS.h
#ifndef SENSOR_TDS_H
#define SENSOR_TDS_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

class SensorTDS {
public:
    SensorTDS(AsyncWebServer* server, int pin = -1);
    void begin();
    void loop();
    float getTDS() const;
    void setPin(int pin);
    int getPin() const;

private:
    bool isValidADCPin(int pin) const;
    struct {
        float tds;
    } _state;
    AsyncWebServer* _server;
    int _pin;
    unsigned long _lastReading;
    const unsigned long _tdsInterval = 5000;
};

#endif