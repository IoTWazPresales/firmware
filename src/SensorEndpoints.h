#ifndef SENSOR_ENDPOINTS_H
#define SENSOR_ENDPOINTS_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "TemperatureSensor.h"
#include "SensorPH.h"

class SensorEndpoints {
public:
         SensorEndpoints(AsyncWebServer* server, SensorPH* phSensor, TemperatureSensor* tempSensor);
        
        void handleSensorData();
    


private:
    AsyncWebServer* _server;
    SensorPH* _phSensor;
    TemperatureSensor* _tempSensor;

    
    
};

#endif // SENSOR_ENDPOINTS_H