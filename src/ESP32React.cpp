#include "ESP32React.h"
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

ESP32React::ESP32React(AsyncWebServer* server) {
  // Must do this before server.begin()
  auto& headers = DefaultHeaders::Instance();
  headers.addHeader("Access-Control-Allow-Origin",  "*");
  headers.addHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
  headers.addHeader("Access-Control-Allow-Headers",
                    "Content-Type,Authorization,x-device-api-key");

  // catch all OPTIONS pre-flights
  server->on("/*", HTTP_OPTIONS, [](AsyncWebServerRequest *request){
    // no body, defaults get injected automatically
    request->send(200);
  });
}

void ESP32React::begin() {}
void ESP32React::loop()  {}
