#include "SpectralSensor.h"

SpectralSensor::SpectralSensor(AsyncWebServer* server, int sdaPin, int sclPin) 
    : _server(server), _sdaPin(sdaPin), _sclPin(sclPin), _lastReading(0), _sensorAvailable(false) {
    _state.LightChannel1 = -1;
    _state.LightChannel2 = -1;
    _state.LightChannel3 = -1;
    _state.LightChannel4 = -1;
    _state.LightChannel5 = -1;
    _state.LightChannel6 = -1;
    _state.LightChannel7 = -1;
    _state.LightChannel8 = -1;
    _state.BlueRatio = -1;
    _state.GreenRatio = -1;
    _state.RedRatio = -1;
    _state.FarRedRatio = -1;
    _state.TotalLight = -1;
    _state.ChlorophyllIndexRedGreen = -1;
    _state.ChlorophyllIndexRedBlue = -1;
    _state.GreenLightIntensity = -1;
    _state.NDVI = -999;
    _state.Lux = -1;
}

void SpectralSensor::begin() {
    if (_sdaPin < 0 || _sclPin < 0) {
        Serial.println("Spectral sensor SDA/SCL pins not assigned");
        return;
    }
    Serial.printf("Initializing AS7341 on SDA: GPIO %d, SCL: GPIO %d...\n", _sdaPin, _sclPin);
    if (!_as7341.begin()) {
        Serial.println("Could not find AS7341");
        _sensorAvailable = false;
        return;
    }
    _sensorAvailable = true;
    Serial.println("AS7341 detected. Configuring sensor...");
    _as7341.setATIME(100);
    _as7341.setASTEP(999);
    _as7341.setGain(AS7341_GAIN_256X);
    Serial.println("AS7341 initialized successfully");

    _server->on("/api/sensor/spectral", HTTP_GET, [this](AsyncWebServerRequest *request) {
        DynamicJsonDocument doc(1024);
        doc["LightChannel1"] = _state.LightChannel1;
        doc["LightChannel2"] = _state.LightChannel2;
        doc["LightChannel3"] = _state.LightChannel3;
        doc["LightChannel4"] = _state.LightChannel4;
        doc["LightChannel5"] = _state.LightChannel5;
        doc["LightChannel6"] = _state.LightChannel6;
        doc["LightChannel7"] = _state.LightChannel7;
        doc["LightChannel8"] = _state.LightChannel8;
        doc["BlueRatio"] = _state.BlueRatio;
        doc["GreenRatio"] = _state.GreenRatio;
        doc["RedRatio"] = _state.RedRatio;
        doc["FarRedRatio"] = _state.FarRedRatio;
        doc["TotalLight"] = _state.TotalLight;
        doc["ChlorophyllIndexRedGreen"] = _state.ChlorophyllIndexRedGreen;
        doc["ChlorophyllIndexRedBlue"] = _state.ChlorophyllIndexRedBlue;
        doc["GreenLightIntensity"] = _state.GreenLightIntensity;
        doc["NDVI"] = _state.NDVI;
        doc["Lux"] = _state.Lux;
        String json;
        serializeJson(doc, json);
        request->send(200, "application/json", json);
    });

}

