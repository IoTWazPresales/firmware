#ifndef NPKSensor_h
#define NPKSensor_h

#include <HardwareSerial.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

class NPKSensor{
 public:

    
  NPKSensor(AsyncWebServer* server);
    void begin();
    void loop();
    void readNPK();
   float getNitrogen() const;      // Getter for humidity value
   float getPotassium() const;      // Getter for air temperature value
    float getPhosphorus() const;   
    float getPHSoil() const;   
 private:
  
   struct SensorState {
        float nitrogen; // pH value
        float potassium;
        float phosphorus;
        float phsoil;
    } _state;
  
      AsyncWebServer* _server;
    const unsigned long DHTInterval = 5000;  // Interval in milliseconds
    unsigned long _lastReading = 0;
    bool _sensorAvailable; // New flag to track sensor presence
    void sendAndRead(uint8_t* query, size_t querySize, uint8_t* responseBuffer, size_t responseSize);
    void printNPKValues();
    void printPHValue();
    void printSoilMoisture();
    void printSoilTemperature();
    void printConductivity();
};

#endif



