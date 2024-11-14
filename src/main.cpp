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
#include "SensorHumidityDHT11.h"
#include "SensorMoisture.h"
#include "SensorTDS.h"
#include "SensorRTC.h"
#include "RelayControl.h"
#include "RelayControlEndpoint.h"
#include "WiFiStatus.h"
#include "WiFiScanner.h"
#include "WiFiSettingsService.h"
#include "WiFiCredentials.h"
#include "SystemStatus.h"
//SensorHumidityDHT11 sensorDHT11(DHT_PIN, DHT11); 
AsyncWebServer server(80);
WiFiSettingsService wifiSettingsService(&server);
SensorPH phSensor = SensorPH(&server);
TemperatureSensor tempSensor = TemperatureSensor(&server);
SensorHumidityDHT11 airTemp = SensorHumidityDHT11(&server, D3, DHT11);
SensorHumidityDHT11 airHumidity = SensorHumidityDHT11(&server, D3, DHT11);
    SensorMoisture soilMoisture = SensorMoisture(&server);
    SensorTDS tdsSensor = SensorTDS(&server);
    SensorRTC realtime = SensorRTC(&server);
    RelayControl waterPump = RelayControl(&server, &soilMoisture);
    RelayControl setPump = RelayControl(&server, &soilMoisture);
  WiFiStatus Wifistatus(&server);
  WiFiScanner Wifiscanner(&server);
    SystemStatus systemstatus(&server);
    SensorEndpoints endpoints(&server, &phSensor, &tempSensor, &airTemp, &airHumidity, &soilMoisture, &realtime, &tdsSensor);
    RelayControlEndpoint RelayEndpoints(&server, &waterPump, &setPump);




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
  wifiSettingsService.begin();
  // Once connected, print the local IP address
  Serial.println("Connected to WiFi!");
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());
  //sensorDHT11.begin();
   tempSensor.begin();
  phSensor.begin();
  airTemp.begin();
  airHumidity.begin();
  soilMoisture.begin();
  realtime.begin();
  tdsSensor.begin();
 waterPump.begin();
  setPump.begin();
   
 setPump.setThresholds(30.0, 80.0); // Set default thresholds
  server.begin();
Serial.println("Server Begun");
 

   server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
  server.serveStatic("/static/", SPIFFS, "/");

}
void loop() {
  // run the framework's loop function
   
  
  tempSensor.loop();
  phSensor.loop();
   airTemp.loop();
   soilMoisture.loop();
 realtime.loop();
  tdsSensor.loop();
  waterPump.loop();
   wifiSettingsService.loop();
  //sensorDHT11.loop();
}