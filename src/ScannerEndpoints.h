#ifndef SCANNER_ENDPOINTS_H
#define SCANNER_ENDPOINTS_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "DeviceScanner.h"

class ScannerEndpoints {
public:
    ScannerEndpoints(AsyncWebServer* server, DeviceScanner* scanner);
    void handleScanData();
    void handleConfig();
    void handleConfigure();

private:
    AsyncWebServer* _server;
    DeviceScanner* _scanner;
};

#endif // SCANNER_ENDPOINTS_H