// Connector_Task.cpp
#include "Connector_Task.h"
#include <Arduino.h>
#include <esp_task_wdt.h>

QueueHandle_t gHybridConnectionQueue = nullptr;

void Connector_Manager(void* parameter) {
  auto* handles = static_cast<ConnectorHandles*>(parameter);
  auto* supa = handles->supabase;
  auto* mq   = handles->mqtt;
  static unsigned long lastRes = 0;
  esp_task_wdt_add(NULL);

  for (;;) {
    HybridConnectionCommand cmd;
    if (gHybridConnectionQueue &&
        xQueueReceive(gHybridConnectionQueue, &cmd, 0) == pdPASS) {
      String apiKey = String(cmd.apiKey);
      if (apiKey.length() > 0) {
        Serial.println("🚀 Processing queued hybrid connection request");
        bool httpSuccess = supa->registerWithApiKey(apiKey);
        if (httpSuccess) {
          Serial.println("✅ Supabase registration completed from queue");
          if (mq) {
            mq->setDeviceCredentials(supa->getDeviceId(), apiKey);
          }
        } else {
          Serial.println("❌ Supabase registration failed from queue");
        }
      }
    }

    // Primary comms over MQTT:
    if (mq->isConnected()) {
      mq->loop();
    } else {
      // Fallback to HTTP/Supabase
      supa->loop();
    }
    
    if (millis() - lastRes >= 5000) {
        lastRes = millis();
        Serial.printf("Free heap: %lu\n", ESP.getFreeHeap());
        Serial.printf("Min free heap: %lu\n", ESP.getMinFreeHeap());
        Serial.printf("RSSI: %d\n", WiFi.RSSI());
         Serial.printf("HTTP Connected: %s\n", supa->isConnected() ? "✅" : "❌");
        Serial.printf("MQTT Connected: %s\n", mq->isConnected() ? "✅" : "❌");
    }
    
    // feed the watchdog if you have one
    esp_task_wdt_reset();
    // run this every 200ms
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

bool enqueueHybridConnection(const String& apiKey) {
  if (!gHybridConnectionQueue) {
    return false;
  }
  HybridConnectionCommand cmd{};
  apiKey.toCharArray(cmd.apiKey, sizeof(cmd.apiKey));
  BaseType_t res = xQueueSend(gHybridConnectionQueue, &cmd, 0);
  if (res != pdPASS) {
    Serial.println("⚠️ Hybrid connection queue full; request dropped");
    return false;
  }
  return true;
}
