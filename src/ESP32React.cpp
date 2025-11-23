#include "ESP32React.h"
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <Preferences.h>

ESP32React::ESP32React(AsyncWebServer* server) {
  // Don't access Preferences in constructor - do it in begin()
  // Just set up routes for now
  server->on("/*", HTTP_OPTIONS, [](AsyncWebServerRequest *request){
    request->send(200);
  });
}

void ESP32React::begin() {
  // Set up CORS headers after Preferences is safe to use
  auto& headers = DefaultHeaders::Instance();
  Preferences prefs;
  prefs.begin("cors", true);
  String allowedOrigin = prefs.getString("origin", "*");
  prefs.end();
  headers.addHeader("Access-Control-Allow-Origin", allowedOrigin.length() > 0 ? allowedOrigin.c_str() : "*");
  headers.addHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS,PUT,DELETE");
  headers.addHeader("Access-Control-Allow-Headers",
                    "Content-Type,Authorization,x-device-api-key,X-Device-Api-Key");
}
void ESP32React::loop()  {}
