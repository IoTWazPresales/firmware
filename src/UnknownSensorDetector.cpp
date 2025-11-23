#include "UnknownSensorDetector.h"
#include <Wire.h>

HardwareSignature UnknownSensorDetector::detectI2CDevice(uint8_t address) {
  HardwareSignature sig;
  sig.interfaceType = "i2c";
  sig.identifier = "0x" + String(address, HEX);
  sig.timestamp = millis();
  sig.metadata = collectI2CSignature(address);
  return sig;
}

HardwareSignature UnknownSensorDetector::detectAnalogPin(int pin) {
  HardwareSignature sig;
  sig.interfaceType = "analog";
  sig.identifier = String(pin);
  sig.timestamp = millis();
  sig.metadata = collectAnalogSignature(pin);
  return sig;
}

HardwareSignature UnknownSensorDetector::detectDigitalPin(int pin) {
  HardwareSignature sig;
  sig.interfaceType = "digital";
  sig.identifier = String(pin);
  sig.timestamp = millis();
  sig.metadata = collectDigitalSignature(pin);
  return sig;
}

HardwareSignature UnknownSensorDetector::detectUARTDevice(int rxPin, int txPin) {
  HardwareSignature sig;
  sig.interfaceType = "uart";
  sig.identifier = String(rxPin) + "," + String(txPin);
  sig.timestamp = millis();
  
  StaticJsonDocument<256> meta;
  meta["rxPin"] = rxPin;
  meta["txPin"] = txPin;
  meta["baudRates"] = JsonArray();
  
  // Try common baud rates
  const unsigned long bauds[] = {9600, 19200, 38400, 57600, 115200};
  for (size_t i = 0; i < sizeof(bauds)/sizeof(bauds[0]); i++) {
    meta["baudRates"].add(bauds[i]);
  }
  
  sig.metadata = meta.as<JsonObject>();
  return sig;
}

JsonObject UnknownSensorDetector::collectI2CSignature(uint8_t address) {
  StaticJsonDocument<512> doc;
  JsonObject meta = doc.to<JsonObject>();
  
  meta["address"] = address;
  meta["addressHex"] = "0x" + String(address, HEX);
  
  // Try to read common device ID registers
  Wire.beginTransmission(address);
  Wire.write(0x00); // Common WHO_AM_I register
  uint8_t error = Wire.endTransmission();
  
  if (error == 0) {
    Wire.requestFrom(address, (uint8_t)8);
    if (Wire.available()) {
      JsonArray regData = meta.createNestedArray("registerData");
      uint8_t count = 0;
      while (Wire.available() && count < 8) {
        regData.add(Wire.read());
        count++;
      }
    }
  }
  
  // Try reading a few common registers
  JsonArray registers = meta.createNestedArray("registers");
  uint8_t commonRegs[] = {0x00, 0x01, 0x02, 0x0F, 0x75, 0x76};
  for (size_t i = 0; i < sizeof(commonRegs)/sizeof(commonRegs[0]); i++) {
    Wire.beginTransmission(address);
    Wire.write(commonRegs[i]);
    if (Wire.endTransmission() == 0) {
      Wire.requestFrom(address, (uint8_t)1);
      if (Wire.available()) {
        JsonObject reg = registers.createNestedObject();
        reg["addr"] = commonRegs[i];
        reg["value"] = Wire.read();
      }
    }
    delay(5);
  }
  
  return meta;
}

JsonObject UnknownSensorDetector::collectAnalogSignature(int pin) {
  StaticJsonDocument<256> doc;
  JsonObject meta = doc.to<JsonObject>();
  
  meta["pin"] = pin;
  
  // Sample over time to detect patterns
  const int SAMPLES = 20;
  JsonArray samples = meta.createNestedArray("samples");
  int minVal = 4095, maxVal = 0, sum = 0;
  
  for (int i = 0; i < SAMPLES; i++) {
    int val = analogRead(pin);
    samples.add(val);
    minVal = min(minVal, val);
    maxVal = max(maxVal, val);
    sum += val;
    delay(10);
  }
  
  meta["min"] = minVal;
  meta["max"] = maxVal;
  meta["avg"] = sum / SAMPLES;
  meta["range"] = maxVal - minVal;
  meta["voltageRange"] = (maxVal - minVal) * 3.3 / 4095.0;
  
  return meta;
}

JsonObject UnknownSensorDetector::collectDigitalSignature(int pin) {
  StaticJsonDocument<256> doc;
  JsonObject meta = doc.to<JsonObject>();
  
  meta["pin"] = pin;
  
  // Test pull-up and pull-down
  pinMode(pin, INPUT_PULLDOWN);
  delay(10);
  int pdValue = digitalRead(pin);
  
  pinMode(pin, INPUT_PULLUP);
  delay(10);
  int puValue = digitalRead(pin);
  
  meta["pullDown"] = pdValue;
  meta["pullUp"] = puValue;
  meta["detected"] = (pdValue == HIGH || puValue == LOW);
  
  // Sample over time
  JsonArray samples = meta.createNestedArray("samples");
  for (int i = 0; i < 20; i++) {
    samples.add(digitalRead(pin));
    delay(10);
  }
  
  pinMode(pin, INPUT);
  return meta;
}

String UnknownSensorDetector::serializeSignature(const HardwareSignature& sig) {
  StaticJsonDocument<512> doc;
  doc["interfaceType"] = sig.interfaceType;
  doc["identifier"] = sig.identifier;
  doc["timestamp"] = sig.timestamp;
  doc["metadata"] = sig.metadata;
  
  String output;
  serializeJson(doc, output);
  return output;
}

