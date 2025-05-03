#include <ESP32React.h>


ESP32React::ESP32React(AsyncWebServer* server) {


// Enable CORS if required
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", CORS_ORIGIN);
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Accept, Content-Type, Authorization");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Credentials", "true");
}


void ESP32React::begin() {

}


void ESP32React::loop() {
  
}
