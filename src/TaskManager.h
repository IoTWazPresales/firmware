#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include "SensorManager.h"
#include "DeviceScanner.h"
#include "RelayControl.h"
#include "Connector_Task.h"

class TaskManager {
public:
    TaskManager(AsyncWebServer* server, SensorManager* sensorMgr, DeviceScanner* scannerMgr, RelayControl* relayMgr, ConnectorHandles* connectorHandles);
    void begin();
    void loop();
   
private:
    struct {
        
    } _state;
  AsyncWebServer* _server;
  SensorManager*  _sensorMgr;
  DeviceScanner* _scannerMgr; // pointer to the DeviceScanner instance
  RelayControl* _relayMgr; // pointer to the RelayControl instance
  ConnectorHandles* _connectorHandles; // pointer to the ConnectorHandles instance
  TaskHandle_t   _sensorHandle; // handle for the SensorManager task
  TaskHandle_t _wifiHandle;   // handle for the WiFi task
  TaskHandle_t _scannerHandle;   // handle for the Scanner task
  TaskHandle_t _relayHandle;   // handle for the Relay task
  TaskHandle_t _connectorHandle;   // handle for the Connector task
};






#endif