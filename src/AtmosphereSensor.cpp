#include "AtmosphereSensor.h"

AtmosphereSensor::AtmosphereSensor(AsyncWebServer* server, int sdaPin, int sclPin) 
    : _server(server), _sdaPin(sdaPin), _sclPin(sclPin), _lastReading(0), _sensorAvailable(false) {
    _state.airquality = -1;
    _state.TVOC = -1;
    _state.CO2 = -1;
}

void AtmosphereSensor::begin() {
    if (_sdaPin == -1 || _sclPin == -1) {
        Serial.println("Atmosphere sensor SDA/SCL pins not assigned");
        return;
    }
    Serial.printf("Starting Atmosphere sensor on SDA: GPIO %d, SCL: GPIO %d...\n", _sdaPin, _sclPin);
    
    if (!_ens.begin()) {
        Serial.println("ENS160 initialization failed!");
        _sensorAvailable = false;
        _state.airquality = -1;
        _state.TVOC = -1;
        _state.CO2 = -1;
        return;
    }
    _sensorAvailable = true;
    if (_ens.setOperatingMode(SFE_ENS160_RESET)) {
        Serial.println("ENS160 reset successful.");
    }
    delay(100);
    _ens.setOperatingMode(SFE_ENS160_STANDARD);
    Serial.printf("Gas Sensor Status Flag: %d\n", _ens.getFlags());
    Serial.printf("Operating Mode: %d\n", _ens.getOperatingMode());

    _server->on("/api/sensor/atmosphere", HTTP_GET, [this](AsyncWebServerRequest *request) {
        DynamicJsonDocument doc(256);
        doc["airquality"] = _state.airquality;
        doc["TVOC"] = _state.TVOC;
        doc["CO2"] = _state.CO2;
        String json;
        serializeJson(doc, json);
        request->send(200, "application/json", json);
    });

    loop();
}

void AtmosphereSensor::loop() {
    if (!_sensorAvailable) {
        Serial.println("ENS160 sensor not initialized");
        return;
    }
    unsigned long currentMillis = millis();
    if (currentMillis - _lastReading >= _ensInterval) {
        if (!_ens.isConnected()) {
            Serial.println("ENS160 not connected");
            _sensorAvailable = false;
            return;
        }
        _state.airquality = _ens.getAQI();
        _state.TVOC = _ens.getTVOC();
        _state.CO2 = _ens.getECO2();
        if (!isnan(_state.airquality) && !isnan(_state.TVOC) && !isnan(_state.CO2)) {
            Serial.printf("Air Quality Index (1-5): %.2f\n", _state.airquality);
            Serial.printf("TVOC: %.2f\n", _state.TVOC);
            Serial.printf("CO2: %.2f\n", _state.CO2);
        } else {
            Serial.println("Failed to read ENS160 data");
        }
        _lastReading = currentMillis;
    }
}

float AtmosphereSensor::getAirQuality() const {
    return _state.airquality;
}

float AtmosphereSensor::getTVOC() const {
    return _state.TVOC;
}

float AtmosphereSensor::getCO2() const {
    return _state.CO2;
}