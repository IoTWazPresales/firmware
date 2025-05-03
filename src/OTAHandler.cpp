    #include "OTAHandler.h"
    #include <ArduinoOTA.h>


    void OTAHandler::begin(const char* ssid, const char* password, const char* deviceName, const char* otaPassword) {
    // Start the OTA process and set up OTA handlers
    ArduinoOTA.setHostname(deviceName);   // Set hostname to identify the device
    ArduinoOTA.setPassword(otaPassword); // Optional password for security
    ArduinoOTA.begin();  // Initialize OTA functionality
    Serial.println("OTA Initialized");
    }

    void OTAHandler::handle() {
    ArduinoOTA.handle(); // Handle the OTA process
    }
