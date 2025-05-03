#ifndef DEVICESCANNER_H
#define DEVICESCANNER_H

#include <Arduino.h>
#include <ArduinoJson.h>

class DeviceScanner {
public:
    DeviceScanner();
    ~DeviceScanner();
    void begin();          // Perform initial scan at startup
    void loop();           // Optionally, perform periodic rescans
    String getScanResults();  // Returns a JSON string with the scan results
    void initializeAllPinsToSafeState();
    void scanI2CBus();
    void scanAnalogPins();
    void scanDigitalPins();
    void scanUARTInterfaces();
    void restorePinStates();
    uint8_t pinModes[40]; // Store previous pin states

private:
  
    // Additional scanning methods (SPI, GDI, etc.) can be added here

    StaticJsonDocument<2048> scanResults;
};

#endif // DEVICESCANNER_H
