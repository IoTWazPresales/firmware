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
    // Handle the GET request for sensor data
     Serial.println("before serve");
    _server->on("/api/relay", HTTP_GET, [this](AsyncWebServerRequest *request) {
        // Create a JSON object to hold the sensor data
        DynamicJsonDocument jsonDoc(1024);
        float pumpState = _waterPump->getWaterPump();
        jsonDoc["pumpState"] = pumpState;
        String response;
        
        serializeJson(jsonDoc, response);
     
        request->send(200, "application/json", response);
        Serial.println("Request Sent");
    });

 _server->on("/api/relay/setThresholds", HTTP_POST, 
        [this](AsyncWebServerRequest *request){
            // No response here, handle it in the onRequestBody
        }, 
        nullptr,
        [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            Serial.println("Receiving JSON body data");



            // Convert incoming data to a String
            String body = String((char*)data, len);
            Serial.print("Received body: ");
            Serial.println(body);

            // Deserialize JSON
            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, body);
            if (error) {
                request->send(400, "application/json", "{\"status\":\"error\", \"message\":\"Invalid JSON\"}");
                return;
            }

               if (!doc.containsKey("minMoisture") || !doc.containsKey("maxMoisture")) {
                request->send(400, "application/json", "{\"status\":\"error\", \"message\":\"Missing parameters\"}");
                return;
            }


          float minMoisture = doc["minMoisture"].as<float>();
        float maxMoisture = doc["maxMoisture"].as<float>();
        Serial.print("minMoisture: ");
        Serial.println(minMoisture);
        Serial.print("maxMoisture: ");
        Serial.println(maxMoisture);

        
            if (minMoisture >= maxMoisture) {
                request->send(400, "application/json", "{\"status\":\"error\", \"message\":\"minMoisture must be less than maxMoisture\"}");
                return;
            }
 // Set the moisture thresholds
            _setPumpThreshold->setThresholds(minMoisture, maxMoisture);

            // Respond with success
            request->send(200, "application/json", "{\"status\":\"success\"}");
});
    



    // Optional: Handle preflight OPTIONS request
    Serial.println("handiling pre flight");
    _server->on("/api/relay", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
        Serial.println("sending preflight");
        request->send(204);
        Serial.println("preflight sent");
    });
}
