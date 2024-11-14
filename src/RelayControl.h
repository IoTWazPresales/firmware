#ifndef RelayControl_h
#define RelayControl_h

#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <SensorMoisture.h>
class RelayControl{
 public:
  RelayControl(AsyncWebServer* server, SensorMoisture* soilMoisture);
  
  void begin();
  void loop();
  void setWaterPumpState();
float getWaterPump() const; // Getter for pH value
void readPump();
float getChannelTwo() const; // Getter for pH value
float getChannelThree() const; // Getter for pH value
float getChannelFour() const; // Getter for pH value
 void setThresholds(float minMoisture, float maxMoisture);
 

 private:
   SensorMoisture* _soilMoisture;  // Declare pointer to SensorMoisture
   float _minMoisture;
    float _maxMoisture;

  struct RelayStates {
         bool waterPump;
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