#include "SensorEndpoints.h"
#include "SensorDataCollector.h"
#include "SupabaseConnector.h"

SensorEndpoints::SensorEndpoints(AsyncWebServer* server, SensorManager* manager, SupabaseConnector* supabase)
    : _server(server), _manager(manager), _supabase(supabase) {
    handleSensorData();
}

void SensorEndpoints::handleSensorData() {
    _server->on("/api/sensor", HTTP_GET, [this](AsyncWebServerRequest *request) {
        DynamicJsonDocument jsonDoc(JSON_OBJECT_SIZE(25) + 500);

        // Pull current sensor values through the SensorManager
        auto* tempSensor = _manager->getFirstTemperatureSensor();
        auto* phSensor = _manager->getFirstPH();
        auto* dhtSensor = _manager->getFirstDHT11();
        auto* moistureSensor = _manager->getFirstMoistureSensor();
        auto* tdsSensor = _manager->getFirstTDS();
        auto* rtc = _manager->getFirstRTC();
        auto* atmosphere = _manager->getFirstAtmosphereSensor();
        auto* spectral = _manager->getFirstSpectralSensor();
        auto* npk = _manager->getFirstNPKSensor();

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

        jsonDoc["total"] = spectral ? spectral->getTotalLight() : -1;
        jsonDoc["blue"] = spectral ? spectral->getBlueRatio() : -1;
        jsonDoc["green"] = spectral ? spectral->getGreenRatio() : -1;
        jsonDoc["red"] = spectral ? spectral->getRedRatio() : -1;
        jsonDoc["farRed"] = spectral ? spectral->getFarRedRatio() : -1;
        jsonDoc["ChlorophyllIndexRedGreen"] = spectral ? spectral->getChlorophyllIndexRedGreen() : -1;
        jsonDoc["ChlorophyllIndexRedBlue"] = spectral ? spectral->getChlorophyllIndexRedBlue() : -1;
        jsonDoc["ndvi"] = spectral ? spectral->getNDVI() : -1;
        jsonDoc["greenIntensity"] = spectral ? spectral->getGreenLightIntensity() : -1;
        jsonDoc["lux"] = spectral ? spectral->getLux() : -1;

        // Sync to Supabase if connected
        if (_supabase && _supabase->isConnected()) {
            _supabase->syncSensorData(jsonDoc);
        }

        String response;
        serializeJson(jsonDoc, response);

        request->send(200, "application/json", response);
    });

    _server->on("/api/sensor", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse(204);
    });
}
