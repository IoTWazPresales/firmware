#ifndef SENSOR_ENDPOINTS_H
#define SENSOR_ENDPOINTS_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "TemperatureSensor.h"
#include "SensorPH.h"
#include "SensorHumidityDHT11.h"
#include "SensorMoisture.h"
#include "SensorTDS.h"
#include "SensorRTC.h"
#include "AtmosphereSensor.h"
#include "SpectralSensor.h"
#include "NPKSensor.h"

class SensorEndpoints {
public:
         SensorEndpoints(AsyncWebServer* server, 
                        SensorPH* phSensor, 
                        TemperatureSensor* tempSensor, 
                        SensorHumidityDHT11* airData,
                        SensorMoisture* soilMoisture,
                        SensorRTC* realtime,
                        SensorTDS* tdsSensor,
                        AtmosphereSensor* _atmosphere,
                        SpectralSensor* spectralSensor,
                        NPKSensor* NPKval
                        );
        
        void handleSensorData();


private:
    AsyncWebServer* _server;
    SensorPH* _phSensor;
    TemperatureSensor* _tempSensor;
    SensorHumidityDHT11* _airData;
    SensorMoisture* _soilMoisture;
    SensorTDS* _tdsSensor;
    SensorRTC* _realtime;
    AtmosphereSensor* _atmosphere;
    SpectralSensor* _spectralSensor;
    NPKSensor* _NPKval;
};

#endif // SENSOR_ENDPOINTS_H
