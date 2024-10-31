#ifndef TemperatureSensor_h
#define TemperatureSensor_h

#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESPAsyncWebServer.h>

class TemperatureSensor {
public:
    explicit TemperatureSensor(AsyncWebServer* server); // Only keep the server constructor

    void begin();
    void loop();
    void readSensor();
    float getTemperature() const;
    

private:
    struct {
        float watertemperature; // Temperature value
    } _state;

    OneWire _ds = OneWire(D2); // Setup OneWire on pin D2
    DallasTemperature _tempSensors = DallasTemperature(&_ds);
    AsyncWebServer* _server; // Server pointer
    unsigned long lastTemperatureRead = 0;          // Timer variable
    const unsigned long temperatureInterval = 5000; // Interval in milliseconds
};

#endif
