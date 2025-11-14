// Connector_Task.h
#ifndef CONNECTOR_TASK_H
#define CONNECTOR_TASK_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <ArduinoJson.h>
#include "SupabaseConnector.h"
#include "MQTTConnector.h"

struct ConnectorHandles {
  SupabaseConnector* supabase;
  MQTTConnector*     mqtt;
};

struct HybridConnectionCommand {
  char apiKey[128];
};

extern QueueHandle_t gHybridConnectionQueue;
extern QueueHandle_t gCloudEventQueue;

// FreeRTOS entry point:
void Connector_Manager(void* parameter);

bool enqueueHybridConnection(const String& apiKey);

enum class CloudEventType : uint8_t {
  SensorSync = 0,
  RelaySync  = 1,
  Status     = 2
};

struct CloudEvent {
  CloudEventType type;
  size_t length;
  char payload[256];
};

bool enqueueSensorSyncEvent(const DynamicJsonDocument& doc);
bool enqueueSensorSyncEvent(const JsonVariantConst& variant);


#endif // CONNECTOR_TASK_H
