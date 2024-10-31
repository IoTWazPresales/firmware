#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <SensorHumidityDHT11.h>
#include <SensorPH.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "TemperatureSensor.h"
#include "SensorEndpoints.h"

//SensorHumidityDHT11 sensorDHT11(DHT_PIN, DHT11); 
AsyncWebServer server(80);
SensorPH phSensor = SensorPH(&server);
TemperatureSensor tempSensor = TemperatureSensor(&server);
    SensorEndpoints endpoints(&server, &phSensor, &tempSensor);





const char* ssid = "9532828 [2Ghz]";     // Replace with your network SSID
const char* password = "0611401627"; // Replace with your network password

void setup() {


Serial.begin(115200);
   if (!SPIFFS.begin(true)) { // Attempt to mount SPIFFS
        Serial.println("SPIFFS Mount Failed"); // Output error if mount fails
        return; // Exit if SPIFFS doesn't mount properly
    }
    File root = SPIFFS.open("/");
File file = root.openNextFile();
while (file) {
    Serial.print("File: ");
    Serial.println(file.name());
    file = root.openNextFile();
}
      Serial.println("SPIFFS Mounted Successfully");
  Serial.println("Starting Application");
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }

  // Once connected, print the local IP address
  Serial.println("Connected to WiFi!");
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());
  //sensorDHT11.begin();
   tempSensor.begin();
  phSensor.begin();



  



  server.begin();
Serial.println("Server Begun");


   server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
  server.serveStatic("/static/", SPIFFS, "/");
 /*sensorMoisture.begin();

  sensorTDS.begin();

  sensorPH.begin();

  sensorRTC.begin();

  sensorRelay.begin();*/

  // start the framework and demo project

}
void loop() {
  // run the framework's loop function
   
   
  tempSensor.loop();
  phSensor.loop();
  
 
  
  //sensorDHT11.loop();
}