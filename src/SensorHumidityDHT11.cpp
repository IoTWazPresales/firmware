#include <SensorHumidityDHT11.h>



SensorHumidityDHT11::SensorHumidityDHT11(AsyncWebServer* server, uint8_t pin, uint8_t type) : _dht(pin, type), _server(server), _lastReading(0) {
    _state.humidity = -1; // Initialize the pH value
    _state.airtemp = -1; // Initialize the pH value
}

float SensorHumidityDHT11::getAirTemperature() const {
    return _state.airtemp;
}
float SensorHumidityDHT11::getHumidity() const {
    return _state.humidity;
}


void SensorHumidityDHT11::begin() {
  // _state.temperature = 999;
  // Serial.println(F("Init temperature sensor"));
  Serial.print("Starting DHT11 sensor...");
  _dht.begin();
  Serial.println("finished!");
  readSensor();
  
}

void SensorHumidityDHT11::loop() {
  unsigned long currentMillis = millis();
    if (currentMillis - _lastReading >= DHTInterval) {
        _lastReading = currentMillis;
        readSensor(); // Update humidity and temperature
        Serial.println("Sensor read executed."); // Debug output
    }

}

void SensorHumidityDHT11::readSensor() {
  Serial.println(F("Polling temperature sensors"));
  float airhumidity = _dht.readHumidity();
  float airtemperature = _dht.readTemperature();

  if (!isnan(airhumidity) && !isnan(airtemperature)) {
    Serial.print(F("DHT11 HUMIDITY: "));
    _state.humidity=airhumidity;
   
    Serial.println(airhumidity);
    Serial.print(F("DHT11 TEMPERATURE: "));
     _state.airtemp=airtemperature;
    Serial.println(airtemperature);

  } else {
    Serial.println(F("DHT11 ERROR"));
  }
}


