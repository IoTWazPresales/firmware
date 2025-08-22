// NPKSensor.cpp
#include "NPKSensor.h"

uint8_t npkQuery[] = {0x01, 0x03, 0x00, 0x1E, 0x00, 0x03, 0x65, 0xCD};
uint8_t phQuery[] = {0x01, 0x03, 0x00, 0x06, 0x00, 0x01, 0x64, 0x0B};
uint8_t soilMoistureQuery[] = {0x01, 0x03, 0x00, 0x12, 0x00, 0x01, 0x24, 0x0F};
uint8_t soilTemperatureQuery[] = {0x01, 0x03, 0x00, 0x13, 0x00, 0x01, 0x75, 0xCF};
uint8_t conductivityQuery[] = {0x01, 0x03, 0x00, 0x15, 0x00, 0x01, 0x95, 0xCE};

NPKSensor::NPKSensor(AsyncWebServer* server, int re_de, int rx, int tx) 
    : _server(server), _re_de(re_de), _rx(rx), _tx(tx), _serial(2), _lastReading(0), _sensorAvailable(false) {
    _state.nitrogen = -1;
    _state.phosphorus = -1;
    _state.potassium = -1;
    _state.phsoil = -1;
    _state.soilmoisture = -1;
    _state.soiltemperature = -1;
    _state.conductivity = -1;
}

void NPKSensor::begin() {
    if (_re_de == -1 || _rx == -1 || _tx == -1) {
        Serial.println("NPK sensor pins (RE_DE, RX, TX) not assigned");
        return;
    }
    Serial.printf("Starting NPK sensor on RE_DE: GPIO %d, RX: GPIO %d, TX: GPIO %d...\n", _re_de, _rx, _tx);
    _serial.begin(9600, SERIAL_8N1, _rx, _tx);
    pinMode(_re_de, OUTPUT);
    digitalWrite(_re_de, LOW);
    _sensorAvailable = true; // Assume connected; adjust based on actual detection if needed
    Serial.println(_sensorAvailable ? "NPK sensor initialized" : "NPK sensor failed to initialize");
    loop();
}

void NPKSensor::sendAndRead(uint8_t* query, size_t querySize, uint8_t* responseBuffer, size_t responseSize) {
    if (!_sensorAvailable) {
        Serial.println("NPK sensor not available, skipping UART operation");
        return;
    }
    digitalWrite(_re_de, HIGH);
    delay(10);
    _serial.write(query, querySize);
    _serial.flush();
    delay(10);
    digitalWrite(_re_de, LOW);
    delay(50);
    if (_serial.available() >= (int)responseSize) {
        _serial.readBytes(responseBuffer, responseSize);
    } else {
        Serial.println("No response or insufficient data from NPK sensor");
        memset(responseBuffer, 0, responseSize); // Clear buffer on failure
    }
}

void NPKSensor::readNPKValues(uint8_t* response) {
    sendAndRead(npkQuery, sizeof(npkQuery), response, 11);
    if (response[0] == 0x01 && response[1] == 0x03) {
        _state.nitrogen = ((response[3] << 8) | response[4]) * 1.0;
        _state.phosphorus = ((response[5] << 8) | response[6]) * 1.0;
        _state.potassium = ((response[7] << 8) | response[8]) * 1.0;
        Serial.printf("Nitrogen: %.2f, Phosphorus: %.2f, Potassium: %.2f\n", 
                      _state.nitrogen, _state.phosphorus, _state.potassium);
    } else {
        _state.nitrogen = -1;
        _state.phosphorus = -1;
        _state.potassium = -1;
        Serial.println("Invalid NPK response");
    }
}

void NPKSensor::readPHValue(uint8_t* response) {
    sendAndRead(phQuery, sizeof(phQuery), response, 7);
    if (response[0] == 0x01 && response[1] == 0x03) {
        _state.phsoil = ((response[3] << 8) | response[4]) / 100.0;
        Serial.printf("Soil pH: %.2f\n", _state.phsoil);
    } else {
        _state.phsoil = -1;
        Serial.println("Invalid pH response");
    }
}

void NPKSensor::readSoilMoisture(uint8_t* response) {
    sendAndRead(soilMoistureQuery, sizeof(soilMoistureQuery), response, 7);
    if (response[0] == 0x01 && response[1] == 0x03) {
        _state.soilmoisture = ((response[3] << 8) | response[4]) / 10.0;
        Serial.printf("Soil Moisture: %.2f%%\n", _state.soilmoisture);
    } else {
        _state.soilmoisture = -1;
        Serial.println("Invalid moisture response");
    }
}

void NPKSensor::readSoilTemperature(uint8_t* response) {
    sendAndRead(soilTemperatureQuery, sizeof(soilTemperatureQuery), response, 7);
    if (response[0] == 0x01 && response[1] == 0x03) {
        _state.soiltemperature = ((response[3] << 8) | response[4]) / 10.0;
        Serial.printf("Soil Temperature: %.2fC\n", _state.soiltemperature);
    } else {
        _state.soiltemperature = -1;
        Serial.println("Invalid temperature response");
    }
}

void NPKSensor::readConductivity(uint8_t* response) {
    sendAndRead(conductivityQuery, sizeof(conductivityQuery), response, 7);
    if (response[0] == 0x01 && response[1] == 0x03) {
        _state.conductivity = ((response[3] << 8) | response[4]) * 1.0;
        Serial.printf("Conductivity: %.2f uS/cm\n", _state.conductivity);
    } else {
        _state.conductivity = -1;
        Serial.println("Invalid conductivity response");
    }
}

void NPKSensor::loop() {
    if (!_sensorAvailable) return;
    unsigned long currentMillis = millis();
    if (currentMillis - _lastReading >= _npkInterval) {
        uint8_t response[11] = {0};
        readNPKValues(response);
        readPHValue(response);
        readSoilMoisture(response);
        readSoilTemperature(response);
        readConductivity(response);
        _lastReading = currentMillis;
    }
}

float NPKSensor::getNitrogen() const { return _state.nitrogen; }
float NPKSensor::getPhosphorus() const { return _state.phosphorus; }
float NPKSensor::getPotassium() const { return _state.potassium; }
float NPKSensor::getPHSoil() const { return _state.phsoil; }
float NPKSensor::getSoilMoisture() const { return _state.soilmoisture; }
float NPKSensor::getSoilTemperature() const { return _state.soiltemperature; }
float NPKSensor::getConductivity() const { return _state.conductivity; }