#include "SensorMoisture.h"

SensorMoisture::SensorMoisture(AsyncWebServer* server, uint8_t pin)
    : _server(server), _pin(pin), _moisture(0.0), _lastReading(0) {
}

void SensorMoisture::begin() {
    pinMode(_pin, INPUT);
    _server->on("/api/sensor/moisture", HTTP_GET, [this](AsyncWebServerRequest *request) {
        String json = "{\"moisture\":" + String(_moisture, 2) + "}";
        request->send(200, "application/json", json);
    });
}

void SensorMoisture::loop() {
    if (millis() - _lastReading < _interval) return;
    _lastReading = millis();

    int rawValue = analogRead(_pin);
    // Map raw ADC (0-4095) to moisture percentage (0-100%)
    // Adjust based on your sensor's dry/wet calibration
    _moisture = map(rawValue, 4095, 0, 0, 100); // Dry: ~4095, Wet: ~0
    _moisture = constrain(_moisture, 0.0, 100.0);

    Serial.printf("Moisture: %.2f%%\n", _moisture);
}

float SensorMoisture::getMoisture() const {
    return _moisture;
}