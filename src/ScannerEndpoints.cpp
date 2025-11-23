#include "ScannerEndpoints.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

// Reference the global SensorManager so we can reload config
extern SensorManager sensorManager;

ScannerEndpoints::ScannerEndpoints(AsyncWebServer* server, DeviceScanner* scanner)
  : _server(server), _scanner(scanner)
{
    // Don't register routes in constructor - do it in begin() after LittleFS is mounted
}

void ScannerEndpoints::begin() {
    handleScanData();
    handleConfig();
}

void ScannerEndpoints::handleScanData() {
    // GET /api/scan → JSON of last scan results
    _server->on("/api/scan", HTTP_GET, [this](AsyncWebServerRequest *request) {
        String json = _scanner->getScanResults();
        request->send(200, "application/json", json);
    });
    // CORS / preflight
    _server->on("/api/scan", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
        request->send(204);
    });
    _server->on("/api/scan/trigger", HTTP_POST, [this](AsyncWebServerRequest *request) {
        // synchronous scan
        _scanner->scanI2CBus();
        _scanner->scanAnalogPins();
        _scanner->scanDigitalPins();
        _scanner->scanUARTInterfaces();
        // send back the new data
        request->send(200, "application/json", _scanner->getScanResults());
    });
}

void ScannerEndpoints::handleConfig() {
    // GET /api/config → serve config.json
    _server->on("/api/config", HTTP_GET, [](AsyncWebServerRequest *request) {
        File f = LittleFS.open("/config.json", "r");
        DynamicJsonDocument doc(1024);
        if (f && f.size() > 0) {
            deserializeJson(doc, f);
            f.close();
        }
        String out;
        serializeJson(doc, out);
        request->send(200, "application/json", out);
    });

    // POST /api/config → overwrite config.json and reload SensorManager
    _server->on("/api/config", HTTP_POST,
        [](AsyncWebServerRequest *request) {}, // no upload handler
        nullptr,
        [](AsyncWebServerRequest *request, uint8_t* data, size_t len, size_t idx, size_t total) {
            DynamicJsonDocument doc(2048);
            auto err = deserializeJson(doc, data, len);
            if (err) {
                request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
                return;
            }
            // Overwrite the file
            File f = LittleFS.open("/config.json", "w");
            serializeJson(doc, f);
            f.close();
            // Immediately reload new config
            sensorManager.loadConfig();
            request->send(200, "application/json", "{\"status\":\"success\"}");
        }
    );

    // CORS / preflight
    _server->on("/api/config", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
        request->send(204);
    });
      // POST /api/config/reset → wipe config.json back to "{}"
    _server->on("/api/config/reset", HTTP_POST, [](AsyncWebServerRequest* request){
  Serial.println("🔄 /api/config/reset called — wiping config.json");
  if (!LittleFS.begin(true)) {
    Serial.println("❌ LittleFS mount failed");
    request->send(500, "application/json", "{\"status\":\"error\",\"message\":\"Filesystem mount failed\"}");
    return;
  }
  File f = LittleFS.open("/config.json", "w");
  if (!f) {
    Serial.println("❌ could not open config.json for reset");
    request->send(500, "application/json", "{\"status\":\"error\",\"message\":\"Could not open file\"}");
    return;
  }
  DynamicJsonDocument emptyDoc(1024);
  serializeJson(emptyDoc, f);
  f.close();
  Serial.println("✅ config.json overwritten with {}");
  sensorManager.loadConfig();
  request->send(200, "application/json", "{\"status\":\"success\"}");
});
// CORS preflight for reset
_server->on("/api/config/reset", HTTP_OPTIONS, [](AsyncWebServerRequest* request){
    request->send(204);
});


}
