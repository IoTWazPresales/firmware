#include <SensorMoisture.h>


SensorMoisture::SensorMoisture(AsyncWebServer* server) : _server(server), _lastReading(0) {
    _state.moisture = -1; // Initialize the pH value
}

float SensorMoisture::getMoisture() const {
    return _state.moisture;
}




void SensorMoisture::begin() {
  // _state.temperature = 999;
  // Serial.println(F("Init temperature sensor"));
  Serial.print("Starting Moisture sensor...");
  readSensor();
 
}

void SensorMoisture::loop() {
  unsigned long currentMillis = millis();
  unsigned long manageElapsed = (unsigned long)(currentMillis - _lastReading);

  if (manageElapsed >= 5000) {
    _lastReading = currentMillis;
    readSensor();
   
  }
}

void SensorMoisture::readSensor() {
  float moisture = (4095.0 - analogRead(A0)) / 40.95;  // convert to %
  Serial.print(F("MOISTURE: "));
  Serial.println(moisture);
  _state.moisture = moisture;
}

