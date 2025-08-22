// WiFi_Task.cpp

#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <esp_task_wdt.h>

extern AsyncWebServer server;

// Your network credentials
static constexpr const char* TARGET_SSID     = "9532828 [2Ghz]";
static constexpr const char* TARGET_PASSWORD = "0611401627";

static bool     wifiReconnecting     = false;
static uint32_t wifiReconnectStart   = 0;
static const uint32_t WIFI_TIMEOUT_MS    = 30U * 1000U;  // 30 seconds
static const uint32_t WATCHDOG_TIMEOUT_S = 120;          // 2 minutes

// Forward declaration of our event callback
static void onWiFiEvent(WiFiEvent_t event);

void WiFi_Manager(void* parameter) {
    // Add this RTOS task to the watchdog once:
    esp_task_wdt_add(NULL);

    Serial.println("\n🔌 Starting WiFi Manager…");

    // Register for WiFi events
    WiFi.onEvent(onWiFiEvent);

    // Configure and start
    WiFi.mode(WIFI_STA);
    WiFi.setTxPower(WIFI_POWER_15dBm);
    WiFi.begin(TARGET_SSID, TARGET_PASSWORD);

    // Start AsyncWebServer only after we're connected (in the event handler)
    // Now just yield to the RTOS scheduler:
    for (;;) {
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

static void onWiFiEvent(WiFiEvent_t event) {
    switch (event) {
        case SYSTEM_EVENT_STA_GOT_IP:
            {
                // Connected!
                Serial.println("\n✅ WiFi Connected!");
                Serial.print("   IP Address: ");
                Serial.println(WiFi.localIP());

                // Start mDNS
                if (MDNS.begin("neurogrow")) {
                    Serial.println("🌐 mDNS responder started: http://neurogrow.local");
                } else {
                    Serial.println("⚠️ mDNS responder failed");
                }

                // Launch HTTP server
                server.begin();
                Serial.println("🚀 HTTP server started");

                // Reset reconnect state
                wifiReconnecting = false;
            }
            break;

        case SYSTEM_EVENT_STA_DISCONNECTED:
            {
                // Lost connection
                Serial.println("⚠️ WiFi disconnected");
                if (!wifiReconnecting) {
                    wifiReconnecting   = true;
                    wifiReconnectStart = millis();
                    Serial.println("🔄 Attempting reconnect…");
                    WiFi.reconnect();
                } else if (millis() - wifiReconnectStart >= WIFI_TIMEOUT_MS) {
                    Serial.println("❌ Reconnect timed out, restarting…");
                    ESP.restart();
                }
            }
            break;

        default:
            break;
    }
}
