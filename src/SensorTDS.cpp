// SensorTDS.cpp
#include "SensorTDS.h"

SensorTDS::SensorTDS(AsyncWebServer* server, int pin) 
    : _server(server), _pin(pin), _lastReading(0) {
    _state.tds = -1;
}

bool SensorTDS::isValidADCPin(int pin) const {
    // Valid ADC pins: GPIO36 (A0), 39 (A1), 34 (A2), 35 (A3), 15 (A4)
    return pin == 36 || pin == 39 || pin == 34 || pin == 35 || pin == 15;
}

void SensorTDS::begin() {
    if (_pin == -1 || !isValidADCPin(_pin)) {
        Serial.printf("TDS sensor invalid or unassigned pin (GPIO %d)\n", _pin);
        return;
    }
    Serial.printf("Starting TDS sensor on GPIO %d...\n", _pin);
    pinMode(_pin, INPUT);
    loop();
}

void SensorTDS::loop() {
    if (_pin == -1 || !isValidADCPin(_pin)) return;
    unsigned long currentMillis = millis();
    if (currentMillis - _lastReading >= _tdsInterval) {
        float tds = analogRead(_pin) / 40.95; // Convert to %
        _state.tds = tds;
        Serial.printf("TDS: %.2f%%\n", tds);
        _lastReading = currentMillis;
    }
}

float SensorTDS::getTDS() const {
    return _state.tds;
}

void SensorTDS::setPin(int pin) {
    _pin = pin;
    if (_pin != -1 && isValidADCPin(_pin)) {
        pinMode(_pin, INPUT);
        Serial.printf("TDS sensor reassigned to GPIO %d\n", _pin);
    } else {
        Serial.printf("Invalid ADC pin for TDS sensor: GPIO %d\n", _pin);
    }
}

int SensorTDS::getPin() const {
    return _pin;
}