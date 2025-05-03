#include "RelayControlEndpoint.h"
RelayControlEndpoint::RelayControlEndpoint(AsyncWebServer* server, 
                                            RelayControl* waterPump, 
                                            RelayControl* intakeFan, 
                                            RelayControl* exhaustFan, 
                                            RelayControl* setPumpThreshold,
                                            RelayControl* setIntakeThreshold,
                                            RelayControl* setExhaustTempThreshold,
                                            RelayControl* setExhaustHumidityThreshold) 
    : _server(server), 
      _waterPump(waterPump), 
      _intakeFan(intakeFan), 
      _exhaustFan(exhaustFan), 
      _setPumpThreshold(setPumpThreshold),
      _setIntakeThreshold(setIntakeThreshold),
      _setExhaustTempThreshold(setExhaustTempThreshold),
      _setExhaustHumidityThreshold(setExhaustHumidityThreshold)
{
    Serial.println("initialize handler");
    handleRelayData(); // Initialize endpoint handlers
    Serial.println("handler initialized");
}

void RelayControlEndpoint::handleRelayData() {
    // Handle the GET request for sensor data
    _server->on("/api/relay/GetDeviceStates", HTTP_GET, [this](AsyncWebServerRequest *request) {
        DynamicJsonDocument jsonDoc(1024);
        jsonDoc["pumpState"] = _waterPump->getWaterPumpState();
        jsonDoc["extractorFanState"] = _exhaustFan->getExtractorFanState();
        jsonDoc["intakeFanState"] = _intakeFan->getIntakeFanState();
        String response;
        serializeJson(jsonDoc, response);
        request->send(200, "application/json", response);
        Serial.println("Request Sent");
    });

    // Get thresholds
    _server->on("/api/relay/getThresholds", HTTP_GET, [this](AsyncWebServerRequest *request) {
        DynamicJsonDocument jsonDoc(1024);
        jsonDoc["minMoisture"] = _setPumpThreshold->getMoistureMinThreshold();
        jsonDoc["maxMoisture"] = _setPumpThreshold->getMoistureMaxThreshold();
        jsonDoc["minCO2"] = _setIntakeThreshold->getCO2MinThreshold();
        jsonDoc["maxCO2"] = _setIntakeThreshold->getCO2MaxThreshold();
        jsonDoc["minTemp"] = _setExhaustTempThreshold->getTemperatureMinThreshold();
        jsonDoc["maxTemp"] = _setExhaustTempThreshold->getTemperatureMaxThreshold();
        jsonDoc["minHumi"] = _setExhaustHumidityThreshold->getHumidityMinThreshold();
        jsonDoc["maxHumi"] = _setExhaustHumidityThreshold->getHumidityMaxThreshold();
        
        String response;
        serializeJson(jsonDoc, response);
        request->send(200, "application/json", response);
        Serial.println("Thresholds Sent");
    });

    // Set thresholds
    _server->on("/api/relay/setThresholds", HTTP_POST, 
        [this](AsyncWebServerRequest *request){
            // No response here, handle it in the onRequestBody
        }, 
        nullptr,
        [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            Serial.println("Receiving JSON body data");

            // Convert incoming data to a String
            String body = String((char*)data, len);
            Serial.println("Received body: ");
            Serial.println(body);

            // Deserialize JSON
            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, body);
            if (error) {
                request->send(400, "application/json", "{\"status\":\"error\", \"message\":\"Invalid JSON\"}");
                return;
            }

            // Validate thresholds
            if (!validateThresholds(doc, request)) {
                return;
            }

            // Extract values
            float minMoisture = doc["minMoisture"].as<float>();
            float maxMoisture = doc["maxMoisture"].as<float>();
            float minCO2 = doc["minCO2"].as<float>();
            float maxCO2 = doc["maxCO2"].as<float>();
            float minTemp = doc["minTemp"].as<float>();
            float maxTemp = doc["maxTemp"].as<float>();
            float minHumi = doc["minHumi"].as<float>();
            float maxHumi = doc["maxHumi"].as<float>();

            // Set the thresholds
            _setPumpThreshold->setMoistureThresholds(minMoisture, maxMoisture);
            _setIntakeThreshold->setCO2Thresholds(minCO2, maxCO2);
            _setExhaustTempThreshold->setTemperatureThresholds(minTemp, maxTemp);
            _setExhaustHumidityThreshold->setHumidityThresholds(minHumi, maxHumi);

            // Respond with success
            request->send(200, "application/json", "{\"status\":\"success\"}");
    });

    // Optional: Handle preflight OPTIONS request
    _server->on("/api/relay", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
        Serial.println("sending preflight");
        request->send(204);
        Serial.println("preflight sent");
    });
}

bool RelayControlEndpoint::validateThresholds(DynamicJsonDocument& doc, AsyncWebServerRequest* request) {
    if (!doc.containsKey("minMoisture") || !doc.containsKey("maxMoisture") ||
        !doc.containsKey("minCO2") || !doc.containsKey("maxCO2") ||
        !doc.containsKey("minTemp") || !doc.containsKey("maxTemp") ||
        !doc.containsKey("minHumi") || !doc.containsKey("maxHumi")) {
        request->send(400, "application/json", "{\"status\":\"error\", \"message\":\"Missing parameters\"}");
        return false;
    }

    // Check if min values are less than max values
    if (doc["minMoisture"].as<float>() >= doc["maxMoisture"].as<float>()) {
        request->send(400, "application/json", "{\"status\":\"error\", \"message\":\"minMoisture must be less than maxMoisture\"}");
        return false;
    }

    if (doc["minCO2"].as<float>() >= doc["maxCO2"].as<float>()) {
        request->send(400, "application/json", "{\"status\":\"error\", \"message\":\"minCO2 must be less than maxCO2\"}");
        return false;
    }

    if (doc["minTemp"].as<float>() >= doc["maxTemp"].as<float>()) {
        request->send(400, "application/json", "{\"status\":\"error\", \"message\":\"minTemp must be less than maxTemp\"}");
        return false;
    }

    if (doc["minHumi"].as<float>() >= doc["maxHumi"].as<float>()) {
        request->send(400, "application/json", "{\"status\":\"error\", \"message\":\"minHumi must be less than maxHumi\"}");
        return false;
    }

    return true;
}
