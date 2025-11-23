#include "SensorHumidityDHT11.h"

SensorHumidityDHT11::SensorHumidityDHT11(AsyncWebServer* server, uint8_t pin, uint8_t type)
    : _dht(pin, type), _server(server), _pin(pin), _lastReading(0) {
    _state.humidity = -1;
    _state.airtemp = -1;
}

void SensorHumidityDHT11::setPin(uint8_t pin) {
    if (_pin != pin) {
        _pin = pin;
        _dht = DHT(_pin, DHT11); // Reinitialize DHT with new pin
        Serial.printf("DHT11 reassigned to GPIO %d\n", _pin);
    }
}

float SensorHumidityDHT11::getAirTemperature() const {
    return _state.airtemp;
}

float SensorHumidityDHT11::getHumidity() const {
    return _state.humidity;
}

void SensorHumidityDHT11::begin() {
    // Validate GPIO pin number (ESP32 GPIO range: 0-39, but some pins are restricted)
    if (_pin > 39 || _pin < 0) {
        Serial.printf("ERROR: Invalid GPIO pin %d for DHT11 (must be 0-39)\n", _pin);
        return;
    }
    
    Serial.printf("Starting DHT11 sensor on GPIO %d...\n", _pin);
    _dht.begin();
    Serial.println("finished!");
    readSensor();
}

void SensorHumidityDHT11::loop() {
    readSensor();
}

void SensorHumidityDHT11::readSensor() {
    if (millis() - _lastReading >= DHTInterval) {
        float airhumidity = _dht.readHumidity();
        float airtemperature = _dht.readTemperature();

        if (!isnan(airhumidity) && !isnan(airtemperature)) {
            Serial.println(F("DHT11 HUMIDITY: "));
            _state.humidity = airhumidity;
            Serial.println(airhumidity);
            Serial.println(F("DHT11 TEMPERATURE: "));
            _state.airtemp = airtemperature;
            Serial.println(airtemperature);
        } else {
            Serial.println(F("DHT11 ERROR"));
        }
        _lastReading = millis();
    }
}