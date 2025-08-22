// Connector_Task.cpp
#include "Connector_Task.h"
#include <Arduino.h>
#include <esp_task_wdt.h>

void Connector_Manager(void* parameter) {
  auto* handles = static_cast<ConnectorHandles*>(parameter);
  auto* supa = handles->supabase;
  auto* mq   = handles->mqtt;
static unsigned long lastRes = 0;
 esp_task_wdt_add(NULL);
  // If you haven’t already called .begin() on these in setup, do it once here:
  supa->begin();
  mq->begin();

  for (;;) {
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
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}
