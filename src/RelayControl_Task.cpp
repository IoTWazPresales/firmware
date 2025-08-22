#include <Arduino.h>
#include <ArduinoJson.h>
#include <RelayControl.h>
#include "RelayControl_Task.h"
#include <esp_task_wdt.h>





void Relay_Manager(void *parameter) {
            esp_task_wdt_add(NULL);

        auto* mgr = static_cast<RelayControl*>(parameter);
        mgr->begin();
        for (;;) {


           mgr->loop();
               
                
                esp_task_wdt_reset();
                vTaskDelay(pdMS_TO_TICKS(2000));
            }
}