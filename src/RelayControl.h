#pragma once

#include <ESPAsyncWebServer.h>
#include "SensorManager.h"

class RelayControl {
public:
    struct RelayConfig {
        int    pin       = -1;
        String parameter = "";
        float  min       = 0.0f;
        float  max       = 100.0f;
        bool   state     = false;
    };

    RelayControl(AsyncWebServer* server, SensorManager* mgr);

    void begin();
    void loop();
    void reloadConfig();  // Public method to reload config files

    bool getWaterPumpState()    const;
    bool getIntakeFanState()    const;
    bool getExhaustFanState()   const;
    bool getLightsState()       const;

    void setThresholds(const String& id, float min, float max);

private:
    AsyncWebServer* _server;
    SensorManager*  _mgr;

    RelayConfig _waterPump;
    RelayConfig _intakeFan;
    RelayConfig _exhaustFan;  // extractorFan
    RelayConfig _lights;

    void loadConfigFiles();
    void updateRelay(const String& id,
                     float value,
                     int pin,
                     float min,
                     float max,
                     bool& state);
    float getSensorValue(const String& param);

    RelayConfig*       getRelayStruct(const String& id);
    const RelayConfig* getRelayStruct(const String& id) const;
};
