#ifndef DEVICESCANNER_H
#define DEVICESCANNER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

class DeviceScanner {
public:
    DeviceScanner(AsyncWebServer* server);
    ~DeviceScanner();

    // One‐time setup + initial scan
    void begin();

    // Called in your FreeRTOS loop/task
    void loop();

    // JSON to return for GET /api/scan
    String getScanResults();

    // Individual scans
    void initializeAllPinsToSafeState();
    void scanI2CBus();
    void scanAnalogPins();
    void scanDigitalPins();
    void scanUARTInterfaces();
    
    // Enhanced signature collection
    void collectUnknownSensorSignatures();
    String getUnknownSensorSignatures();

private:
    AsyncWebServer*         _server;
    StaticJsonDocument<4096> scanResults;
    StaticJsonDocument<2048> unknownSignatures;
};

#endif // DEVICESCANNER_H
