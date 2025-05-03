#ifndef SensorMoisture_h
#define SensorMoisture_h

#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
class SensorMoisture{
 public:
  SensorMoisture(AsyncWebServer* server);
  
  void begin();
  void loop();
  void readSensor();
float getMoisture() const; // Getter for pH value

 private:
  struct SensorState {
        float moisture; // pH value
    } _state;
      AsyncWebServer* _server;
    
    unsigned long _lastReading = 0;
    const unsigned long moistureInterval = 5000;  // Interval in milliseconds


};

#endif