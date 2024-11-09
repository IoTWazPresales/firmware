#ifndef SensorHumidityDHT11_h
#define SensorHumidityDHT11_h

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>


class SensorHumidityDHT11{
 public:
  SensorHumidityDHT11(AsyncWebServer* server, uint8_t pin, uint8_t type);
    void begin();
    void loop();
    void readSensor();
   float getHumidity() const;      // Getter for humidity value
    float getAirTemperature() const;   // Getter for air temperature value

 private:
  
   struct SensorState {
        float humidity; // pH value
        float airtemp;
    } _state;
    DHT _dht;
      AsyncWebServer* _server;
    const unsigned long DHTInterval = 5000;  // Interval in milliseconds
    unsigned long _lastReading = 0;
  
};

#endif



