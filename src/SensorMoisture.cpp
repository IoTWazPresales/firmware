#ifndef SENSOR_MOISTURE_H
#define SENSOR_MOISTURE_H
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
  Serial.println("Starting Moisture sensor...");
  readSensor();
 
}

void SensorMoisture::loop() {

    readSensor();
   
 
}

void SensorMoisture::readSensor() {
  float moisture = (4095.0 - analogRead(A2)) / 40.95;  // convert to %
  Serial.println(F("MOISTURE: "));
  Serial.println(moisture);
  _state.moisture = moisture;
}

#endif