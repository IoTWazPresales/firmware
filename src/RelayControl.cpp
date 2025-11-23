// RelayControl.cpp

#include "RelayControl.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <esp_task_wdt.h>

// Helper: "D10" → 10, etc.
static int stringPinToGPIO(const String& pin) {
  if (pin.startsWith("D")) {
    return pin.substring(1).toInt();
  }
  return pin.toInt();
}

RelayControl::RelayControl(AsyncWebServer* server, SensorManager* mgr)
  : _server(server), _mgr(mgr)
{}

void RelayControl::begin() {
    Serial.println(">>> RelayControl::begin()");
    // Feed the watchdog immediately
    esp_task_wdt_reset();

    // 1) Load on-disk config
    loadConfigFiles();
    yield();  // let the RTOS run
    esp_task_wdt_reset();

    Serial.println("    config files loaded");
    yield();
    esp_task_wdt_reset();

    // 2) For each relay ID, set up the pin if valid
    for (auto id : {"waterPump", "intakeFan", "exhaustFan", "lights"}) {
        esp_task_wdt_reset();
        Serial.print  ("    Loading config for: ");
        Serial.println(id);

        RelayConfig* rc = getRelayStruct(id);
        if (!rc) {
            Serial.println("      ↳ no such RelayConfig struct, skipping");
            continue;
        }

        // If pin was assigned (>=0), initialize it
        if (rc->pin >= 0) {
            Serial.printf("      ↳ setting up GPIO %d\n", rc->pin);
            pinMode(rc->pin, OUTPUT);
            digitalWrite(rc->pin, HIGH);
            rc->state = false;

            // Give a summary
            Serial.printf(
              "      → %-12s GPIO%2d  param='%s'  range[%.2f–%.2f]\n",
              id,
              rc->pin,
              rc->parameter.c_str(),
              rc->min, rc->max
            );
        } else {
            Serial.println("      ↳ pin < 0, skipping");
        }

        // Every iteration, yield & reset WDT
        yield();
        esp_task_wdt_reset();
    }

    Serial.println(">>> RelayControl::begin() complete");
}

void RelayControl::reloadConfig() {
    loadConfigFiles();
}

void RelayControl::loadConfigFiles() {
    // Feed watchdog
    esp_task_wdt_reset();

    // Load relays.json
    DynamicJsonDocument rd(2048);
    File rf = LittleFS.open("/relays.json", "r");
    if (rf && rf.size() > 0) {
        auto err = deserializeJson(rd, rf);
        rf.close();
        Serial.print("    deserialized /relays.json: ");
        Serial.println(err.c_str());
    } else {
        rd.createNestedArray("relays");
        Serial.println("    /relays.json missing or empty, using empty array");
    }

    // Load thresholds.json
    esp_task_wdt_reset();
    DynamicJsonDocument td(1024);
    File tf = LittleFS.open("/thresholds.json", "r");
    if (tf && tf.size() > 0) {
        auto err = deserializeJson(td, tf);
        tf.close();
        Serial.print("    deserialized /thresholds.json: ");
        Serial.println(err.c_str());
    } else {
        Serial.println("    /thresholds.json missing or empty");
    }

    // Merge into our RelayConfig structs
    esp_task_wdt_reset();
    for (JsonObject r : rd["relays"].as<JsonArray>()) {
        String id        = r["id"].as<String>();
        String pinLabel;
        // pin can be string "D10" or integer 10
        if (r["pin"].is<const char*>()) {
            pinLabel = r["pin"].as<String>();
        } else {
            pinLabel = String("D") + r["pin"].as<int>();
        }

        int pinNum = stringPinToGPIO(pinLabel);
        if (pinNum < 0) {
            Serial.printf("    ⚠️ Invalid pinLabel '%s' for id '%s'\n",
                          pinLabel.c_str(), id.c_str());
            continue;
        }

        // JSON uses "extractorFan", our struct is _exhaustFan
        String key = (id == "extractorFan") ? "exhaustFan" : id;
        RelayConfig* rc = getRelayStruct(key);
        if (!rc) {
            Serial.printf("    ⚠️ No RelayConfig for key '%s'\n", key.c_str());
            continue;
        }

        // Assign pin & parameter
        rc->pin       = pinNum;
        rc->parameter = r["parameter"].as<String>();

        // Pull thresholds if present
        if (td.containsKey(id)) {
            rc->min = td[id]["min"].as<float>();
            rc->max = td[id]["max"].as<float>();
        } else {
            rc->min = 0.0f;
            rc->max = 100.0f;
        }

        Serial.printf(
          "    ↳ loaded JSON entry '%s': pin=%s→GPIO%d, param=%s, range=%.2f–%.2f\n",
          id.c_str(),
          pinLabel.c_str(),
          pinNum,
          rc->parameter.c_str(),
          rc->min,
          rc->max
        );

        // Yield every iteration
        yield();
        esp_task_wdt_reset();
    }
}


void RelayControl::loop() {
    // Called every cycle
    for (auto id : {"waterPump","intakeFan","exhaustFan","lights"}) {
        RelayConfig* rc = getRelayStruct(id);
        if (!rc || rc->pin < 0 || rc->parameter.isEmpty()) {
            Serial.printf("Skipping %-12s (pin=%d param='%s')\n",
                          id, rc ? rc->pin : -1, rc ? rc->parameter.c_str() : "");
            continue;
        }

        // 1) read sensor
        float val = getSensorValue(rc->parameter);

        // 2) decide
        bool shouldBeOn = (val < rc->min || val > rc->max);

        // 3) ALWAYS print summary
        Serial.printf(
          "%-12s GPIO%2d '%s': val=%.2f  range[%.2f–%.2f] → %s\n",
          id, rc->pin,
          rc->parameter.c_str(),
          val,
          rc->min, rc->max,
          shouldBeOn ? "ON" : "OFF"
        );

        // 4) only trigger if changed
        if (shouldBeOn != rc->state) {
            // **If your relay module is active-LOW**, swap HIGH/LOW here
           digitalWrite(rc->pin, shouldBeOn ? LOW : HIGH);
            rc->state = shouldBeOn;
            Serial.printf("  >>> Relay %-10s turned %s\n",
                          id, shouldBeOn ? "ON" : "OFF");
        }
    }
    Serial.println(); // blank line between loops
}

float RelayControl::getSensorValue(const String& param) {
    if (!_mgr) return -1.0f;
    return _mgr->getParameterValue(param);
}

RelayControl::RelayConfig*
RelayControl::getRelayStruct(const String& id) {
    if      (id=="waterPump")   return &_waterPump;
    else if (id=="intakeFan")   return &_intakeFan;
    else if (id=="exhaustFan")  return &_exhaustFan;
    else if (id=="lights")      return &_lights;
    return nullptr;
}

const RelayControl::RelayConfig*
RelayControl::getRelayStruct(const String& id) const {
    return const_cast<RelayControl*>(this)->getRelayStruct(id);
}

bool RelayControl::getWaterPumpState()  const { return _waterPump.state; }
bool RelayControl::getIntakeFanState()  const { return _intakeFan.state; }
bool RelayControl::getExhaustFanState() const { return _exhaustFan.state; }
bool RelayControl::getLightsState()     const { return _lights.state; }

void RelayControl::setThresholds(const String& id, float min, float max) {
    String key = (id=="extractorFan" ? "exhaustFan" : id);
    if (auto* rc = getRelayStruct(key)) {
        rc->min = min;
        rc->max = max;
        Serial.printf("Set thresholds %-10s to [%.2f–%.2f]\n",
                      key.c_str(), min, max);
    }
}
