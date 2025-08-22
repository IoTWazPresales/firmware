#ifndef SENSOR_MOISTURE_H
#define SENSOR_MOISTURE_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

class SensorMoisture {
public:
    SensorMoisture(AsyncWebServer* server, uint8_t pin);
    void begin();
    void loop();
    float getMoisture() const;

private:
    AsyncWebServer* _server;
    uint8_t _pin;
    float _moisture;
    unsigned long _lastReading;
    const unsigned long _interval = 5000;
};

#endif