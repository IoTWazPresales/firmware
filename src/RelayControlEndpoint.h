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
                        RelayControl* setPumpThreshold
                                               
                        );
        
        void handleRelayData();
    


private:
    AsyncWebServer* _server;
    RelayControl* _setPumpThreshold;
    RelayControl* _waterPump;
};

#endif // SENSOR_ENDPOINTS_H
