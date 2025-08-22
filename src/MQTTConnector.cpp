#include "MQTTConnector.h"
#include "SensorManager.h"

// Static instance pointer for callback
MQTTConnector* MQTTConnector::instance = nullptr;

MQTTConnector::MQTTConnector() : _mqttClient(_wifiClient) {
    instance = this;
    
    // MQTT Broker settings - using HiveMQ Cloud defaults
    _mqttHost = "d89a599904dd4403b1708c005c270842.s1.eu.hivemq.cloud"; // User will need to update this
    _mqttPort = 8883; // TLS port
    _mqttUsername = ""; // Will be set from preferences
    _mqttPassword = ""; // Will be set from preferences
    
    _sensorManager = nullptr;
    _lastReconnectAttempt = 0;
    _lastSensorPublish = 0;
    _lastStatusPublish = 0;
    _reconnectInterval = 5000; // 5 seconds
    _sensorPublishInterval = 30000; // 30 seconds
    _statusPublishInterval = 60000; // 60 seconds
    _lastConnectionState = false;
}

void MQTTConnector::begin() {
    _prefs.begin("mqtt", false);
    _deviceId = _prefs.getString("device_id", "");
    _deviceApiKey = _prefs.getString("api_key", "");
    _mqttUsername = _prefs.getString("mqtt_user", "neurogrow_user"); // Default username
    _mqttPassword = _prefs.getString("mqtt_pass", "neurogrow_pass"); // Default password
    
    // Configure secure client
    _wifiClient.setInsecure(); // For now, skip cert validation
    
    // Set MQTT server
    _mqttClient.setServer(_mqttHost.c_str(), _mqttPort);
    _mqttClient.setCallback(mqttCallback);
    _mqttClient.setKeepAlive(60);
    
    Serial.println("🔗 MQTT Connector initialized");
    Serial.printf("📡 MQTT Host: %s:%d\n", _mqttHost.c_str(), _mqttPort);
    
    if (_deviceId.length() > 0) {
        Serial.printf("🆔 Device ID: %s\n", _deviceId.c_str());
        connectToMQTT();
    } else {
        Serial.println("⚠️ No device ID - MQTT connection will be established after device registration");
    }
}

void MQTTConnector::setSensorManager(SensorManager* manager) {
    _sensorManager = manager;
    Serial.println("✅ SensorManager linked to MQTT Connector");
}

void MQTTConnector::setDeviceCredentials(const String& deviceId, const String& apiKey) {
    _deviceId = deviceId;
    _deviceApiKey = apiKey;
    _prefs.putString("device_id", deviceId);
    _prefs.putString("api_key", apiKey);
    
    Serial.println("🔑 MQTT device credentials updated");
    
    // Try to connect to MQTT if WiFi is available
    if (WiFi.status() == WL_CONNECTED) {
        connectToMQTT();
    }
}

void MQTTConnector::loop() {
    if (!_mqttClient.connected() && WiFi.status() == WL_CONNECTED && _deviceId.length() > 0) {
        unsigned long now = millis();
        if (now - _lastReconnectAttempt > _reconnectInterval) {
            _lastReconnectAttempt = now;
            if (connectToMQTT()) {
                _lastReconnectAttempt = 0;
            }
        }
    }
    
    _mqttClient.loop();
    
    // Publish sensor data periodically
    unsigned long now = millis();
    if (_mqttClient.connected() && _sensorManager && (now - _lastSensorPublish > _sensorPublishInterval)) {
        _lastSensorPublish = now;
        collectAndPublishSensorData();
    }
    
    // Publish status periodically
    if (_mqttClient.connected() && (now - _lastStatusPublish > _statusPublishInterval)) {
        _lastStatusPublish = now;
        publishDeviceStatus("online", WiFi.localIP().toString());
    }
    
    // Track connection state changes
    bool currentState = _mqttClient.connected();
    if (currentState != _lastConnectionState) {
        _lastConnectionState = currentState;
        if (currentState) {
            Serial.println("✅ MQTT Connected");
            publishDeviceStatus("online", WiFi.localIP().toString());
        } else {
            Serial.println("❌ MQTT Disconnected");
        }
    }
}

