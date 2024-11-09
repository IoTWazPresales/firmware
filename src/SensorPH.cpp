#include "SensorPH.h"

SensorPH::SensorPH(AsyncWebServer* server) : _server(server), _lastReading(0) {
    _state.ph = -1; // Initialize the pH value
}

float SensorPH::getPH() const {
    return _state.ph;
}

void SensorPH::begin() {
    Serial.println("Starting pH sensor...");
    readSensor(); // Initial read
}
void SensorPH::loop() {
    unsigned long currentMillis = millis();
  if (currentMillis - _lastReading >= pHInterval) {
        _lastReading = currentMillis;
        readSensor(); // Update pH
    }
}

void SensorPH::readSensor() {
    // Read the analog input and convert it to voltage
    float voltage = analogRead(A2) * 3.3 / 4096.0;  // Convert to voltage (3.3V ref, 12-bit ADC)
    
    // Calculate the pH value based on voltage
    _state.ph = ((voltage - 0.97) * 7.0) + 7.0; // Adjust these constants based on calibration
    
    // Log the pH value
    Serial.print(F("pH: "));
    Serial.println(getPH());
}


