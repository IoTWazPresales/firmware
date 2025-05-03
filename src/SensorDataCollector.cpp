#include "SensorDataCollector.h"
#include <ArduinoJson.h>

SensorDataCollector::SensorDataCollector(SensorPH* phSensor, 
                                         TemperatureSensor* tempSensor, 
                                         SensorHumidityDHT11* airData, 
                                         SensorMoisture* soilMoisture, 
                                         SensorRTC* realtime, 
                                         SensorTDS* tdsSensor,
                                         AtmosphereSensor* atmosphere)
    : _phSensor(phSensor), 
      _tempSensor(tempSensor),
      _airData(airData),
      _soilMoisture(soilMoisture),
      _realtime(realtime),
      _tdsSensor(tdsSensor),
      _atmosphere(atmosphere)
{}

String SensorDataCollector::collectSensorData() {
    // Collecting sensor data
    float temperature = _tempSensor->getTemperature();
    float ph = _phSensor->getPH();
    float humidity = _airData->getHumidity();
    float airtemp = _airData->getAirTemperature();
    float moisture = _soilMoisture->getMoisture();
    float tdsSens = _tdsSensor->getTDS();
    float airquality = _atmosphere->getAirQuality();
    float TVOC = _atmosphere->getTVOC();
    float CO2 = _atmosphere->getCO2();
    String real = _realtime->getRTC();

    // Create a JSON document to hold the sensor data
    DynamicJsonDocument jsonDoc(1024);
    jsonDoc["temperature"] = temperature;
    jsonDoc["humidity"] = humidity;
    jsonDoc["airtemp"] = airtemp;
    jsonDoc["ph"] = ph;
    jsonDoc["moisture"] = moisture;
    jsonDoc["tdsSens"] = tdsSens;
    jsonDoc["airquality"] = airquality;
    jsonDoc["TVOC"] = TVOC;
    jsonDoc["CO2"] = CO2;
    jsonDoc["real"] = real;

    // Serialize JSON and return the string
    String data;
    serializeJson(jsonDoc, data);
    return data;
}
