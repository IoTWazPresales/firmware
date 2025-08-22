#ifndef SPECTRAL_SENSOR_H
#define SPECTRAL_SENSOR_H

#include <Arduino.h>
#include <Adafruit_AS7341.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

class SpectralSensor {
public:
    SpectralSensor(AsyncWebServer* server, int sdaPin = -1, int sclPin = -1);
    void begin();
    void loop();
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

private:
    struct {
        float LightChannel1;
        float LightChannel2;
        float LightChannel3;
        float LightChannel4;
        float LightChannel5;
        float LightChannel6;
        float LightChannel7;
        float LightChannel8;
        float BlueRatio;
        float GreenRatio;
        float RedRatio;
        float FarRedRatio;
        float TotalLight;
        float ChlorophyllIndexRedGreen;
        float ChlorophyllIndexRedBlue;
        float GreenLightIntensity;
        float NDVI;
        float Lux;
    } _state;
    Adafruit_AS7341 _as7341;
    AsyncWebServer* _server;
    bool _sensorAvailable;
    int _sdaPin;
    int _sclPin;
    unsigned long _lastReading;
    const unsigned long _spectralSensorInterval = 5000;
};

#endif