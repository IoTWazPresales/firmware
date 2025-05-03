#include "TemperatureSensor.h"
TemperatureSensor::TemperatureSensor(AsyncWebServer* server) : _server(server) {
    _state.watertemperature = -1; // Initialize temperature
}
float TemperatureSensor::getTemperature() const {
    return _state.watertemperature;
}
void TemperatureSensor::begin() {
    Serial.println("Starting temperature sensor...");
    _tempSensors.begin();
    _tempSensors.setResolution(12); // Set the resolution to 12-bit
    _tempSensors.setWaitForConversion(true);
    
    Serial.println("Found ");
    Serial.println(_tempSensors.getDS18Count());
    Serial.println(" temperature sensor(s).");

    readSensor(); // Initial read to get temperature
}

void TemperatureSensor::loop() {
    unsigned long currentMillis = millis();

    // Check if the interval has passed
    
        readSensor(); // Update temperature
    
}

void TemperatureSensor::readSensor() {
    _tempSensors.requestTemperatures(); // Request temperature reading from sensors

    float tempC = _tempSensors.getTempCByIndex(0); // Read temperature from the first sensor
    if (tempC != DEVICE_DISCONNECTED_C) {
        _state.watertemperature = tempC; // Store valid temperature reading
        Serial.println(F("DS18B20 TEMPERATURE: "));
        Serial.println(tempC);
    } else {
        Serial.println("DS18B20 ERROR: Device disconnected");
    }
}
