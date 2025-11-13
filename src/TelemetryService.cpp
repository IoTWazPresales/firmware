#include "TelemetryService.h"

#include <WiFi.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include "Connector_Task.h"

TelemetryService::TelemetryService(AsyncWebServer* server) {
  server->on("/api/telemetry", HTTP_GET, [this](AsyncWebServerRequest* request) {
    handleTelemetry(request);
  });
}

void TelemetryService::handleTelemetry(AsyncWebServerRequest* request) {
  AsyncJsonResponse* response = new AsyncJsonResponse(false, 512);
  JsonObject root = response->getRoot();

  root["uptime_ms"] = millis();
  root["free_heap"] = ESP.getFreeHeap();
  root["min_free_heap"] = ESP.getMinFreeHeap();
  root["max_alloc_heap"] = ESP.getMaxAllocHeap();
  root["wifi_connected"] = WiFi.status() == WL_CONNECTED;
  if (WiFi.status() == WL_CONNECTED) {
    root["wifi_rssi"] = WiFi.RSSI();
  }

  JsonObject queues = root.createNestedObject("queues");
  if (gHybridConnectionQueue) {
    UBaseType_t spaces = uxQueueSpacesAvailable(gHybridConnectionQueue);
    queues["hybrid_available"] = spaces;
  }
  if (gCloudEventQueue) {
    UBaseType_t spaces = uxQueueSpacesAvailable(gCloudEventQueue);
    queues["cloud_available"] = spaces;
  }

  response->setLength();
  request->send(response);
}
