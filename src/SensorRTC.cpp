#include <SensorRTC.h>

// https://registry.platformio.org/libraries/adafruit/RTClib/examples/ds3231/ds3231.ino

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

SensorRTC::SensorRTC(AsyncWebServer* server) : _server(server), _lastReading(0) {
    _state.rtc = "Unavaiable"; // Initialize the pH value
}
String SensorRTC::getRTC() const {
    return _state.rtc;
}

void SensorRTC::begin() {
  // _state.temperature = 999;
  // Serial.println(F("Init temperature sensor"));
  Serial.print("Starting RTC sensor...");
  if (!_rtc.begin()) {
    Serial.print(F("RTC ERROR"));
  }

  if (_rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    _rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  readSensor();
 
}

void SensorRTC::loop() {

    unsigned long currentMillis = millis();
  if (currentMillis - _lastReading >= RTCInterval) {
        _lastReading = currentMillis;
        readSensor(); // Update pH
    }
}

void SensorRTC::readSensor() {
  DateTime now = _rtc.now();
  String timestamp = now.timestamp();
  // Serial.print(now.year(), DEC);
  // Serial.print('/');
  // Serial.print(now.month(), DEC);
  // Serial.print('/');
  // Serial.print(now.day(), DEC);
  // Serial.print(" (");
  // Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  // Serial.print(") ");
  // Serial.print(now.hour(), DEC);
  // Serial.print(':');
  // Serial.print(now.minute(), DEC);
  // Serial.print(':');
  // Serial.print(now.second(), DEC);
  // Serial.println();

  // Serial.print(" since midnight 1/1/1970 = ");
  // Serial.print(now.unixtime());
  // Serial.print("s = ");
  // Serial.print(now.unixtime() / 86400L);
  // Serial.println("d");

  _state.rtc = timestamp;  //.c_str();
  Serial.print(F("RTC: "));
  Serial.println(_state.rtc);
}


