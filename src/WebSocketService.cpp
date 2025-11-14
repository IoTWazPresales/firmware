#include "WebSocketService.h"
#include <ArduinoJson.h>
#include <AsyncWebSocket.h>

WebSocketService::WebSocketService(AsyncWebServer* server, SensorManager* sensorManager)
    : _ws(new AsyncWebSocket("/ws")), _sensorManager(sensorManager), _lastBroadcast(0) {
  server->addHandler(_ws);
  
  _ws->onEvent([this](AsyncWebSocket* server, AsyncWebSocketClient* client, 
                      AwsEventType type, void* arg, uint8_t* data, size_t len) {
    onEvent(server, client, type, arg, data, len);
  });
}

void WebSocketService::begin() {
  Serial.println("📡 WebSocket service started on /ws");
}

void WebSocketService::loop() {
  _ws->cleanupClients();
  
  // Broadcast sensor data every 2 seconds
  if (millis() - _lastBroadcast >= 2000) {
    _lastBroadcast = millis();
    broadcastSensorData();
  }
}

void WebSocketService::onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
                               AwsEventType type, void* arg, uint8_t* data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.printf("📡 WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    // Send initial data
    broadcastSensorData();
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.printf("📡 WebSocket client #%u disconnected\n", client->id());
  } else if (type == WS_EVT_DATA) {
    // Handle incoming messages if needed
    // For now, just echo back
    client->text("{\"type\":\"ack\"}");
  }
}

void WebSocketService::broadcastSensorData() {
  if (!_sensorManager || _ws->count() == 0) {
    return;
  }
  
  DynamicJsonDocument doc(1536);
  JsonObject values = doc.createNestedObject("values");
  JsonArray meta = doc.createNestedArray("meta");
  
  _sensorManager->fillValuesJson(values);
  _sensorManager->describeCapabilities(meta);
  
  doc["type"] = "sensor_data";
  doc["timestamp"] = millis();
  
  String payload;
  serializeJson(doc, payload);
  _ws->textAll(payload);
}

