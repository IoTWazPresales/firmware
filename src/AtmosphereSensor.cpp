#include <AtmosphereSensor.h>
bool sensorInitialized;
int ensStatus;
#define I2C_SDA 21 // Define SDA pin
#define I2C_SCL 22 // Define SCL pin
AtmosphereSensor::AtmosphereSensor(AsyncWebServer* server) : _server(server), _lastReading(0) {

     _state.airquality = -1; // pH value
      _state.TVOC = -1;
       _state.CO2 = -1; // pH value
}

float AtmosphereSensor::getAirQuality() const {
    return _state.airquality;
}
float AtmosphereSensor::getTVOC() const {
    return _state.TVOC;
}
float AtmosphereSensor::getCO2() const {
    return _state.CO2;
}

void AtmosphereSensor::begin() {
  Serial.println("Starting Atmosphere sensor...");

  if (!_ens.begin()) {
    Serial.println("[ERROR] ENS160 initialization failed!");
    _sensorAvailable = false;
    _state.airquality = -1;
    _state.TVOC = -1;
    _state.CO2 = -1;
    return;
    }
    sensorInitialized = true;


    if (_ens.setOperatingMode(SFE_ENS160_RESET))
    Serial.println("ENS160 reset successful.");
     delay(100);
  _ens.setOperatingMode(SFE_ENS160_STANDARD);
  ensStatus = _ens.getFlags();
  Serial.println("Gas Sensor Status Flag: ");
  Serial.println(ensStatus);
  int mode = _ens.getOperatingMode();
  Serial.println("Current Operating Mode: ");
    Serial.println(mode);
      sensorInitialized = true;
  readSensor();
  
}

void AtmosphereSensor::loop() {
       if (!sensorInitialized) { // ✅ Skip if sensor failed to initialize
        Serial.println("[ERROR] ENS160 sensor not initialized. Skipping loop.");
        return;
    }
 
        readSensor(); // Update humidity and temperature
        Serial.println();
    

}

void AtmosphereSensor::readSensor() {
  if (!_ens.isConnected()) { // Check if ENS160 is available
        Serial.println("[ERROR] ENS160 sensor not initialized or not connected.");
        return; // Avoid crash
    }

  float airquality = _ens.getAQI();
  float airtvoc = _ens.getTVOC();
  float airco2 = _ens.getECO2();
  

  if (!isnan(airquality) && !isnan(airtvoc) && !isnan(airco2)) {
    Serial.println(F("Air Quality Index (1-5) : "));
    _state.airquality=airquality;
    Serial.println(airquality);
    
    Serial.println(F("Total Volatile Organic Compounds: "));
     _state.TVOC=airtvoc;
    Serial.println(airtvoc);
     Serial.println(F("CO2 concentration: "));
     _state.CO2=airco2;
    Serial.println(airco2);

  } else {
    Serial.println(F("[ERROR] Failed to read ENS160 data."));
  }
}