bool MQTTConnector::connectToMQTT() {
    if (_deviceId.length() == 0) {
        Serial.println("⚠️ Cannot connect to MQTT: No device ID");
        return false;
    }
    
    Serial.println("🔄 Connecting to MQTT...");
    
    String clientId = "neurogrow_" + _deviceId;
    
    if (_mqttClient.connect(clientId.c_str(), _mqttUsername.c_str(), _mqttPassword.c_str())) {
        Serial.println("✅ Connected to MQTT broker");
        
        // Subscribe to relay command topic
        String relayTopic = getRelayCommandTopic();
        if (_mqttClient.subscribe(relayTopic.c_str())) {
            Serial.printf("📋 Subscribed to: %s\n", relayTopic.c_str());
        } else {
            Serial.printf("❌ Failed to subscribe to: %s\n", relayTopic.c_str());
        }
        
        return true;
    } else {
        Serial.printf("❌ MQTT connection failed, rc=%d\n", _mqttClient.state());
        return false;
    }
}

void MQTTConnector::collectAndPublishSensorData() {
    if (!_sensorManager) {
        Serial.println("⚠️ No SensorManager available for MQTT publish");
        return;
    }
    
    Serial.println("📊 Collecting sensor data for MQTT publish...");
    
    DynamicJsonDocument jsonDoc(JSON_OBJECT_SIZE(15) + 300);

    // Pull current sensor values through the SensorManager
    auto* tempSensor = _sensorManager->getFirstTemperatureSensor();
    auto* phSensor = _sensorManager->getFirstPH();
    auto* dhtSensor = _sensorManager->getFirstDHT11();
    auto* moistureSensor = _sensorManager->getFirstMoistureSensor();
    auto* tdsSensor = _sensorManager->getFirstTDS();
    auto* rtc = _sensorManager->getFirstRTC();
    auto* atmosphere = _sensorManager->getFirstAtmosphereSensor();
    auto* npk = _sensorManager->getFirstNPKSensor();

    jsonDoc["temperature"] = tempSensor ? tempSensor->getTemperature() : -1;
    jsonDoc["ph"] = npk ? npk->getPHSoil() : (phSensor ? phSensor->getPH() : -1);
    jsonDoc["humidity"] = dhtSensor ? dhtSensor->getHumidity() : -1;
    jsonDoc["airtemp"] = dhtSensor ? dhtSensor->getAirTemperature() : -1;
    jsonDoc["moisture"] = moistureSensor ? moistureSensor->getMoisture() : -1;
    jsonDoc["tdsSens"] = tdsSensor ? tdsSensor->getTDS() : -1;
    jsonDoc["airquality"] = atmosphere ? atmosphere->getAirQuality() : -1;
    jsonDoc["TVOC"] = atmosphere ? atmosphere->getTVOC() : -1;
    jsonDoc["CO2"] = atmosphere ? atmosphere->getCO2() : -1;
    jsonDoc["real"] = rtc ? rtc->getRTC() : "N/A";
    jsonDoc["nitro"] = npk ? npk->getNitrogen() : -1;
    jsonDoc["potas"] = npk ? npk->getPotassium() : -1;
    jsonDoc["phos"] = npk ? npk->getPhosphorus() : -1;
    jsonDoc["soilph"] = npk ? npk->getPHSoil() : -1;
    jsonDoc["lux"] = 0; // Simplified for now

    publishSensorData(jsonDoc);
}

void MQTTConnector::publishSensorData(const DynamicJsonDocument& sensorData) {
    if (!_mqttClient.connected()) {
        Serial.println("❌ Cannot publish sensor data: MQTT not connected");
        return;
    }
    
    String topic = getSensorTopic();
    String payload;
    serializeJson(sensorData, payload);
    
    if (_mqttClient.publish(topic.c_str(), payload.c_str())) {
        Serial.printf("📤 Published sensor data to: %s\n", topic.c_str());
    } else {
        Serial.printf("❌ Failed to publish sensor data to: %s\n", topic.c_str());
    }
}

