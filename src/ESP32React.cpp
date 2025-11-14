#include "ESP32React.h"
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <Preferences.h>

ESP32React::ESP32React(AsyncWebServer* server) {
  // Must do this before server.begin()
  auto& headers = DefaultHeaders::Instance();
  // CORS: Allow specific origins (configurable via Preferences)
  Preferences prefs;
  prefs.begin("cors", true);
  String allowedOrigin = prefs.getString("origin", "*");
  prefs.end();
  headers.addHeader("Access-Control-Allow-Origin", allowedOrigin.length() > 0 ? allowedOrigin.c_str() : "*");
  headers.addHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS,PUT,DELETE");
  headers.addHeader("Access-Control-Allow-Headers",
                    "Content-Type,Authorization,x-device-api-key,X-Device-Api-Key");

  // catch all OPTIONS pre-flights
  server->on("/*", HTTP_OPTIONS, [](AsyncWebServerRequest *request){
    // no body, defaults get injected automatically
    request->send(200);
  });
}

void ESP32React::begin() {}
void ESP32React::loop()  {}
