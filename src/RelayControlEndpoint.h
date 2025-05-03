#ifndef RELAY_CONTROL_ENDPOINT_H
#define RELAY_CONTROL_ENDPOINT_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "RelayControl.h"

class RelayControlEndpoint {
public:
    RelayControlEndpoint(AsyncWebServer* server, 
                         RelayControl* waterPump, 
                         RelayControl* intakeFan, 
                         RelayControl* exhaustFan, 
                         RelayControl* setPumpThreshold,
                         RelayControl* setIntakeThreshold,
                         RelayControl* setExhaustTempThreshold,
                         RelayControl* setExhaustHumidityThreshold);

    void handleRelayData();

private:
    bool validateThresholds(DynamicJsonDocument& doc, AsyncWebServerRequest* request);

    AsyncWebServer* _server;
    RelayControl* _setPumpThreshold;
    RelayControl* _setIntakeThreshold;
    RelayControl* _setExhaustTempThreshold;
    RelayControl* _setExhaustHumidityThreshold;
    RelayControl* _waterPump;
    RelayControl* _intakeFan;
    RelayControl* _exhaustFan;

    // Member variables for the thresholds (optional, if you need to store them separately)
    float _minMoisture;
    float _maxMoisture;
    float _minTemp;
    float _maxTemp;
    float _minHumi;
    float _maxHumi;
    float _minCO2;
    float _maxCO2;
};

#endif // RELAY_CONTROL_ENDPOINT_H
