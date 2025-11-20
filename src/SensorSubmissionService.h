#pragma once

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

class SensorSubmissionService {
public:
    SensorSubmissionService(AsyncWebServer* server);
    void begin();

private:
    AsyncWebServer* _server;
    
    void handleSubmitSensor(AsyncWebServerRequest* request, JsonVariant& json);
    void handleGetSubmissions(AsyncWebServerRequest* request);
    void handleGetSubmissionStatus(AsyncWebServerRequest* request, uint16_t id);
};