void MQTTConnector::publishRelayStates(bool pumpState, bool intakeFanState, bool extractorFanState, bool lightsState) {
    if (!_mqttClient.connected()) return;
    
    DynamicJsonDocument doc(256);
    doc["pumpState"] = pumpState;
    doc["intakeFanState"] = intakeFanState;
    doc["extractorFanState"] = extractorFanState;
    doc["lightsState"] = lightsState;
    doc["timestamp"] = millis();
    
    String topic = getRelayStatusTopic();
    String payload;
    serializeJson(doc, payload);
    
    if (_mqttClient.publish(topic.c_str(), payload.c_str())) {
        Serial.printf("📤 Published relay states to: %s\n", topic.c_str());
    } else {
        Serial.printf("❌ Failed to publish relay states to: %s\n", topic.c_str());
    }
}

void MQTTConnector::publishDeviceStatus(const String& status, const String& localIP) {
    if (!_mqttClient.connected()) return;
    
    DynamicJsonDocument doc(256);
    doc["status"] = status;
    doc["local_ip"] = localIP;
    doc["uptime"] = millis();
    doc["free_heap"] = ESP.getFreeHeap();
    doc["wifi_rssi"] = WiFi.RSSI();
    doc["timestamp"] = millis();
    
    String topic = getDeviceStatusTopic();
    String payload;
    serializeJson(doc, payload);
    
    if (_mqttClient.publish(topic.c_str(), payload.c_str())) {
        Serial.printf("📤 Published device status to: %s\n", topic.c_str());
    } else {
        Serial.printf("❌ Failed to publish device status to: %s\n", topic.c_str());
    }
}

bool MQTTConnector::isConnected() {
    return _mqttClient.connected() && WiFi.status() == WL_CONNECTED && _deviceId.length() > 0;
}

String MQTTConnector::getConnectionStatus() {
    if (!WiFi.status() == WL_CONNECTED) return "WiFi Disconnected";
    if (_deviceId.length() == 0) return "No Device ID";
    if (!_mqttClient.connected()) return "MQTT Disconnected";
    return "Connected";
}

// Topic helper methods
String MQTTConnector::getSensorTopic() {
    return "neurogrow/" + _deviceId + "/sensors/data";
}

String MQTTConnector::getRelayCommandTopic() {
    return "neurogrow/" + _deviceId + "/relays/command";
}

String MQTTConnector::getRelayStatusTopic() {
    return "neurogrow/" + _deviceId + "/relays/status";
}

String MQTTConnector::getDeviceStatusTopic() {
    return "neurogrow/" + _deviceId + "/device/status";
}

// Static callback wrapper
void MQTTConnector::mqttCallback(char* topic, byte* payload, unsigned int length) {
    if (instance) {
        instance->onMqttMessage(topic, payload, length);
    }
}

void MQTTConnector::onMqttMessage(char* topic, byte* payload, unsigned int length) {
    String message = "";
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    
    Serial.printf("📥 MQTT message on topic: %s\n", topic);
    Serial.printf("📥 Message: %s\n", message.c_str());
    
    String topicStr = String(topic);
    
    // Handle relay commands
    if (topicStr == getRelayCommandTopic()) {
        DynamicJsonDocument doc(256);
        deserializeJson(doc, message);
        
        Serial.println("🔌 Processing relay command from MQTT");
        
        // Here you would integrate with your RelayControl system
        // For now, just log the command
        if (doc.containsKey("pump_state")) {
            Serial.printf("🚰 Pump command: %s\n", doc["pump_state"].as<bool>() ? "ON" : "OFF");
        }
        if (doc.containsKey("intake_fan_state")) {
            Serial.printf("🌪️ Intake fan command: %s\n", doc["intake_fan_state"].as<bool>() ? "ON" : "OFF");
        }
        if (doc.containsKey("extractor_fan_state")) {
            Serial.printf("💨 Extractor fan command: %s\n", doc["extractor_fan_state"].as<bool>() ? "ON" : "OFF");
        }
        if (doc.containsKey("lights_state")) {
            Serial.printf("💡 Lights command: %s\n", doc["lights_state"].as<bool>() ? "ON" : "OFF");
        }
    }
}