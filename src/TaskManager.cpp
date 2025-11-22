#include "TaskManager.h"
#include <Arduino.h>
#include "WiFi_Task.h"
#include "SensorManager_Task.h"
#include "Scanner_Task.h"
#include "RelayControl_Task.h"
#include "Connector_Task.h"
#include <freertos/queue.h>

TaskManager::TaskManager(AsyncWebServer* server, SensorManager* sensorMgr, DeviceScanner* scannerMgr, RelayControl* relayMgr, ConnectorHandles* connectorHandles)
  : _server(server)
  ,_sensorMgr(sensorMgr)
  , _scannerMgr(scannerMgr)
  , _relayMgr(relayMgr)
  , _connectorHandles(connectorHandles)
  , _sensorHandle(nullptr)
  , _wifiHandle(nullptr)
  , _scannerHandle(nullptr)
  , _relayHandle(nullptr)
  , _connectorHandle(nullptr)
{}

void TaskManager::begin() {
    Serial.println("[TaskManager] Starting initialization...");
    Serial.flush();
    
    if (_sensorMgr) {
        Serial.println("[TaskManager] Starting SensorManager...");
        Serial.flush();
        _sensorMgr->begin();
        Serial.println("[TaskManager] SensorManager started OK");
        Serial.flush();
    }
    
    if (_scannerMgr) {
        Serial.println("[TaskManager] Starting DeviceScanner...");
        Serial.flush();
        _scannerMgr->begin();
        Serial.println("[TaskManager] DeviceScanner started OK");
        Serial.flush();
    }
    
    if (_relayMgr) {
        Serial.println("[TaskManager] Starting RelayControl...");
        Serial.flush();
        _relayMgr->begin();
        Serial.println("[TaskManager] RelayControl started OK");
        Serial.flush();
    }
    
    if (_connectorHandles) {
        if (_connectorHandles->supabase) {
            Serial.println("[TaskManager] Starting SupabaseConnector...");
            Serial.flush();
            _connectorHandles->supabase->begin();
        }
        if (_connectorHandles->mqtt) {
            Serial.println("[TaskManager] Starting MQTTConnector...");
            Serial.flush();
            _connectorHandles->mqtt->begin();
        }
    }
    
    Serial.println("[TaskManager] Creating queues...");
    Serial.flush();
    
    if (!gHybridConnectionQueue) {
        gHybridConnectionQueue = xQueueCreate(4, sizeof(HybridConnectionCommand));
        if (!gHybridConnectionQueue) {
            Serial.println("❌ Failed to create hybrid connection queue");
        } else {
            Serial.println("✅ Hybrid connection queue created");
        }
        Serial.flush();
    }
    if (!gCloudEventQueue) {
        gCloudEventQueue = xQueueCreate(6, sizeof(CloudEvent));
        if (!gCloudEventQueue) {
            Serial.println("❌ Failed to create cloud event queue");
        } else {
            Serial.println("✅ Cloud event queue created");
        }
        Serial.flush();
    }
    
    Serial.println("[TaskManager] Creating FreeRTOS tasks...");
    Serial.flush();

    Serial.println("[TaskManager] Creating WiFi task...");
    Serial.flush();
    xTaskCreatePinnedToCore(
        WiFi_Manager,
        "WiFiManagerTask",
        3072,
        NULL,
        3,
        &_wifiHandle,
        0
    );
    Serial.println("[TaskManager] WiFi task created");
    Serial.flush();
    
    Serial.println("[TaskManager] Creating Sensor task...");
    Serial.flush();
    xTaskCreatePinnedToCore(
        Sensor_Manager,
        "SensorManagerTask",
        3072,
        _sensorMgr,
        1,
        &_sensorHandle,
        1
    );
    Serial.println("[TaskManager] Sensor task created");
    Serial.flush();
    
    Serial.println("[TaskManager] Creating Scanner task...");
    Serial.flush();
    xTaskCreatePinnedToCore(
        Scanner_Manager,
        "ScannerManagerTask",
        3072,
        _scannerMgr,
        1,
        &_scannerHandle,
        1
    );
    Serial.println("[TaskManager] Scanner task created");
    Serial.flush();
    
    Serial.println("[TaskManager] Creating Relay task...");
    Serial.flush();
    xTaskCreatePinnedToCore(
        Relay_Manager,
        "RelayManagerTask",
        3072,
        _relayMgr,
        1,
        &_relayHandle,
        1
    );
    Serial.println("[TaskManager] Relay task created");
    Serial.flush();
    
    Serial.println("[TaskManager] Creating Connector task...");
    Serial.flush();
    xTaskCreatePinnedToCore(
        Connector_Manager,
        "ConnectorManagerTask",
        3072,
        _connectorHandles,
        2,
        &_connectorHandle,
        1
    );
    Serial.println("[TaskManager] Connector task created");
    Serial.flush();
    
    Serial.println("[TaskManager] All tasks created successfully");
    Serial.flush();
}

 
