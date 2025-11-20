#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Wire.h>

struct HardwareSignature {
  String interfaceType;  // "i2c", "analog", "digital", "uart", "ble", "lora", "sigfox"
  String identifier;     // I2C address, pin number, MAC address, etc.
  JsonObject metadata;  // Additional detected characteristics
  unsigned long timestamp;
};

class UnknownSensorDetector {
public:
  static HardwareSignature detectI2CDevice(uint8_t address);
  static HardwareSignature detectAnalogPin(int pin);
  static HardwareSignature detectDigitalPin(int pin);
  static HardwareSignature detectUARTDevice(int rxPin, int txPin);
  
  static JsonObject collectI2CSignature(uint8_t address);
  static JsonObject collectAnalogSignature(int pin);
  static JsonObject collectDigitalSignature(int pin);
  
  static String serializeSignature(const HardwareSignature& sig);
};

