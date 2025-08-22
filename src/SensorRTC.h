// SensorRTC.h
#ifndef SENSOR_RTC_H
#define SENSOR_RTC_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <RTClib.h>
#include <ESPAsyncWebServer.h>

class SensorRTC {
public:
    SensorRTC(AsyncWebServer* server);
    void begin();
    void loop();
    String getRTC() const;

private:
    struct {
        String rtc;
    } _state;
    AsyncWebServer* _server;
    RTC_DS3231 _rtc;
    bool _rtcAvailable;
    unsigned long _lastReading;
    const unsigned long _rtcInterval = 5000;
};

#endif