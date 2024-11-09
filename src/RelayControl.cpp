#include <RelayControl.h>
 int readPin;
float status;
RelayControl::RelayControl(AsyncWebServer* server) : _server(server), _lastReading(0) {
    _state.waterPump = false; // Initialize the pH value
    _state.setPump = -1;
}

float RelayControl::getWaterPump() const {
    return _state.waterPump;
}



void RelayControl::setWaterPumpState() {

if (readPin = true)
{
    Serial.print("Pump is on");
}
else
{
  digitalWrite(0,HIGH);
  status=true;
}
 
  _state.waterPump = status;
}



void RelayControl::begin() {
  // _state.temperature = 999;
  // Serial.println(F("Init temperature sensor"));
  Serial.print("Starting Pump reader...");
  pinMode(2, INPUT);
  readPump();
 
}

void RelayControl::loop() {
  unsigned long currentMillis = millis();
  unsigned long manageElapsed = (unsigned long)(currentMillis - _lastReading);

  if (manageElapsed >= 5000) {
    _lastReading = currentMillis;
    readPump();
   
  }
}

void RelayControl::readPump() {
 
  
  readPin = digitalRead(0); //connect sensor to Analog 0
  if (readPin == LOW) {
        status = true;
        Serial.println("Pump is on");  // Pin state is HIGH (ON)
    } else {
        status = false;
        Serial.println("Pump is off"); // Pin state is LOW (OFF)
    }

  _state.waterPump = status;
}
