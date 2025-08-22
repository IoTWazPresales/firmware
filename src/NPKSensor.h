// NPKSensor.h
#ifndef NPK_SENSOR_H
#define NPK_SENSOR_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

class NPKSensor {
public:
    NPKSensor(AsyncWebServer* server, int re_de = -1, int rx = -1, int tx = -1);
    void begin();
    void loop();
    float getNitrogen() const;
    float getPhosphorus() const;
    float getPotassium() const;
    float getPHSoil() const;
    float getSoilMoisture() const;
    float getSoilTemperature() const;
    float getConductivity() const;

private:
    struct {
        float nitrogen;
        float phosphorus;
        float potassium;
        float phsoil;
        float soilmoisture;
        float soiltemperature;
        float conductivity;
    } _state;
    AsyncWebServer* _server;
    HardwareSerial _serial;
    int _re_de;
    int _rx;
    int _tx;
    bool _sensorAvailable;
    unsigned long _lastReading;
    const unsigned long _npkInterval = 5000;
    void sendAndRead(uint8_t* query, size_t querySize, uint8_t* responseBuffer, size_t responseSize);
    void readNPKValues(uint8_t* response);
    void readPHValue(uint8_t* response);
    void readSoilMoisture(uint8_t* response);
    void readSoilTemperature(uint8_t* response);
    void readConductivity(uint8_t* response);
};

#endif