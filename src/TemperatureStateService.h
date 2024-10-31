#ifndef TemperatureStateService_h
#define TemperatureStateService_h

#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <HttpEndpoint.h>
#include <WebSocketTxRx.h>
class TemperatureState {
 public:
  float watertemperature;

 static void read(TemperatureState& settings, JsonObject& root) {
    root["watertemperature"] = settings.watertemperature;
  }

  static StateUpdateResult update(JsonObject& root, TemperatureState& state) {
    float watertemperature = root["watertemperature"];
    state.watertemperature = watertemperature;
    return StateUpdateResult::CHANGED;
  }
};

class TemperatureStateService : public StatefulService<TemperatureState> {
 public:
  
    TemperatureStateService(AsyncWebServer* server);


  void begin();
  void loop();
  void readSensor();

 private:

  HttpEndpoint<TemperatureState> _httpEndpoint;
  WebSocketTxRx<TemperatureState> _webSocket;
  OneWire _ds = OneWire(D2);
  DallasTemperature _tempSensors = DallasTemperature(&_ds);
  unsigned long _lastReading = 0;
TemperatureState _state;

  void onConfigUpdated();
};

#endif
