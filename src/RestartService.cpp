#include <RestartService.h>

RestartService::RestartService(AsyncWebServer* server) {
 server->on("/restart", HTTP_POST, [](AsyncWebServerRequest* request) {
        // Handle the restart logic here
        request->send(200, "application/json", "{\"success\":true}");
        ESP.restart(); // Restart the ESP device
    });
}

void RestartService::restart(AsyncWebServerRequest* request) {
  request->onDisconnect(RestartService::restartNow);
  request->send(200);
}
