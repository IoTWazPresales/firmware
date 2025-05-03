#include "SensorEndpoints.h"
#include "SensorDataCollector.h"  // Include your saveDataToSPIFFS function here

SensorEndpoints::SensorEndpoints(AsyncWebServer* server, 
                                SensorPH* phSensor, 
                                TemperatureSensor* tempSensor,
                                SensorHumidityDHT11* airData,
                                SensorMoisture* soilMoisture,
                                SensorRTC* realtime,
                                SensorTDS* tdsSensor,
                                AtmosphereSensor* atmosphere,
                                SpectralSensor* spectralSensor,
                                NPKSensor* NPKval                               
                                ) 

    : _server(server), 
    _phSensor(phSensor), 
    _tempSensor(tempSensor),
    _airData(airData),
    _soilMoisture(soilMoisture),
    _realtime(realtime),
    _tdsSensor(tdsSensor),
    _atmosphere(atmosphere),
    _spectralSensor(spectralSensor),
    _NPKval(NPKval)
    {
    handleSensorData(); // Initialize endpoint handler
   
}

void SensorEndpoints::handleSensorData() {
    // Enable CORS for all endpoint
    // Handle the GET request for sensor data
    _server->on("/api/sensor", HTTP_GET, [this](AsyncWebServerRequest *request) {
        // Create a JSON object to hold the sensor data
        DynamicJsonDocument jsonDoc(JSON_OBJECT_SIZE(25) + 500); // Adjusted buffer size
        // Fetch sensor data from your sensor classes
        float temperature = _tempSensor->getTemperature();
        float ph = _NPKval->getPHSoil();
        float humidity= _airData->getHumidity();
        float airtemp= _airData->getAirTemperature();
        float moisture= _soilMoisture->getMoisture();
        float tdsSens= _tdsSensor->getTDS();
        float airquality = _atmosphere->getAirQuality();
        float TVOC = _atmosphere->getTVOC();
        float CO2 = _atmosphere->getCO2();
        float total = _spectralSensor->getTotalLight();
        float blue = _spectralSensor->getBlueRatio();
        float green = _spectralSensor->getGreenRatio();
        float red = _spectralSensor->getRedRatio();
        float farRed = _spectralSensor->getFarRedRatio();
        float ChlorophyllIndexRedGreen = _spectralSensor->getChlorophyllIndexRedGreen();
        float ChlorophyllIndexRedBlue = _spectralSensor->getChlorophyllIndexRedBlue();
        float NDVI = _spectralSensor->getNDVI();
        float GreenLightIntensity = _spectralSensor->getGreenLightIntensity();
        float Lux = _spectralSensor->getLux();
        float Nitrogen = _NPKval->getNitrogen();
        float Potassium = _NPKval->getPotassium();
        float Phosphorus = _NPKval->getPhosphorus();
        float soilPH = _NPKval->getPHSoil();

        String real= _realtime->getRTC();

        auto validateReading = [](float value) {
            return !isnan(value) && value >= 0 ? value : -1; // Use -1 as error indicator
        };
        jsonDoc["temperature"] = temperature;
        jsonDoc["humidity"] = humidity;
        jsonDoc["airtemp"]= airtemp;
        jsonDoc["ph"] = ph;
        jsonDoc["moisture"] = moisture;
        jsonDoc["tdsSens"] = tdsSens;
        jsonDoc["airquality"] = airquality;
        jsonDoc["TVOC"] = TVOC;
        jsonDoc["CO2"] = CO2;
        jsonDoc["real"] = real;
        jsonDoc["nitro"] = Nitrogen;
        jsonDoc["potas"] = Potassium;
        jsonDoc["phos"] = Phosphorus;
        jsonDoc["soilph"] = soilPH;
        
        
        jsonDoc["total"] = total;
        jsonDoc["blue"] = blue;
        jsonDoc["green"] = green;
        jsonDoc["red"] = red;
        jsonDoc["farRed"] = farRed;
        jsonDoc["ChlorophyllIndexRedGreen"] = ChlorophyllIndexRedGreen;
        jsonDoc["ChlorophyllIndexRedBlue"] = ChlorophyllIndexRedBlue;
        jsonDoc["ndvi"] = NDVI;
        jsonDoc["greenIntensity"] = GreenLightIntensity;
        jsonDoc["lux"] = Lux;
        
        String response;
        serializeJson(jsonDoc, response);
               

        // Send the response back to the client (frontend)
        request->send(200, "application/json", response);


    });

    // Optional: Handle preflight OPTIONS request
    _server->on("/api/sensor", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
        request->send(204);
    });
}
