#include "SensorEndpoints.h"

SensorEndpoints::SensorEndpoints(AsyncWebServer* server, 
                                SensorPH* phSensor, 
                                TemperatureSensor* tempSensor,
                                SensorHumidityDHT11* airTemp,
                                SensorHumidityDHT11* airHumidity,
                                SensorMoisture* soilMoisture,
                                SensorRTC* realtime,
                                SensorTDS* tdsSensor
                                                               
                                ) 

    : _server(server), 
    _phSensor(phSensor), 
    _tempSensor(tempSensor),
    _airTemp(airTemp),
    _airHumidity(airHumidity),
    _soilMoisture(soilMoisture),
    _realtime(realtime),
    _tdsSensor(tdsSensor)
    
    {
    handleSensorData(); // Initialize endpoint handler
}

void SensorEndpoints::handleSensorData() {
    // Enable CORS for all endpoint
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");
    // Handle the GET request for sensor data
    _server->on("/api/sensor", HTTP_GET, [this](AsyncWebServerRequest *request) {
        // Create a JSON object to hold the sensor data
        DynamicJsonDocument jsonDoc(1024);
        // Fetch sensor data from your sensor classes
        float temperature = _tempSensor->getTemperature();
        float ph = _phSensor->getPH();
        float humidity= _airHumidity->getHumidity();
        float airtemp= _airTemp->getAirTemperature();
        float moisture= _soilMoisture->getMoisture();
        float tdsSens= _tdsSensor->getTDS();
        String real= _realtime->getRTC();
        jsonDoc["temperature"] = temperature;
        jsonDoc["humidity"] = humidity;
        jsonDoc["airtemp"]= airtemp;
        jsonDoc["ph"] = ph;
        jsonDoc["moisture"] = moisture;
        jsonDoc["tdsSens"] = tdsSens;
        jsonDoc["real"] = real;
        
        String response;
        serializeJson(jsonDoc, response);
        request->send(200, "application/json", response);
    });

    // Optional: Handle preflight OPTIONS request
    _server->on("/api/sensor", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
        request->send(204);
    });
}
