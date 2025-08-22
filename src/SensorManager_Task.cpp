#include <Arduino.h>
#include <ArduinoJson.h>
#include <SensorManager.h>
#include "SensorManager_Task.h"
#include <esp_task_wdt.h>




void Sensor_Manager(void *parameter) {
                esp_task_wdt_add(NULL);

        auto* mgr = static_cast<SensorManager*>(parameter);
 

        for (;;) {
                mgr->loop();

                esp_task_wdt_reset();
                vTaskDelay(pdMS_TO_TICKS(2000));
            }
}