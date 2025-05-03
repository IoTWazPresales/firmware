#ifndef RelayControl_h
#define RelayControl_h

#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <SensorMoisture.h>
#include <SensorPH.h>
#include <SensorHumidityDHT11.h>
#include <TemperatureSensor.h>
#include <AtmosphereSensor.h>


class RelayControl {
public:
    RelayControl(AsyncWebServer* server, SensorMoisture* soilMoisture, SensorHumidityDHT11* airData, SensorPH* phSensor, AtmosphereSensor* CO2level);

    void begin();
    void loop();

    void readPump();
    void readExtractor();
    void readIntake();

    float getMoistureMinThreshold() const;
    float getMoistureMaxThreshold() const;
    float getCO2MinThreshold() const;
    float getCO2MaxThreshold() const;
    float getTemperatureMinThreshold() const;
    float getTemperatureMaxThreshold() const;
    float getHumidityMinThreshold() const;
    float getHumidityMaxThreshold() const;
    float getExtractorFanState() const;
    float getWaterPumpState() const;
    float getIntakeFanState() const;

    void setWaterPumpState();
    void setExtractorFanState();
    void setIntakeFanState();

    // Set thresholds
    void setMoistureThresholds(float minMoisture, float maxMoisture);
    void setCO2Thresholds(float minCO2, float maxCO2);
    void setTemperatureThresholds(float minTemp, float maxTemp);
    void setHumidityThresholds(float minHumi, float maxHumi);

private:
    SensorMoisture* _soilMoisture;
    SensorHumidityDHT11* _airData;
    SensorPH* _phSensor;
    AtmosphereSensor* _CO2level;
    
    float _minMoisture;
    float _maxMoisture;
    float _minCO2;
    float _maxCO2;
    float _minTemp;
    float _maxTemp;
    float _minHumi;
    float _maxHumi;

    struct RelayStates {
        bool waterPump;
        bool exhaustFan;
        bool intakeFan;
        float setPump;
        float setExtractor;
        float setIntake;
        float channelFour; // pH value
    } _state;

    AsyncWebServer* _server;
    unsigned long _lastReading = 0;
    const unsigned long moistureInterval = 5000;  // Interval in milliseconds
};

#endif
