#ifndef SensorHumidityDHT11_h
#define SensorHumidityDHT11_h

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

class SensorHumidityDHT11 {
public:
    SensorHumidityDHT11(AsyncWebServer* server, uint8_t pin = 0, uint8_t type = DHT11);
    void setPin(uint8_t pin); // New method to set pin dynamically
    void begin();
    void loop();
    void readSensor();
    float getHumidity() const;
    float getAirTemperature() const;

private:
    struct SensorState {
        float humidity;
        float airtemp;
    } _state;
    DHT _dht;
    AsyncWebServer* _server;
    uint8_t _pin; // Store the pin number
    const unsigned long DHTInterval = 5000;
    unsigned long _lastReading = 0;
};

#endif