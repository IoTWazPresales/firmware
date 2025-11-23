#ifndef SCANNER_ENDPOINTS_H
#define SCANNER_ENDPOINTS_H

#include <ESPAsyncWebServer.h>
#include "DeviceScanner.h"
#include "SensorManager.h"
#include "RelayControl.h"

// Bring in the global SensorManager and RelayControl instances from main.cpp
extern SensorManager sensorManager;
extern RelayControl relayControl;

class ScannerEndpoints {
public:
    ScannerEndpoints(AsyncWebServer* server, DeviceScanner* scanner);

private:
    AsyncWebServer* _server;
    DeviceScanner*  _scanner;

    // Set up GET /api/scan and OPTIONS
    void handleScanData();
    // Set up GET & POST /api/config and OPTIONS
    void handleConfig();
};

#endif // SCANNER_ENDPOINTS_H
