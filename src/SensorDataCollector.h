#ifndef SENSORDATACOLLECTOR_H
#define SENSORDATACOLLECTOR_H

#include <Arduino.h>
#include "SensorPH.h"
#include "TemperatureSensor.h"
#include "SensorHumidityDHT11.h"
#include "SensorMoisture.h"
#include "SensorRTC.h"
#include "SensorTDS.h"
#include "AtmosphereSensor.h"

class SensorDataCollector {
public:
    // Constructor to initialize the SensorDataCollector with sensor objects
    SensorDataCollector(SensorPH* phSensor, 
                        TemperatureSensor* tempSensor, 
                        SensorHumidityDHT11* airData, 
                        SensorMoisture* soilMoisture, 
                        SensorRTC* realtime, 
                        SensorTDS* tdsSensor,
                        AtmosphereSensor* _atmosphere);

    // Method to collect sensor data and return it as a JSON string
    String collectSensorData();

private:
    SensorPH* _phSensor;
    TemperatureSensor* _tempSensor;
    SensorHumidityDHT11* _airData;
    SensorMoisture* _soilMoisture;
    SensorRTC* _realtime;
    SensorTDS* _tdsSensor;
    AtmosphereSensor* _atmosphere;
};

#endif
