// Connector_Task.h
#ifndef CONNECTOR_TASK_H
#define CONNECTOR_TASK_H

#include "SupabaseConnector.h"
#include "MQTTConnector.h"

struct ConnectorHandles {
  SupabaseConnector* supabase;
  MQTTConnector*     mqtt;
};

// FreeRTOS entry point:
void Connector_Manager(void* parameter);

#endif // CONNECTOR_TASK_H
