#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

class TelemetryService {
 public:
  TelemetryService(AsyncWebServer* server);

 private:
  void handleTelemetry(AsyncWebServerRequest* request);
};
