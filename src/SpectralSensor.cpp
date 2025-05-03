#include "SpectralSensor.h"

SpectralSensor::SpectralSensor(AsyncWebServer* server)
    : _server(server), _lastReading(0), _address(0) {
    _state.LightChannel1 = -1;
    _state.LightChannel2 = -1;
    _state.LightChannel3 = -1;
    _state.LightChannel4 = -1;
    _state.LightChannel5 = -1;
    _state.LightChannel6 = -1;
    _state.LightChannel7 = -1;
    _state.LightChannel8 = -1;
    _state.TotalLight = -1;
    _state.BlueRatio = -1;
    _state.GreenRatio = -1;
    _state.RedRatio = -1;
    _state.FarRedRatio = -1;
    _state.ChlorophyllIndexRedGreen = -1;
    _state.ChlorophyllIndexRedBlue = -1;
    _state.GreenLightIntensity = -1;
    _state.NDVI = -999;
    _state.Lux = -1;
}
// Getter functions for individual light channels
float SpectralSensor::getLightChannel1() const { return _state.LightChannel1; }
float SpectralSensor::getLightChannel2() const { return _state.LightChannel2; }
float SpectralSensor::getLightChannel3() const { return _state.LightChannel3; }
float SpectralSensor::getLightChannel4() const { return _state.LightChannel4; }
float SpectralSensor::getLightChannel5() const { return _state.LightChannel5; }
float SpectralSensor::getLightChannel6() const { return _state.LightChannel6; }
float SpectralSensor::getLightChannel7() const { return _state.LightChannel7; }
float SpectralSensor::getLightChannel8() const { return _state.LightChannel8; }

// Getter functions for ratios
float SpectralSensor::getBlueRatio() const { return _state.BlueRatio; }
float SpectralSensor::getGreenRatio() const { return _state.GreenRatio; }
float SpectralSensor::getRedRatio() const { return _state.RedRatio; }
float SpectralSensor::getFarRedRatio() const { return _state.FarRedRatio; }
float SpectralSensor::getTotalLight() const { return _state.TotalLight; }
// Getter function for total light
float SpectralSensor::getChlorophyllIndexRedGreen() const { return _state.ChlorophyllIndexRedGreen; }
float SpectralSensor::getChlorophyllIndexRedBlue() const { return _state.ChlorophyllIndexRedBlue; }
float SpectralSensor::getGreenLightIntensity() const { return _state.GreenLightIntensity; }
float SpectralSensor::getNDVI() const { return _state.NDVI; }
float SpectralSensor::getLux() const { return _state.Lux; }

void SpectralSensor::begin() {
 Serial.println("Initializing AS7341...");
if (!_as7341.begin()) {
Serial.println("Could not find AS7341. Sensor not connected or faulty.");
_sensorAvailable = false;
return;
    }
    _sensorAvailable = true;
    Serial.println("AS7341 detected. Configuring sensor...");
    // Sensor configuration
   Serial.println("Setup Atime, ASTEP, Gain");
  _as7341.setATIME(100);
  _as7341.setASTEP(999);
  _as7341.setGain(AS7341_GAIN_256X);
  Serial.println("AS7341 initialized successfully.");
readSensor(); // Initial read
}

void SpectralSensor::loop() {
  if (!_sensorAvailable) {
    Serial.println("AS7341 not available, skipping read...");
    return;
    }
    unsigned long currentMillis = millis();
    if (currentMillis - _lastReading >= SpectralSensorInterval) {
    _lastReading = currentMillis;
    readSensor();
    }
}

