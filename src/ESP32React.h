#ifndef ESP32React_h
#define ESP32React_h


#include <Arduino.h>
#include <AsyncTCP.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

#define CORS_ORIGIN "*"



class ESP32React {
 public:
  ESP32React(AsyncWebServer* server);


  void begin();
  void loop();
};


#endif
