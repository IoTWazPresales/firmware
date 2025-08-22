#include <Arduino.h>
#include <ArduinoJson.h>
#include <DeviceScanner.h>
#include "Scanner_Task.h"
#include <esp_task_wdt.h>
#include <Wire.h>




void Scanner_Manager(void *parameter) {
          esp_task_wdt_add(NULL);

        auto* mgr = static_cast<DeviceScanner*>(parameter);
        const unsigned long scannerInterval  = 50000;
        unsigned long lastScannerRun         = 0;
        
        mgr->begin();
        for (;;) {


            if (millis() - lastScannerRun >= scannerInterval) {
                mgr->loop();
                lastScannerRun = millis();
            }
               
                esp_task_wdt_reset();
        
                vTaskDelay(pdMS_TO_TICKS(2000));
            }
}