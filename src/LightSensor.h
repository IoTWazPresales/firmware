#ifndef LIGHTSENSOR_h
#define LIGHTSENSOR_h

#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

class SensorLight{
 public:
  SensorLight(AsyncWebServer* server);
  
  void begin();
  void loop();
  void readSensor();
float getLight() const; // Getter for pH value

 private:
  struct SensorState {
        float light; // pH value
    } _state;
      AsyncWebServer* _server;
    
    unsigned long _lastReading = 0;
    const unsigned long lightInterval = 5000;  // Interval in milliseconds


};

#endif