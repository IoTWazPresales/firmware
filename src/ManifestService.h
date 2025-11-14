#pragma once

#include <ESPAsyncWebServer.h>
#include "SensorManager.h"

class ManifestService {
public:
  ManifestService(AsyncWebServer* server, SensorManager* sensorManager);

private:
  SensorManager* _sensorManager;
  void handleListManifests(AsyncWebServerRequest* request);
};

