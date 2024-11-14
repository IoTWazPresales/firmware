#include <RelayControl.h>

 int readPin;
float status;

RelayControl::RelayControl(AsyncWebServer* server, SensorMoisture* soilMoisture) : _server(server), _soilMoisture(soilMoisture), _lastReading(0) {
    _state.waterPump = false; // Initialize the pH value
    _state.setPump = -1;
      _minMoisture = 0.0;        // Default minimum moisture threshold
    _maxMoisture = 100.0;      // Default maximum moisture threshold
}
float RelayControl::getWaterPump() const {
    return _state.waterPump;
}
void RelayControl::setWaterPumpState() {
    static bool previousState = false;  // Track the previous state

 if (_state.waterPump != previousState) {  // Only change if the state is different
        if (_state.waterPump) {
            Serial.println("Pump is on");
            digitalWrite(0, LOW);  // Assuming LOW is the active state for the pump
        } else {
            Serial.println("Pump is off");
            digitalWrite(0, HIGH); // Assuming HIGH is the inactive state for the pump
        }
        previousState = _state.waterPump;  // Update the previous state
    }
 
}
void RelayControl::begin() {
  // _state.temperature = 999;
  // Serial.println(F("Init temperature sensor"));
  Serial.print("Starting Pump reader...");
  pinMode(2, INPUT);
  pinMode(0, OUTPUT); // Ensure pin 0 is set as an output for the pump control

  readPump();
 
}

void RelayControl::loop() {
  unsigned long currentMillis = millis();
  unsigned long manageElapsed = (unsigned long)(currentMillis - _lastReading);

  if (manageElapsed >= 5000) {
    _lastReading = currentMillis;
    readPump();
   
  }
    float moistureLevel = _soilMoisture->getMoisture(); // Assuming the SensorMoisture instance is named _soilMoisture
    _state.setPump = moistureLevel; // Update the setPump value with the latest moisture level

 if (moistureLevel < _minMoisture) {
      _state.waterPump = true;  // Turn the pump on
    } else if (moistureLevel > _maxMoisture) {
      _state.waterPump = false; // Turn the pump off
    }

    setWaterPumpState();  // Apply the pump state (on/off)

}
void RelayControl::setThresholds(float minMoisture, float maxMoisture) {
    _minMoisture = minMoisture;
    _maxMoisture = maxMoisture;
    // Optionally, add logic to control the pump based on these new thresholds
    Serial.printf("Thresholds set: Min: %.2f, Max: %.2f\n", _minMoisture, _maxMoisture);
}
void RelayControl::readPump() {
 
  
  int readPin = digitalRead(0); //connect sensor to Analog 0
  if (readPin == LOW) {
    _state.waterPump = true;
    Serial.println("Pump is on");
  } else {
    _state.waterPump = false;
    Serial.println("Pump is off");
  }
}
