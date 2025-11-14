#include "ManifestService.h"
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include "SensorManager.h"

ManifestService::ManifestService(AsyncWebServer* server, SensorManager* sensorManager)
    : _sensorManager(sensorManager) {
  server->on("/api/manifests", HTTP_GET, [this](AsyncWebServerRequest* request) {
    handleListManifests(request);
  });
  
  server->on("/api/manifests", HTTP_OPTIONS, [](AsyncWebServerRequest* request) {
    request->send(204);
  });
}

void ManifestService::handleListManifests(AsyncWebServerRequest* request) {
  AsyncJsonResponse* response = new AsyncJsonResponse(false, 1024);
  JsonObject root = response->getRoot();
  JsonArray manifests = root.createNestedArray("manifests");
  
  if (_sensorManager) {
    const auto& manifestList = _sensorManager->getManifests();
    root["count"] = manifestList.size();
    root["loaded"] = true;
    
    for (const auto& manifest : manifestList) {
      JsonObject m = manifests.createNestedObject();
      m["driver"] = manifest.driverId;
      m["label"] = manifest.label;
      m["unit"] = manifest.unit;
      m["autoDetect"] = manifest.autoDetect;
      
      JsonArray caps = m.createNestedArray("capabilities");
      for (const auto& cap : manifest.capabilities) {
        caps.add(cap);
      }
      
      JsonArray tags = m.createNestedArray("tags");
      for (const auto& tag : manifest.tags) {
        tags.add(tag);
      }
    }
  } else {
    root["count"] = 0;
    root["loaded"] = false;
    root["error"] = "SensorManager not available";
  }
  
  response->setLength();
  request->send(response);
}