void SpectralSensor::loop() {
    if (!_sensorAvailable) {
        Serial.println("AS7341 not available");
        return;
    }
    unsigned long currentMillis = millis();
    if (currentMillis - _lastReading >= _spectralSensorInterval) {
        uint16_t channels[12];
        if (!_as7341.readAllChannels(channels)) {
            Serial.println("Error reading AS7341 channels");
            _sensorAvailable = false;
            return;
        }
        yield(); 
        _state.LightChannel1 = channels[0]; // 415nm (F1)
        _state.LightChannel2 = channels[1]; // 445nm (F2)
        _state.LightChannel3 = channels[2]; // 480nm (F3)
        _state.LightChannel4 = channels[3]; // 515nm (F4)
        _state.LightChannel5 = channels[4]; // 555nm (F5)
        _state.LightChannel6 = channels[5]; // 590nm (F6)
        _state.LightChannel7 = channels[6]; // 630nm (F7)
        _state.LightChannel8 = channels[7]; // 680nm (F8)
        float NIR = channels[9]; // Near-IR
        float clear = channels[10]; // Clear channel

        float emittedRedLight = 500.0;
        float emittedBlueLight = 400.0;
        float emittedGreenLight = 300.0;

        _state.TotalLight = _state.LightChannel1 + _state.LightChannel2 + _state.LightChannel3 +
                            _state.LightChannel4 + _state.LightChannel5 + _state.LightChannel6 +
                            _state.LightChannel7 + _state.LightChannel8 + NIR;

        _state.BlueRatio = (_state.LightChannel1 + _state.LightChannel2 + _state.LightChannel3) / _state.TotalLight * 100;
        _state.GreenRatio = (_state.LightChannel4 + _state.LightChannel5) / _state.TotalLight * 100;
        _state.RedRatio = (_state.LightChannel6 + _state.LightChannel7) / _state.TotalLight * 100;
        _state.FarRedRatio = (_state.LightChannel8 + NIR) / _state.TotalLight * 100;

        float reflectedBlue = _state.LightChannel1 + _state.LightChannel2 + _state.LightChannel3;
        float reflectedRed = _state.LightChannel6 + _state.LightChannel7;
        float reflectedGreen = _state.LightChannel4 + _state.LightChannel5;

        float absorbedBlue = emittedBlueLight - reflectedBlue;
        float absorbedRed = emittedRedLight - reflectedRed;
        float absorbedGreen = emittedGreenLight - reflectedGreen;

        _state.ChlorophyllIndexRedGreen = (absorbedGreen > 0) ? (absorbedRed / absorbedGreen) : 0;
        _state.ChlorophyllIndexRedBlue = (absorbedBlue > 0) ? (absorbedRed / absorbedBlue) : 0;
        _state.GreenLightIntensity = reflectedGreen;

        float RED = _state.LightChannel6 + _state.LightChannel7;
        _state.NDVI = (NIR + RED > 0) ? (NIR - RED) / (NIR + RED) : 0;

        float F2 = _state.LightChannel2;
        float F3 = _state.LightChannel3;
        float F4 = _state.LightChannel4;
        float F5 = _state.LightChannel5;
        float F7 = _state.LightChannel7;

        const float W_blue1 = 0.1;
        const float W_blue2 = 0.2;
        const float W_green1 = 0.4;
        const float W_green2 = 0.4;
        const float W_red = 0.1;

        _state.Lux = (W_blue1 * F2) + (W_blue2 * F3) + (W_green1 * F4) + (W_green2 * F5) + (W_red * F7);

        Serial.println("AS7341 Readings:");
        Serial.printf("F1 415nm: %.2f\n", _state.LightChannel1);
        Serial.printf("F2 445nm: %.2f\n", _state.LightChannel2);
        Serial.printf("F3 480nm: %.2f\n", _state.LightChannel3);
        Serial.printf("F4 515nm: %.2f\n", _state.LightChannel4);
        Serial.printf("F5 555nm: %.2f\n", _state.LightChannel5);
        Serial.printf("F6 590nm: %.2f\n", _state.LightChannel6);
        Serial.printf("F7 630nm: %.2f\n", _state.LightChannel7);
        Serial.printf("F8 680nm: %.2f\n", _state.LightChannel8);
        Serial.printf("Clear: %.2f\n", clear);
        Serial.printf("Near IR: %.2f\n", NIR);
        Serial.printf("Blue Ratio: %.2f%%\n", _state.BlueRatio);
        Serial.printf("Green Ratio: %.2f%%\n", _state.GreenRatio);
        Serial.printf("Red Ratio: %.2f%%\n", _state.RedRatio);
        Serial.printf("Far Red Ratio: %.2f%%\n", _state.FarRedRatio);
        Serial.printf("Chlorophyll Index (Red/Green): %.2f\n", _state.ChlorophyllIndexRedGreen);
        Serial.printf("Chlorophyll Index (Red/Blue): %.2f\n", _state.ChlorophyllIndexRedBlue);
        Serial.printf("Green Light Intensity: %.2f\n", _state.GreenLightIntensity);
        Serial.printf("NDVI: %.2f\n", _state.NDVI);
        Serial.printf("Lux: %.2f\n", _state.Lux);
        _lastReading = currentMillis;
    }
}

float SpectralSensor::getLightChannel1() const { return _state.LightChannel1; }
float SpectralSensor::getLightChannel2() const { return _state.LightChannel2; }
float SpectralSensor::getLightChannel3() const { return _state.LightChannel3; }
float SpectralSensor::getLightChannel4() const { return _state.LightChannel4; }
float SpectralSensor::getLightChannel5() const { return _state.LightChannel5; }
float SpectralSensor::getLightChannel6() const { return _state.LightChannel6; }
float SpectralSensor::getLightChannel7() const { return _state.LightChannel7; }
float SpectralSensor::getLightChannel8() const { return _state.LightChannel8; }
float SpectralSensor::getBlueRatio() const { return _state.BlueRatio; }
float SpectralSensor::getGreenRatio() const { return _state.GreenRatio; }
float SpectralSensor::getRedRatio() const { return _state.RedRatio; }
float SpectralSensor::getFarRedRatio() const { return _state.FarRedRatio; }
float SpectralSensor::getTotalLight() const { return _state.TotalLight; }
float SpectralSensor::getChlorophyllIndexRedGreen() const { return _state.ChlorophyllIndexRedGreen; }
float SpectralSensor::getChlorophyllIndexRedBlue() const { return _state.ChlorophyllIndexRedBlue; }
float SpectralSensor::getGreenLightIntensity() const { return _state.GreenLightIntensity; }
float SpectralSensor::getNDVI() const { return _state.NDVI; }
float SpectralSensor::getLux() const { return _state.Lux; }