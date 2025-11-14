#include <RestartService.h>
#include "AuthMiddleware.h"

RestartService::RestartService(AsyncWebServer* server) {
 server->on("/api/restart", HTTP_POST, [](AsyncWebServerRequest* request) {
        // Admin auth required
        if (!AuthMiddleware::checkAdminAuth(request)) {
            request->send(401, "application/json", "{\"status\":\"error\",\"message\":\"Unauthorized\"}");
            return;
        }
        // Handle the restart logic here
        request->send(200, "application/json", "{\"success\":true}");
        delay(100); // Allow response to send
        ESP.restart(); // Restart the ESP device
    });
}

void RestartService::restart(AsyncWebServerRequest* request) {
  request->onDisconnect(RestartService::restartNow);
  request->send(200);
}
