#include <SensorHumidityDHT11.h>

void SensorHumidityDHT11::begin() {
  // _state.temperature = 999;
  // Serial.println(F("Init temperature sensor"));
  Serial.print("Starting DHT11 sensor...");
  _dht.begin();
  Serial.println("finished!");
  readSensor();
  onConfigUpdated();
}

void SensorHumidityDHT11::loop() {
  unsigned long currentMillis = millis();
  unsigned long manageElapsed = (unsigned long)(currentMillis - _lastReading);

  if (manageElapsed >= 5000) {
    _lastReading = currentMillis;
    readSensor();

  }
}

void SensorHumidityDHT11::readSensor() {
  // Serial.println(F("Polling temperature sensors"));
  float airhumidity = _dht.readHumidity();
  float airtemperature = _dht.readTemperature();

  if (!isnan(airhumidity) && !isnan(airtemperature)) {
    Serial.print(F("DHT11 HUMIDITY: "));
    Serial.println(airhumidity);
    Serial.print(F("DHT11 TEMPERATURE: "));
    Serial.println(airtemperature);

  } else {
    Serial.println(F("DHT11 ERROR"));
  }
}

void SensorHumidityDHT11::onConfigUpdated() {
  // digitalWrite(LED_PIN, _state.ledOn ? LED_ON : LED_OFF);
}
