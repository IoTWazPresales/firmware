// SensorPH.cpp
#include "SensorPH.h"

SensorPH::SensorPH(AsyncWebServer* server, int pin) 
    : _server(server), _pin(pin), _lastReading(0) {
    _state.ph = -1;
}

bool SensorPH::isValidADCPin(int pin) const {
    // Valid ADC pins: GPIO36 (A0), 39 (A1), 34 (A2), 35 (A3), 15 (A4)
    return pin == 36 || pin == 39 || pin == 34 || pin == 35 || pin == 15;
}

void SensorPH::begin() {
    if (_pin == -1 || !isValidADCPin(_pin)) {
        Serial.printf("pH sensor invalid or unassigned pin (GPIO %d)\n", _pin);
        return;
    }
    Serial.printf("Starting pH sensor on GPIO %d...\n", _pin);
    pinMode(_pin, INPUT);
    loop();
}

void SensorPH::loop() {
    if (_pin == -1 || !isValidADCPin(_pin)) return;
    unsigned long currentMillis = millis();
    if (currentMillis - _lastReading >= _phInterval) {
        float voltage = analogRead(_pin) * 3.3 / 4096.0;
        _state.ph = ((voltage - 0.97) * 7.0) + 7.0; // Adjust based on calibration
        Serial.printf("pH: %.2f\n", _state.ph);
        _lastReading = currentMillis;
    }
}

float SensorPH::getPH() const {
    return _state.ph;
}

void SensorPH::setPin(int pin) {
    _pin = pin;
    if (_pin != -1 && isValidADCPin(_pin)) {
        pinMode(_pin, INPUT);
        Serial.printf("pH sensor reassigned to GPIO %d\n", _pin);
    } else {
        Serial.printf("Invalid ADC pin for pH sensor: GPIO %d\n", _pin);
    }
}

int SensorPH::getPin() const {
    return _pin;
}