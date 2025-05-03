#ifndef SpectralSensor_h
#define SpectralSensor_h

#include <Wire.h>
#include <Adafruit_AS7341.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#define I2C_SDA 21 // Define SDA pin (D2 or your specific pin)
#define I2C_SCL 22 // Define SCL pin (D3 or your specific pin)

extern Adafruit_AS7341 as7341;
extern TwoWire I2CW19564;
class SpectralSensor {
public:
    SpectralSensor(AsyncWebServer* server);
    void begin();
    void loop();
    void readSensor();
    float getLightChannel1() const;
    float getLightChannel2() const;
     float getLightChannel3() const;
    float getLightChannel4() const;
     float getLightChannel5() const;
    float getLightChannel6() const;
     float getLightChannel7() const;
    float getLightChannel8() const;
     float getBlueRatio() const;
    float getGreenRatio() const;
    float getRedRatio() const;
    float getFarRedRatio() const;
    float getTotalLight() const;
    float getChlorophyllIndexRedGreen() const;
float getChlorophyllIndexRedBlue() const;
float getGreenLightIntensity() const;
float getNDVI() const;
float getLux() const;
    // Add getters for other channels as needed

private:
    struct SensorState {
        float LightChannel1;
        float LightChannel2;
        float LightChannel3;
        float LightChannel4;
        float LightChannel5;
        float LightChannel6;
        float LightChannel7;
        float LightChannel8;
         float BlueRatio;
             float TotalLight; // Total light intensity

    float GreenRatio;
    float RedRatio;
    float FarRedRatio;
    float ChlorophyllIndexRedGreen;
    float ChlorophyllIndexRedBlue;
    float GreenLightIntensity;
    float NDVI;  
    float Lux; 
    } _state;
    Adafruit_AS7341 _as7341; // Sensor instance
    AsyncWebServer* _server;
    bool _sensorAvailable; // Flag for sensor presence
    uint8_t _address;
    unsigned long _lastReading;
    const unsigned long SpectralSensorInterval = 5000; // Interval in milliseconds
};

#endif
