#include "TemperatureSensor.h"

TemperatureSensor::TemperatureSensor(AsyncWebServer* server, int pin) 
    : _server(server), _pin(pin), _lastTemperatureRead(0), _oneWire(nullptr), _tempSensors(nullptr) {
    _state.watertemperature = -1.0f;
    if (_pin != -1) {
        _oneWire = new OneWire(_pin);
        _tempSensors = new DallasTemperature(_oneWire);
        Serial.printf("Attempting to initialize temperature sensor on GPIO %d\n", _pin);
    } else {
        Serial.println("Temperature sensor disabled: Invalid pin (-1)");
    }
}

TemperatureSensor::~TemperatureSensor() {
    if (_tempSensors) delete _tempSensors;
    if (_oneWire) delete _oneWire;
}

void TemperatureSensor::begin() {
    if (_pin == -1 || !_oneWire || !_tempSensors) {
        Serial.println("Temperature sensor not initialized: Invalid pin or no sensor configured");
        _state.watertemperature = -1.0f;
        return;
    }
    Serial.printf("Starting temperature sensor on GPIO %d...\n", _pin);
    _tempSensors->begin();
    _tempSensors->setResolution(12);
    _tempSensors->setWaitForConversion(true);
    int deviceCount = _tempSensors->getDS18Count();
    Serial.printf("Found %d temperature sensor(s).\n", deviceCount);
    if (deviceCount == 0) {
        Serial.println("No DS18B20 sensors detected, disabling temperature sensor");
        _state.watertemperature = -1.0f;
        return;
    }
    loop(); // Initial reading
}

void TemperatureSensor::loop() {
    if (_pin == -1 || !_oneWire || !_tempSensors || _tempSensors->getDS18Count() == 0) {
        _state.watertemperature = -1.0f;
        return;
    }
    unsigned long currentMillis = millis();
    if (currentMillis - _lastTemperatureRead >= _temperatureInterval) {
        _tempSensors->requestTemperatures();
        float tempC = _tempSensors->getTempCByIndex(0);
        if (tempC != DEVICE_DISCONNECTED_C && !isnan(tempC)) {
            _state.watertemperature = tempC;
            Serial.printf("DS18B20 TEMPERATURE: %.2f\n", tempC);
        } else {
            Serial.println("DS18B20 ERROR: Device disconnected or invalid reading");
            _state.watertemperature = -1.0f;
        }
        _lastTemperatureRead = currentMillis;
    }
}

float TemperatureSensor::getTemperature() const {
    return _state.watertemperature;
}

void TemperatureSensor::setPin(int pin) {
    if (_pin != pin) {
        if (_oneWire) delete _oneWire;
        if (_tempSensors) delete _tempSensors;
        _pin = pin;
        if (_pin != -1) {
            _oneWire = new OneWire(_pin);
            _tempSensors = new DallasTemperature(_oneWire);
            Serial.printf("Temperature sensor reassigned to GPIO %d\n", _pin);
            begin(); // Reinitialize sensor
        } else {
            _oneWire = nullptr;
            _tempSensors = nullptr;
            Serial.println("Temperature sensor disabled: Invalid pin (-1)");
        }
    }
}

int TemperatureSensor::getPin() const {
    return _pin;
}