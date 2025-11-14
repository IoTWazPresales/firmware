#include "SensorEndpoints.h"
#include "SensorDataCollector.h"
#include "SupabaseConnector.h"
#include "Connector_Task.h"
#include <AsyncJson.h>
#include "AuthMiddleware.h"
#include "RateLimitMiddleware.h"

SensorEndpoints::SensorEndpoints(AsyncWebServer* server, SensorManager* manager, SupabaseConnector* supabase)
    : _server(server), _manager(manager), _supabase(supabase) {
    handleSensorData();
}

void SensorEndpoints::handleSensorData() {
    _server->on("/api/sensor", HTTP_GET, [this](AsyncWebServerRequest *request) {
        // Rate limiting (more lenient for sensor data)
        extern RateLimiter sensorRateLimiter;
        if (!RateLimitMiddleware::checkRateLimit(request, sensorRateLimiter, 120, 60000)) {
            return; // Response already sent
        }
        
        // Auth check (public read-only endpoint, but log access)
        if (!AuthMiddleware::checkApiKey(request)) {
            request->send(401, "application/json", "{\"status\":\"error\",\"message\":\"Unauthorized\"}");
            return;
        }
        AsyncJsonResponse* response = new AsyncJsonResponse(false, 2048);
        JsonObject root = response->getRoot();
        JsonObject values = root.createNestedObject("values");
        JsonArray meta = root.createNestedArray("meta");

        if (_manager) {
            _manager->fillValuesJson(values);
            _manager->describeCapabilities(meta);
            for (JsonPair kv : values) {
                root[kv.key()] = kv.value();
            }
        }

        if (_supabase && _supabase->isConnected() && _manager) {
            DynamicJsonDocument numericDoc(1536);
            JsonObject numeric = numericDoc.to<JsonObject>();
            _manager->fillNumericJson(numeric);
            enqueueSensorSyncEvent(numericDoc);
        }

        response->setLength();
        request->send(response);
    });

    _server->on("/api/sensor", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
        request->send(204);
    });
}
