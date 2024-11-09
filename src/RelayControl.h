#ifndef RelayControl_h
#define RelayControl_h

#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

class RelayControl{
 public:
  RelayControl(AsyncWebServer* server);
  
  void begin();
  void loop();
  void readPump();
  void setWaterPumpState();
float getWaterPump() const; // Getter for pH value

float getChannelTwo() const; // Getter for pH value
float getChannelThree() const; // Getter for pH value
float getChannelFour() const; // Getter for pH value


 private:
  struct RelayStates {
        float waterPump;
        float setPump;
        float channelTwo;
        float channelThree;
        float channelFour; // pH value
    } _state;
      AsyncWebServer* _server;
    
    unsigned long _lastReading = 0;
    const unsigned long moistureInterval = 5000;  // Interval in milliseconds


};

#endif