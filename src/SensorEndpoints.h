#ifndef SENSOR_ENDPOINTS_H
#define SENSOR_ENDPOINTS_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "SupabaseConnector.h"

#include "SensorManager.h"  // Include the full SensorManager to access all sensors

class SensorEndpoints {
public:
    SensorEndpoints(AsyncWebServer* server, SensorManager* manager, SupabaseConnector* supabase = nullptr);

private:
    AsyncWebServer* _server;
    SensorManager* _manager;
    SupabaseConnector* _supabase;

        void handleSensorData();

};

#endif // SENSOR_ENDPOINTS_H
