#include <SystemStatus.h>


SystemStatus::SystemStatus(AsyncWebServer* server) {
  // Corrected lambda function syntax
  server->on(SYSTEM_STATUS_SERVICE_PATH, HTTP_GET, [this](AsyncWebServerRequest *request) {
    systemStatus(request);
  });}


void SystemStatus::systemStatus(AsyncWebServerRequest* request) {
  AsyncJsonResponse* response = new AsyncJsonResponse(false, MAX_ESP_STATUS_SIZE);
  JsonObject root = response->getRoot();

  root["esp_platform"] = "esp32";
  root["max_alloc_heap"] = ESP.getMaxAllocHeap();
  root["psram_size"] = ESP.getPsramSize();
  root["free_psram"] = ESP.getFreePsram();  
  root["cpu_freq_mhz"] = ESP.getCpuFreqMHz();
  root["free_heap"] = ESP.getFreeHeap();
  root["sketch_size"] = ESP.getSketchSize();
  root["free_sketch_space"] = ESP.getFreeSketchSpace();
  root["sdk_version"] = ESP.getSdkVersion();
  root["flash_chip_size"] = ESP.getFlashChipSize();
  root["flash_chip_speed"] = ESP.getFlashChipSpeed();

// TODO - Ideally this class will take an *FS and extract the file system information from there.
// ESP8266 and ESP32 do not have feature parity in FS.h which currently makes that difficult.

  root["fs_total"] = LittleFS.totalBytes();
  root["fs_used"] = LittleFS.usedBytes();

  response->setLength();
  request->send(response);
}
