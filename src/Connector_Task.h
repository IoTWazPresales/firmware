// Connector_Task.h
#ifndef CONNECTOR_TASK_H
#define CONNECTOR_TASK_H

#include "SupabaseConnector.h"
#include "MQTTConnector.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <Arduino.h>

struct ConnectorHandles {
  SupabaseConnector* supabase;
  MQTTConnector*     mqtt;
};

struct HybridConnectionCommand {
  char apiKey[128];
};

extern QueueHandle_t gHybridConnectionQueue;

// FreeRTOS entry point:
void Connector_Manager(void* parameter);

bool enqueueHybridConnection(const String& apiKey);

#endif // CONNECTOR_TASK_H
