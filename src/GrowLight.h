#ifndef GROWLIGHT_h
#define GROWLIGHT_h

#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

class GrowLight{
 public:
  GrowLight(AsyncWebServer* server);
  
  void begin();
  void loop();
  void emit();
float getGrLight() const; // Getter for pH value

 private:
  struct SensorState {
        float grlight; // pH value
    } _state;
      AsyncWebServer* _server;
    
    unsigned long _lastReading = 0;
    const unsigned long lightInterval = 5000;  // Interval in milliseconds


};

#endif