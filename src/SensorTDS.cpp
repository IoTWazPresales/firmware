#include <SensorTDS.h>



SensorTDS::SensorTDS(AsyncWebServer* server) : _server(server), _lastReading(0) {
    _state.tds = -1; // Initialize the pH value
}

float SensorTDS::getTDS() const {
    return _state.tds;
}

void SensorTDS::begin() {
  // _state.temperature = 999;
  // Serial.println(F("Init temperature sensor"));
  Serial.println("Starting TDS sensor...");
  readSensor();
 
}

void SensorTDS::loop() {
 
        readSensor(); // Update pH
   
}

void SensorTDS::readSensor() {
  float tds = analogRead(A1) / 40.95;  // convert to %
  Serial.println(F("TDS: "));
  Serial.println(tds);
  _state.tds = tds;
}




