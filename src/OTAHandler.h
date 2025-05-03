#ifndef OTAHandler_h
#define OTAHandler_h

#include <ArduinoOTA.h>
#include <WiFi.h>

class OTAHandler {
public:
    OTAHandler() = default;
    void begin(const char* ssid, const char* password, const char* hostname = "ESP32", const char* otaPassword = nullptr);
    void handle();

private:
    const char* _hostname;
};

#endif
