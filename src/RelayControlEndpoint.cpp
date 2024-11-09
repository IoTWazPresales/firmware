#include "RelayControlEndpoint.h"

RelayControlEndpoint::RelayControlEndpoint(AsyncWebServer* server, 
                                RelayControl* waterPump, 
                                RelayControl* setPumpThreshold
                                                               
                                ) 

    : _server(server), 
    _waterPump(waterPump), 
    _setPumpThreshold(setPumpThreshold)
    
    {
        Serial.println("initialize handler");
    handleRelayData(); // Initialize endpoint handlers
     Serial.println("handler initialized");
}

void RelayControlEndpoint::handleRelayData() {
    // Enable CORS for all endpoints
    Serial.println("initialize cors");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");
     Serial.println("cors initialized");
    // Handle the GET request for sensor data
     Serial.println("before serve");
    _server->on("/api/relay", HTTP_GET, [this](AsyncWebServerRequest *request) {
        // Create a JSON object to hold the sensor data
        Serial.println("before json");
        DynamicJsonDocument jsonDoc(1024);
Serial.println("after json");
        // Fetch sensor data from your sensor classes
        Serial.println("waterpump");
        float pumpState = _waterPump->getWaterPump();
        Serial.println("is waterpump");
         Serial.println("before fetch");
    
        


Serial.println("after PH fetch");
        // Populate the JSON objectx
        Serial.println("populate template temp");
        
        jsonDoc["pumpState"] = pumpState;
     
        

Serial.println("templates populated");
        // Send the JSON response
        Serial.println("Sending response");
        String response;
        Serial.println("serialize Json");
        serializeJson(jsonDoc, response);
        Serial.println("json serialized");
        Serial.println("Sending request");
        request->send(200, "application/json", response);
        Serial.println("Request Sent");
    });

    // Optional: Handle preflight OPTIONS request
    Serial.println("handiling pre flight");
    _server->on("/api/relay", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
        Serial.println("sending preflight");
        request->send(204);
        Serial.println("preflight sent");
    });
}