void SpectralSensor::readSensor() {
  if (!_sensorAvailable) {
    Serial.println("AS7341 not available, skipping read...");
    return;
    }
    uint16_t channels[12];
if (!_as7341.readAllChannels(channels)) {
Serial.println("Error reading AS7341 channels!");
_sensorAvailable = false; // Mark sensor as unavailable on failure
return;
}

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


    float emittedRedLight = 500.0;  // Example emitted red light intensity in µmol/s
    float emittedBlueLight = 400.0; // Example emitted blue light intensity in µmol/s
    float emittedGreenLight = 300.0; // Example emitted green light intensity in µmol/s


     _state.TotalLight = _state.LightChannel1 + _state.LightChannel2 + _state.LightChannel3 +
                        _state.LightChannel4 + _state.LightChannel5 + _state.LightChannel6 +
                        _state.LightChannel7 + _state.LightChannel8 + NIR;


     _state.BlueRatio = (_state.LightChannel1 + _state.LightChannel2 + _state.LightChannel3) / _state.TotalLight * 100;
    _state.GreenRatio = (_state.LightChannel4 + _state.LightChannel5) / _state.TotalLight * 100;
    _state.RedRatio = (_state.LightChannel6 + _state.LightChannel7) / _state.TotalLight * 100;
    _state.FarRedRatio = (_state.LightChannel8 + NIR) / _state.TotalLight * 100;

    // Log for debugging
    Serial.println("Light Ratios:");
    Serial.println("Blue: , Green: , Red: , Far Red: \n");
                   Serial.println(_state.BlueRatio);
                  Serial.println(_state.GreenRatio);
                   Serial.println(_state.RedRatio);
                   Serial.println(_state.FarRedRatio);

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
     Serial.println("Chlorophyll Index (Red/Green): %.2f\n");
      Serial.println(_state.ChlorophyllIndexRedGreen);
    Serial.println("Chlorophyll Index (Red/Blue): %.2f\n");
    Serial.println(_state.ChlorophyllIndexRedBlue);
 float F2 = _state.LightChannel2; // 445nm (Blue)
    float F3 = _state.LightChannel3; // 480nm (Blue)
    float F4 = _state.LightChannel4; // 515nm (Green)
    float F5 = _state.LightChannel5; // 555nm (Green)
    float F7 = _state.LightChannel7; // 630nm (Red)

    const float W_blue1 = 0.1;
    const float W_blue2 = 0.2;
    const float W_green1 = 0.4;
    const float W_green2 = 0.4;
    const float W_red = 0.1;

_state.Lux = (W_blue1 * F2) + (W_blue2 * F3) + (W_green1 * F4) + (W_green2 * F5) + (W_red * F7);

Serial.println("AS7341 Readings:");
Serial.print("F1 415nm: "); Serial.println(_state.LightChannel1);
Serial.print("F2 445nm: "); Serial.println(_state.LightChannel2);
Serial.print("F3 480nm: "); Serial.println(_state.LightChannel3);
Serial.print("F4 515nm: "); Serial.println(_state.LightChannel4);
Serial.print("F5 555nm: "); Serial.println(_state.LightChannel5);
Serial.print("F6 590nm: "); Serial.println(_state.LightChannel6);
Serial.print("F7 630nm: "); Serial.println(_state.LightChannel7);
Serial.print("F8 680nm: "); Serial.println(_state.LightChannel8);
Serial.print("Clear: "); Serial.println(clear);
Serial.print("Near IR: "); Serial.println(NIR);
Serial.println("Ratios (%):");
Serial.print("Blue: "); Serial.println(_state.BlueRatio, 2);
Serial.print("Green: "); Serial.println(_state.GreenRatio, 2);
Serial.print("Red: "); Serial.println(_state.RedRatio, 2);
Serial.print("Far Red: "); Serial.println(_state.FarRedRatio, 2);
Serial.print("Chlorophyll Index (Red/Green): "); Serial.println(_state.ChlorophyllIndexRedGreen, 2);
Serial.print("Chlorophyll Index (Red/Blue): "); Serial.println(_state.ChlorophyllIndexRedBlue, 2);
Serial.print("Green Light Intensity: "); Serial.println(_state.GreenLightIntensity);
Serial.print("NDVI: "); Serial.println(_state.NDVI, 2);
Serial.print("Lux: "); Serial.println(_state.Lux);
Serial.println();
  
}


// Add additional getters for other channels as needed
