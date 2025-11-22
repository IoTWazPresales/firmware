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
    }
    if (!gCloudEventQueue) {
        gCloudEventQueue = xQueueCreate(6, sizeof(CloudEvent));
    }

    // DISABLED: WiFi_Manager task - using WiFiSettingsService instead for AP mode support
    // xTaskCreatePinnedToCore(WiFi_Manager, "WiFiMgr", 3072, NULL, 3, &_wifiHandle, 0);
    _wifiHandle = nullptr;  // Mark as not used
    
    xTaskCreatePinnedToCore(Sensor_Manager, "SensorMgr", 3072, _sensorMgr, 1, &_sensorHandle, 1);
    xTaskCreatePinnedToCore(Scanner_Manager, "ScannerMgr", 3072, _scannerMgr, 1, &_scannerHandle, 1);
    xTaskCreatePinnedToCore(Relay_Manager, "RelayMgr", 3072, _relayMgr, 1, &_relayHandle, 1);
    xTaskCreatePinnedToCore(Connector_Manager, "ConnMgr", 3072, _connectorHandles, 2, &_connectorHandle, 1);
}

 
