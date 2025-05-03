#include <SensorRTC.h>
// https://registry.platformio.org/libraries/adafruit/RTClib/examples/ds3231/ds3231.ino

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

SensorRTC::SensorRTC(AsyncWebServer* server) : _server(server), _lastReading(0),_rtcAvailable(false) {
    _state.rtc = "Unavailable"; // Initialize the pH value
}
String SensorRTC::getRTC() const {
    return _state.rtc;
}

void SensorRTC::begin() {
  // _state.temperature = 999;
  // Serial.println(F("Init temperature sensor"));
  Serial.println("Starting RTC sensor...");
  if (!_rtc.begin()) {
    Serial.println(F("RTC ERROR"));
    _rtcAvailable = false;
_state.rtc = "Unavailable";
return;
  }

  else {
    _rtcAvailable = true;   // RTC successfully initialized
    if (_rtc.lostPower()) {
        Serial.println("RTC lost power, let's set the time!");
        _rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Set to compile time
    }
    readSensor();  // Initial read if RTC is available
}


 
}

void SensorRTC::loop() {

  if (!_rtcAvailable) {
    Serial.println("[ERROR] RTC not initialized. Skipping loop.");
    return;
    }
    unsigned long currentMillis = millis();
    if (currentMillis - _lastReading >= RTCInterval) {
    _lastReading = currentMillis;
    readSensor();
    
}
}

void SensorRTC::readSensor() {
  if (!_rtcAvailable) {
    Serial.println("[ERROR] RTC not available. Skipping read.");
    _state.rtc = "Unavailable";
    return;
    }
    // Check I2C communication
    Wire.beginTransmission(0x68); // DS3231 address
    if (Wire.endTransmission() != 0) {
    Serial.println("[ERROR] RTC not responding.");
    _rtcAvailable = false;
    _state.rtc = "Unavailable";
    return;
    }
    DateTime now = _rtc.now();
    if (!now.isValid()) {
    Serial.println("[ERROR] Invalid RTC time.");
    _rtcAvailable = false;
    _state.rtc = "Unavailable";
    return;
    }
    _state.rtc = now.timestamp();
    Serial.print("RTC: ");
    Serial.println(_state.rtc);
    }


