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
    if (_sensorMgr) {
        _sensorMgr->begin();
    }
    if (_scannerMgr) {
        _scannerMgr->begin();
    }
    if (_relayMgr) {
        _relayMgr->begin();
    }
    if (_connectorHandles) {
        if (_connectorHandles->supabase) {
            _connectorHandles->supabase->begin();
        }
        if (_connectorHandles->mqtt) {
            _connectorHandles->mqtt->begin();
        }
    }

    if (!gHybridConnectionQueue) {
        gHybridConnectionQueue = xQueueCreate(4, sizeof(HybridConnectionCommand));
        if (!gHybridConnectionQueue) {
            Serial.println("❌ Failed to create hybrid connection queue");
        }
    }

     xTaskCreatePinnedToCore(
        WiFi_Manager,
        "WiFiManagerTask",
        6144,
        NULL,
        3,
        &_wifiHandle,
        0

    );
    xTaskCreatePinnedToCore(
        Sensor_Manager,
        "SensorManagerTask",
        6144,
        _sensorMgr,
        1,
        &_sensorHandle,
        1

    );
     xTaskCreatePinnedToCore(
        Scanner_Manager,
        "ScannerManagerTask",
        6144,
        _scannerMgr,
        1,
        &_scannerHandle,
        1

    );
    xTaskCreatePinnedToCore(
        Relay_Manager,
        "RelayManagerTask",
        6144,
        _relayMgr,
        1,
        &_relayHandle,
        1

    );
    xTaskCreatePinnedToCore(
        Connector_Manager,
        "ConnectorManagerTask",
        6144,
        _connectorHandles,
        2,
        &_connectorHandle,
        1

    );
   
   
}

 
