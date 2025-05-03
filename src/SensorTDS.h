#ifndef SensorTDS_h
#define SensorTDS_h
#include <WiFi.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
class SensorTDS {
 public:
  SensorTDS(AsyncWebServer* server);
  void begin();
  void loop();
  void readSensor();
    float getTDS() const; // Getter for pH value

 private:
    struct SensorState {
        float tds; // pH value
    } _state;
      AsyncWebServer* _server;
        unsigned long _lastReading = 0;
    const unsigned long TDSInterval = 5000;  // Interval in milliseconds


  
};

#endif



