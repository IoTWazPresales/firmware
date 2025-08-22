// SensorRTC.cpp
#include "SensorRTC.h"
#include <Wire.h>

SensorRTC::SensorRTC(AsyncWebServer* server) 
    : _server(server), _lastReading(0), _rtcAvailable(false) {
    _state.rtc = "Unavailable";
}

void SensorRTC::begin() {
    Serial.println("Starting RTC sensor...");
    if (!_rtc.begin()) {
        Serial.println("RTC ERROR");
        _rtcAvailable = false;
        _state.rtc = "Unavailable";
        return;
    }
    _rtcAvailable = true;
    if (_rtc.lostPower()) {
        Serial.println("RTC lost power, setting time...");
        _rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    loop();
}

void SensorRTC::loop() {
    if (!_rtcAvailable) return;
    unsigned long currentMillis = millis();
    if (currentMillis - _lastReading >= _rtcInterval) {
        Wire.beginTransmission(0x68);
        if (Wire.endTransmission() != 0) {
            Serial.println("RTC not responding");
            _rtcAvailable = false;
            _state.rtc = "Unavailable";
            return;
        }
        DateTime now = _rtc.now();
        if (!now.isValid()) {
            Serial.println("Invalid RTC time");
            _rtcAvailable = false;
            _state.rtc = "Unavailable";
            return;
        }
        _state.rtc = now.timestamp();
        Serial.printf("RTC: %s\n", _state.rtc.c_str());
        _lastReading = currentMillis;
    }
}

String SensorRTC::getRTC() const {
    return _state.rtc;
}