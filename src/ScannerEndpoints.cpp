#include "ScannerEndpoints.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>

ScannerEndpoints::ScannerEndpoints(AsyncWebServer* server, DeviceScanner* scanner)
    : _server(server), _scanner(scanner) {
    handleScanData();
    handleConfig();
    handleConfigure();
}

void ScannerEndpoints::handleScanData() {
    _server->on("/api/scan", HTTP_GET, [this](AsyncWebServerRequest *request) {
        String json = _scanner->getScanResults();
        if (json.isEmpty() || json == "{}") {
            Serial.println("No scan data available, returning empty object");
            request->send(200, "application/json", "{}");
        } else {
            Serial.println("Sending scan results: " + json);
            request->send(200, "application/json", json);
        }
    });

    _server->on("/api/scan", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
        request->send(204);
    });
}

void ScannerEndpoints::handleConfig() {
    _server->on("/api/config", HTTP_GET, [](AsyncWebServerRequest *request) {
        File file = SPIFFS.open("/config.json", "r");
        DynamicJsonDocument doc(512);
        if (file) {
            deserializeJson(doc, file);
            file.close();
        }
        String response;
        serializeJson(doc, response);
        Serial.println("Sending config: " + response);
        request->send(200, "application/json", response);
    });

    _server->on("/api/config", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
        request->send(204);
    });
}

void ScannerEndpoints::handleConfigure() {
    _server->on("/api/configure", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (request->hasParam("body", true)) {
            DynamicJsonDocument doc(512);
            deserializeJson(doc, request->getParam("body", true)->value());
            String pin = doc["pin"];
            String sensorType = doc["sensorType"];

            // Load existing config
            File file = SPIFFS.open("/config.json", "r");
            DynamicJsonDocument config(512);
            if (file) {
                deserializeJson(config, file);
                file.close();
            }

            // Update config
            config[pin] = sensorType;

            // Save config
            file = SPIFFS.open("/config.json", "w");
            serializeJson(config, file);
            file.close();

            Serial.println("Saved assignment: pin=" + pin + ", sensorType=" + sensorType);
            request->send(200, "application/json", "{\"status\":\"success\"}");
        } else {
            Serial.println("Invalid configure request");
            request->send(400, "application/json", "{\"error\":\"Invalid request\"}");
        }
    });

    _server->on("/api/configure", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
        request->send(204);
    });
}