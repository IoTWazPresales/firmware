#ifndef TEMPERATURE_SENSOR_H
#define TEMPERATURE_SENSOR_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESPAsyncWebServer.h>

class TemperatureSensor {
public:
    TemperatureSensor(AsyncWebServer* server, int pin = -1);
    ~TemperatureSensor();
    void begin();
    void loop();
    float getTemperature() const;
    void setPin(int pin);
    int getPin() const;

private:
    struct {
        float watertemperature;
    } _state;

    OneWire* _oneWire;
    DallasTemperature* _tempSensors;
    AsyncWebServer* _server;
    int _pin;
    unsigned long _lastTemperatureRead;
    const unsigned long _temperatureInterval = 5000;
};

#endif