#include "SensorSubmissionService.h"
#include "UnknownSensorDetector.h"
#include "DeviceScanner.h"
#include <AsyncJson.h>
#include <Preferences.h>
#include <LittleFS.h>

static Preferences prefs;
static const char* SUBMISSIONS_NAMESPACE = "sensor_sub";

struct SensorSubmission {
    uint16_t id;
    String userEmail;
    String sensorName;
    String sensorBrand;
    String datasheetUrl;
    String hardwareSignature;
    unsigned long timestamp;
    String status; // "pending", "in_review", "approved", "rejected", "integrated"
    String notes;
};

SensorSubmissionService::SensorSubmissionService(AsyncWebServer* server)
    : _server(server) {
}

void SensorSubmissionService::begin() {
    prefs.begin(SUBMISSIONS_NAMESPACE, false);
    
    // Submit unknown sensor for integration
    _server->on("/api/sensors/submit", HTTP_POST,
        [](AsyncWebServerRequest* req) {
            // Pre-handler - can add auth/rate limiting here
        },
        nullptr,
        [this](AsyncWebServerRequest* req, uint8_t* data, size_t len, size_t, size_t) {
            String body((char*)data, len);
            StaticJsonDocument<2048> doc;
            
            if (deserializeJson(doc, body)) {
                req->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
                return;
            }
            
            handleSubmitSensor(req, doc.as<JsonVariant>());
        }
    );
    
    // Get all submissions (for admin/review)
    _server->on("/api/sensors/submissions", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleGetSubmissions(request);
    });
    
    // Get submission status
    _server->on("/api/sensors/submissions/:id", HTTP_GET, [this](AsyncWebServerRequest* request) {
        String idStr = request->pathArg(0);
        uint16_t id = idStr.toInt();
        handleGetSubmissionStatus(request, id);
    });
    
    // OPTIONS for CORS
    _server->on("/api/sensors/submit", HTTP_OPTIONS, [](AsyncWebServerRequest* request) {
        request->send(204);
    });
    _server->on("/api/sensors/submissions", HTTP_OPTIONS, [](AsyncWebServerRequest* request) {
        request->send(204);
    });
}

void SensorSubmissionService::handleSubmitSensor(AsyncWebServerRequest* request, const JsonVariant& json) {
    AsyncJsonResponse* response = new AsyncJsonResponse(false, 1536);
    JsonObject root = response->getRoot();
    
    // Get next submission ID
    uint16_t nextId = prefs.getUShort("next_id", 1);
    
    // Extract submission data
    String userEmail = json["userEmail"] | "";
    String sensorName = json["sensorName"] | "Unknown Sensor";
    String sensorBrand = json["sensorBrand"] | "";
    String datasheetUrl = json["datasheetUrl"] | "";
    String hardwareSignature = json["hardwareSignature"] | "";
    
    // If hardware signature not provided, try to get from current scan
    if (hardwareSignature.length() == 0) {
        // This would need access to DeviceScanner - simplified for now
        hardwareSignature = "{}";
    }
    
    // Store submission
    String key = "sub_" + String(nextId);
    StaticJsonDocument<2048> submission;
    submission["id"] = nextId;
    submission["userEmail"] = userEmail;
    submission["sensorName"] = sensorName;
    submission["sensorBrand"] = sensorBrand;
    submission["datasheetUrl"] = datasheetUrl;
    submission["hardwareSignature"] = hardwareSignature;
    submission["timestamp"] = millis();
    submission["status"] = "pending";
    submission["notes"] = "";
    
    String submissionJson;
    serializeJson(submission, submissionJson);
    prefs.putString(key.c_str(), submissionJson);
    prefs.putUShort("next_id", nextId + 1);
    
    // Also store in LittleFS for persistence
    File file = LittleFS.open("/submissions/" + String(nextId) + ".json", "w");
    if (file) {
        file.print(submissionJson);
        file.close();
    }
    
    root["status"] = "success";
    root["submissionId"] = nextId;
    root["message"] = "Sensor submission received. We'll review it and notify you when it's integrated.";
    
    response->setLength();
    request->send(response);
    
    Serial.printf("📤 Sensor submission received: ID=%d, Name=%s\n", nextId, sensorName.c_str());
}

void SensorSubmissionService::handleGetSubmissions(AsyncWebServerRequest* request) {
    AsyncJsonResponse* response = new AsyncJsonResponse(false, 4096);
    JsonObject root = response->getRoot();
    JsonArray submissions = root.createNestedArray("submissions");
    
    // Read from Preferences
    uint16_t nextId = prefs.getUShort("next_id", 1);
    for (uint16_t i = 1; i < nextId; i++) {
        String key = "sub_" + String(i);
        String submissionJson = prefs.getString(key.c_str(), "");
        if (submissionJson.length() > 0) {
            StaticJsonDocument<512> doc;
            deserializeJson(doc, submissionJson);
            submissions.add(doc.as<JsonObject>());
        }
    }
    
    root["count"] = submissions.size();
    response->setLength();
    request->send(response);
}

void SensorSubmissionService::handleGetSubmissionStatus(AsyncWebServerRequest* request, uint16_t id) {
    AsyncJsonResponse* response = new AsyncJsonResponse(false, 1536);
    JsonObject root = response->getRoot();
    
    String key = "sub_" + String(id);
    String submissionJson = prefs.getString(key.c_str(), "");
    
    if (submissionJson.length() == 0) {
        root["status"] = "error";
        root["message"] = "Submission not found";
        response->setLength();
        request->send(404, "application/json", "");
        delete response;
        return;
    }
    
    StaticJsonDocument<512> doc;
    deserializeJson(doc, submissionJson);
    root = doc.as<JsonObject>();
    
    response->setLength();
    request->send(response);
}

