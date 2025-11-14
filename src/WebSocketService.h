#pragma once

#include <ESPAsyncWebServer.h>
#include <AsyncWebSocket.h>
#include "SensorManager.h"

class WebSocketService {
public:
  WebSocketService(AsyncWebServer* server, SensorManager* sensorManager);
  void begin();
  void loop();
  
private:
  AsyncWebSocket* _ws;
  SensorManager* _sensorManager;
  unsigned long _lastBroadcast;
  
  void onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, 
               AwsEventType type, void* arg, uint8_t* data, size_t len);
  void broadcastSensorData();
};

