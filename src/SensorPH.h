#ifndef SensorPH_h
#define SensorPH_h

#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <OneWire.h>

class SensorPH {
public:
   
 SensorPH(AsyncWebServer* server);
    void begin();
    void loop();
    void readSensor();
    float getPH() const; // Getter for pH value
private:
    struct SensorState {
        float ph; // pH value
    } _state;
      AsyncWebServer* _server;
    OneWire _ds = OneWire(D2);
    unsigned long _lastReading = 0;
    const unsigned long pHInterval = 5000;  // Interval in milliseconds
};

#endif
