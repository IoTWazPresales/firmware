#include "SupabaseSync.h"
#include <Arduino.h>

// how often we wake up to let SupabaseConnector::loop() drive its timers
static constexpr TickType_t SYNC_TASK_DELAY = pdMS_TO_TICKS(100);

static void supabaseSyncTask(void* pvParameters) {
  auto* supabase = static_cast<SupabaseConnector*>(pvParameters);
  for (;;) {
    // call the existing loop() which handles status-updates, sensor-syncs, relay-polls
    supabase->loop();
    // give other tasks (e.g. AsyncWebServer) CPU time
    vTaskDelay(SYNC_TASK_DELAY);
  }
}

void initSupabaseSyncTask(SupabaseConnector* connector) {
  // pin to core 0 so your main loop (core 1) stays super-snappy
  xTaskCreatePinnedToCore(
    supabaseSyncTask,      // task function
    "SupabaseSync",        // name
    16348,                  // stack size in bytes (adjust if you add more work)
    connector,             // pvParameters
    1,                     // priority
    nullptr,               // handle, if you need to vTaskDelete() later
    0                      // run on Core 0
  );
}
