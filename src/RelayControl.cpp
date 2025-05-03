#include <RelayControl.h>
const int PUMP_PIN = 0;
const int INTAKE_FAN_PIN = 26;
const int EXHAUST_FAN_PIN = 25;

RelayControl::RelayControl(AsyncWebServer* server, SensorMoisture* soilMoisture, SensorHumidityDHT11* airData, SensorPH* phSensor, AtmosphereSensor* CO2level) 
    : _server(server), _soilMoisture(soilMoisture), _airData(airData), _phSensor(phSensor),_CO2level(CO2level), _lastReading(0) {
    _state.waterPump = false;
    _state.exhaustFan = false;
    _state.intakeFan = false;
    _state.setPump = -1;
    _state.setExtractor = -1;
    _state.setIntake = -1;
  
    // Default thresholds
    _minMoisture = 0.0;
    _maxMoisture = 100.0;
    _minTemp = 0.0;
    _maxTemp = 100.0;
    _minHumi = 0.0;
    _maxHumi = 100.0;
    _minCO2 = 0.0;
    _maxCO2 = 2000.0;
}

float RelayControl::getMoistureMinThreshold() const {
    return _minMoisture;
}

float RelayControl::getMoistureMaxThreshold() const {
    return _maxMoisture;
}

float RelayControl::getCO2MinThreshold() const {
    return _minCO2;
}

float RelayControl::getCO2MaxThreshold() const {
    return _maxCO2;
}

float RelayControl::getTemperatureMinThreshold() const {
    return _minTemp;
}

float RelayControl::getTemperatureMaxThreshold() const {
    return _maxTemp;
}

float RelayControl::getHumidityMinThreshold() const {
    return _minHumi;
}

float RelayControl::getHumidityMaxThreshold() const {
    return _maxHumi;
}

float RelayControl::getExtractorFanState() const {
    return _state.exhaustFan;
}

float RelayControl::getWaterPumpState() const {
    return _state.waterPump;
}

float RelayControl::getIntakeFanState() const {
    return _state.intakeFan;
}

void RelayControl::begin() {
    pinMode(PUMP_PIN, OUTPUT);
    pinMode(INTAKE_FAN_PIN, OUTPUT);
    pinMode(EXHAUST_FAN_PIN, OUTPUT);
    readPump();
    readIntake();
    readExtractor();
}

void RelayControl::loop() {
    // Read current relay states
    readPump();
    readIntake();
    readExtractor();

    // Fetch sensor readings
   float moistureLevel = _soilMoisture ? _soilMoisture->getMoisture() : -1;
    float airTempLevel = _airData ? _airData->getAirTemperature() : -1;
    float airHumidityLevel = _airData ? _airData->getHumidity() : -1;
    float CO2 = _CO2level ? _CO2level->getCO2() : -1; 

    // Debug logs
    Serial.println("Moisture: ");
    Serial.println(moistureLevel);
    Serial.println(", Temp: ");
    Serial.println(airTempLevel);
    Serial.println(", Humidity: ");
    Serial.println(airHumidityLevel);
     Serial.println(", CO2: ");
    Serial.println(CO2);
    Serial.println(", Exhaust Fan State (Before): ");
    Serial.println(_state.exhaustFan ? "ON" : "OFF");

    // Update water pump state
    _state.setPump = moistureLevel;
    if (moistureLevel < _minMoisture - 2) { // Hysteresis: turn on below min - 2
        _state.waterPump = true;
    } else if (moistureLevel > _maxMoisture + 2) { // Hysteresis: turn off above max + 2
        _state.waterPump = false;
    }
    setWaterPumpState();

    // Update exhaust fan state based on temperature and humidity thresholds
    if (airTempLevel < _minTemp - 2) {
        _state.exhaustFan = true; // Turn on if either is below their respective min - hysteresis
    } else if (airTempLevel > _maxTemp +2) {
        _state.exhaustFan = false; // Turn off if both exceed their max + hysteresis
    }
    setExtractorFanState();

    if (CO2 < _minCO2 - 2) {
        _state.intakeFan = true; // Turn on if either is below their respective min - hysteresis
    } else if (CO2 > _maxCO2 + 2) {
        _state.intakeFan = false; // Turn off if both exceed their max + hysteresis
    }
    setIntakeFanState();

    // Debug logs after state update
    Serial.println("Exhaust Fan State (After): ");
    Serial.println(_state.exhaustFan ? "ON" : "OFF");
}


void RelayControl::setMoistureThresholds(float minMoisture, float maxMoisture) {
    _minMoisture = minMoisture;
    _maxMoisture = maxMoisture;
}

void RelayControl::setCO2Thresholds(float minCO2, float maxCO2) {
    _minCO2 = minCO2;
    _maxCO2 = maxCO2;
}

void RelayControl::setTemperatureThresholds(float minTemp, float maxTemp) {
    _minTemp = minTemp;
    _maxTemp = maxTemp;
}

void RelayControl::setHumidityThresholds(float minHumi, float maxHumi) {
    _minHumi = minHumi;
    _maxHumi = maxHumi;
}

void RelayControl::setWaterPumpState() {
    static bool previousState = false;
    if (_state.waterPump != previousState) {
        if (_state.waterPump) {
            digitalWrite(PUMP_PIN, LOW); // Active state
        } else {
            digitalWrite(PUMP_PIN, HIGH); // Inactive state
        }
        previousState = _state.waterPump;
    }
}

void RelayControl::setExtractorFanState() {
    static bool previousState = false;
    if (_state.exhaustFan != previousState) {
        if (_state.exhaustFan) {
            digitalWrite(EXHAUST_FAN_PIN, LOW);
        } else {
            digitalWrite(EXHAUST_FAN_PIN, HIGH);
        }
        previousState = _state.exhaustFan;
    }
}
void RelayControl::setIntakeFanState() {
    static bool previousState = false;
    if (_state.intakeFan != previousState) {
        if (_state.intakeFan) {
            digitalWrite(INTAKE_FAN_PIN, LOW);
        } else {
            digitalWrite(INTAKE_FAN_PIN, HIGH);
        }
        previousState = _state.intakeFan;
    }
}

void RelayControl::readPump() {
    if (digitalRead(PUMP_PIN) == LOW) {
        _state.waterPump = true;
    } else {
        _state.waterPump = false;
    }
}

void RelayControl::readIntake() {
    if (digitalRead(INTAKE_FAN_PIN) == LOW) {
        _state.intakeFan = true;
    } else {
        _state.intakeFan = false;
    }
}

void RelayControl::readExtractor() {
    if (digitalRead(EXHAUST_FAN_PIN) == LOW) {
        _state.exhaustFan = true;
    } else {
        _state.exhaustFan = false;
    }
}
