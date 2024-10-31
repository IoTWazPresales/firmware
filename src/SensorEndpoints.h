/*#ifndef SensorEndpoints_h
#define SensorEndpoints_h

#include <ESPAsyncWebServer.h>
#include "SensorPH.h"
#include "TemperatureSensor.h"

class SensorEndpoints {
public:
    SensorEndpoints(AsyncWebServer* server, SensorPH* phSensor, TemperatureSensor* tempSensor) {
        // Setup endpoints for each sensor
        setupPHEndpoint(server, phSensor);
        setupTemperatureEndpoint(server, tempSensor);
           // Setup notFound handler
        server->onNotFound(notFound);
    }

private:
    void setupPHEndpoint(AsyncWebServer* server, SensorPH* phSensor) {
        // GET endpoint for pH sensor
        server->on("/api/ph", HTTP_GET, [&](AsyncWebServerRequest *request) {
            phSensor->readSensor();  // Read the current pH sensor value
            String jsonResponse;
            StaticJsonDocument<512> doc;
            doc["ph"] = phSensor->getPH(); // Access pH value
            serializeJson(doc, jsonResponse);
            sendResponse(request, jsonResponse);
        });
    }

    void setupTemperatureEndpoint(AsyncWebServer* server, TemperatureSensor* tempSensor) {
        // GET endpoint for temperature sensor
        server->on("/api/temperature", HTTP_GET, [&](AsyncWebServerRequest *request) {
            tempSensor->readSensor();  // Read the current temperature sensor value
            String jsonResponse;
            StaticJsonDocument<500> doc;
            doc["watertemperature"] = tempSensor->getTemperature(); // Access temperature value
            serializeJson(doc, jsonResponse);
            sendResponse(request, jsonResponse);
        });

     
    }


 void sendResponse(AsyncWebServerRequest *request, const String &jsonResponse) {
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonResponse);
        setCorsHeaders(response);
        request->send(response);
    }

    void setCorsHeaders(AsyncWebServerResponse *response) {
        response->addHeader("Access-Control-Allow-Origin", "*");
        response->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        response->addHeader("Access-Control-Allow-Headers", "Content-Type");
    }

    static void notFound(AsyncWebServerRequest *request) {
        if (request->method() == HTTP_OPTIONS) {
            request->send(200);
        } else {
            request->send(404, "application/json", "{\"message\":\"Not found\"}");
        }
    }





};

#endif*/




// SensorEndpoints.h

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
