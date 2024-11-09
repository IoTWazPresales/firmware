#ifndef SensorRTC_h
#define SensorRTC_h

#include <string>
#include <Adafruit_BusIO_Register.h>
#include <RTClib.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>


  


class SensorRTC {
 public:

  SensorRTC(AsyncWebServer* server);
  void begin();
  void loop();
  void readSensor();
  String getRTC() const;

 private:

struct SensorState {
        String rtc; // pH value
        String timestamp;
    } _state;
      AsyncWebServer* _server;
        unsigned long _lastReading = 0;
    const unsigned long RTCInterval = 5000;  // Interval in milliseconds
  RTC_DS3231 _rtc;

 };

#endif
