// DeviceScanner.cpp

#include "DeviceScanner.h"
#include "UnknownSensorDetector.h"
#include <Wire.h>
#include <ArduinoJson.h>

// --- Configuration based on your JST wiring ---
#define SDA_PIN 21
#define SCL_PIN 22

static const int analogPins[]      = { 36, 39, 34, 35 };   // A0–A3
static const char* analogLabels[] = { "A0", "A1", "A2", "A3" };

static const int digitalPins[]      = { 25, 26, 0, 14, 13, 2, 4, 12 };  // D2,D3,D5,D6,D7,D9,D12,D13
static const char* digitalLabels[] = { "D2","D3","D5","D6","D7","D9","D12","D13" };

// UART pins (unused for now)
#define UART1_RX 16
#define UART1_TX 17

DeviceScanner::DeviceScanner(AsyncWebServer* server)
  : _server(server)
{}

DeviceScanner::~DeviceScanner() {}

void DeviceScanner::initializeAllPinsToSafeState() {
    Serial.println("🔧 initializeAllPinsToSafeState()");
    for (int pin = 0; pin < 40; pin++) {
        // skip reserved pins + I²C lines
        bool skip = (
            pin==1||pin==3||pin==6||pin==7||pin==8||pin==9||
            pin==10||pin==11||pin==20||pin==24||pin==28||
            pin==29||pin==30||pin==31||pin==37||pin==38||
            pin==SDA_PIN||pin==SCL_PIN
        );
        if (!skip) {
            pinMode(pin, INPUT);
        }
    }
}

void DeviceScanner::scanI2CBus() {
    Serial.println("🔍 Scanning I2C Bus…");
    
    // Ensure I2C is initialized
    if (!Wire.getClock()) {
        Serial.println("  ⚠️ I2C not initialized, initializing now...");
        Wire.begin(SDA_PIN, SCL_PIN);
        delay(100);
    }
    
    JsonArray arr = scanResults["i2cDevices"].to<JsonArray>();
    arr.clear();

    uint8_t foundCount = 0;
    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        uint8_t error = Wire.endTransmission();
        
        if (error == 0) {
            String s = "0x" + String(addr, HEX);
            JsonObject device = arr.createNestedObject();
            device["address"] = s;
            
            // Check if we have a driver for this device
            // Common sensor addresses: ADS1115 (0x48, 0x49), BME280/BMP280 (0x76, 0x77), 
            // ENS160 (0x53), AS7341 (0x39), DS3231 RTC (0x68), etc.
            bool hasDriver = false;
            if (addr == 0x48 || addr == 0x49 ||  // ADS1115 ADC
                addr == 0x76 || addr == 0x77 ||  // BME280/BMP280
                addr == 0x53 ||                  // ENS160 Air Quality
                addr == 0x39 ||                  // AS7341 Spectral
                addr == 0x68) {                  // DS3231 RTC
                device["known"] = true;
                hasDriver = true;
            } else {
                device["known"] = false;
            }
            
            Serial.printf("  ✅ I2C @ %s %s\n", s.c_str(), hasDriver ? "(known)" : "(unknown)");
            foundCount++;
        } else if (error == 4) {
            // Bus error - might indicate I2C bus issue
            Serial.printf("  ⚠️ I2C bus error at address 0x%02X\n", addr);
        }
    }
    
    Serial.printf("🔍 → found %u I²C device(s)\n", foundCount);
    if (foundCount == 0) {
        Serial.println("  ⚠️ No I2C devices found.");
        Serial.println("     Check wiring: SDA=" + String(SDA_PIN) + ", SCL=" + String(SCL_PIN));
        Serial.println("     Ensure pull-up resistors (4.7kΩ) are connected to 3.3V");
    }
}

void DeviceScanner::scanAnalogPins() {
    JsonArray arr = scanResults["analogPins"].to<JsonArray>();
    arr.clear();


   for (size_t i = 0; i < sizeof(analogPins)/sizeof(analogPins[0]); ++i) {
      int pin = analogPins[i];
      pinMode(pin, INPUT);
      delayMicroseconds(10);

        constexpr int SAMPLES = 5;
        int sum = 0, minv = 4095, maxv = 0;
        for (int i = 0; i < SAMPLES; i++) {
            int v = analogRead(pin);
            sum  += v;
            minv  = min(minv, v);
            maxv  = max(maxv, v);
            delayMicroseconds(20);
        }
        int avg = sum / SAMPLES;
        int variance = maxv - minv;
        // Tighter detection: must have variance > 100 AND be in middle range (not floating)
        // This reduces false positives from floating pins
        bool detected = (variance > 100) && (minv > 100 && maxv < 4000);

        JsonObject o = arr.createNestedObject();
        o["pin"]       = pin;
        o["key"]      = analogLabels[i];
        o["detected"]  = detected;
        o["rawValue"]  = detected ? avg : 0;

        Serial.printf("  A%d → %s (avg=%d,min=%d,max=%d)\n",
                      pin,
                      detected ? "sensor" : "open",
                      avg, minv, maxv);
    }
    Serial.printf("💧 → %u analog pins scanned\n", arr.size());
}

void DeviceScanner::scanDigitalPins() {
    Serial.println("🔌 Scanning digital GPIOs…");
    JsonArray arr = scanResults["digitalPins"].to<JsonArray>();
    arr.clear();

    for (size_t i=0; i<sizeof(digitalPins)/sizeof(digitalPins[0]); ++i) {
      int pin = digitalPins[i];

        // pull-down test
        pinMode(pin, INPUT_PULLDOWN);
        delay(10);
        int vpd = digitalRead(pin);

        // pull-up test
        pinMode(pin, INPUT_PULLUP);
        delay(10);
        int vpu = digitalRead(pin);

        bool detected = (vpd == HIGH || vpu == LOW);

        JsonObject o = arr.createNestedObject();
        o["pin"]      = pin;
         o["key"]      = digitalLabels[i];
        o["detected"] = detected;
        o["value"]    = detected ? (vpd == HIGH ? 1 : 0) : -1;

        Serial.printf("  %s (GPIO %d) → %s (pd=%d, pu=%d)\n",
                      digitalLabels[i],
                      pin,
                      detected ? "attached" : "open",
                      vpd, vpu);

        // restore to clean state
        pinMode(pin, INPUT);
    }
    Serial.printf("🔌 → %u digital pins scanned\n", arr.size());
}

void DeviceScanner::scanUARTInterfaces() {
    // no-op for now
}

void DeviceScanner::collectUnknownSensorSignatures() {
    unknownSignatures.clear();
    JsonArray signatures = unknownSignatures.createNestedArray("signatures");
    
    // Collect I2C signatures for unknown devices
    JsonArray i2cDevices = scanResults["i2cDevices"].to<JsonArray>();
    for (size_t i = 0; i < i2cDevices.size(); i++) {
        JsonObject device = i2cDevices[i].as<JsonObject>();
        if (device.containsKey("known") && !device["known"].as<bool>()) {
            String addrStr = device["address"].as<String>();
            uint8_t addr = (uint8_t)strtol(addrStr.c_str() + 2, nullptr, 16);
            auto sig = UnknownSensorDetector::detectI2CDevice(addr);
            JsonObject sigObj = signatures.createNestedObject();
            sigObj["interfaceType"] = sig.interfaceType;
            sigObj["identifier"] = sig.identifier;
            sigObj["timestamp"] = sig.timestamp;
            sigObj["metadata"] = sig.metadata;
        }
    }
    
    // Collect analog signatures for detected but unknown sensors
    JsonArray analogPins = scanResults["analogPins"].to<JsonArray>();
    for (size_t i = 0; i < analogPins.size(); i++) {
        JsonObject pin = analogPins[i].as<JsonObject>();
        if (pin["detected"].as<bool>()) {
            int pinNum = pin["pin"].as<int>();
            auto sig = UnknownSensorDetector::detectAnalogPin(pinNum);
            JsonObject sigObj = signatures.createNestedObject();
            sigObj["interfaceType"] = sig.interfaceType;
            sigObj["identifier"] = sig.identifier;
            sigObj["timestamp"] = sig.timestamp;
            sigObj["metadata"] = sig.metadata;
        }
    }
    
    // Collect digital signatures
    JsonArray digitalPins = scanResults["digitalPins"].to<JsonArray>();
    for (size_t i = 0; i < digitalPins.size(); i++) {
        JsonObject pin = digitalPins[i].as<JsonObject>();
        if (pin["detected"].as<bool>()) {
            int pinNum = pin["pin"].as<int>();
            auto sig = UnknownSensorDetector::detectDigitalPin(pinNum);
            JsonObject sigObj = signatures.createNestedObject();
            sigObj["interfaceType"] = sig.interfaceType;
            sigObj["identifier"] = sig.identifier;
            sigObj["timestamp"] = sig.timestamp;
            sigObj["metadata"] = sig.metadata;
        }
    }
}

String DeviceScanner::getUnknownSensorSignatures() {
    String output;
    serializeJsonPretty(unknownSignatures, output);
    return output;
}

void DeviceScanner::begin() {
    Serial.println("🚀 DeviceScanner::begin()");
    scanResults.clear();
    scanResults.createNestedArray("i2cDevices");
    scanResults.createNestedArray("analogPins");
    scanResults.createNestedArray("digitalPins");
    scanResults.createNestedArray("uartInterfaces");
    
    unknownSignatures.clear();

    // I²C setup
    Wire.begin(SDA_PIN, SCL_PIN);

   // initializeAllPinsToSafeState();
    scanI2CBus();
    scanAnalogPins();
    scanDigitalPins();
    scanUARTInterfaces();
    collectUnknownSensorSignatures();

    // HTTP endpoints
    _server->on("/api/scan", HTTP_GET, [this](AsyncWebServerRequest* req){
        String json = getScanResults();
        Serial.println("→ RAW SCAN JSON:\n" + json);
        req->send(200, "application/json", json);
    });
    _server->on("/api/scan/trigger", HTTP_POST, [this](AsyncWebServerRequest* req){
        scanI2CBus();
        scanAnalogPins();
        scanDigitalPins();
        scanUARTInterfaces();
        collectUnknownSensorSignatures();
        req->send(200, "application/json", getScanResults());
    });
    
    // Unknown sensor signature endpoint
    _server->on("/api/scan/unknown", HTTP_GET, [this](AsyncWebServerRequest* req){
        collectUnknownSensorSignatures();
        String json = getUnknownSensorSignatures();
        req->send(200, "application/json", json);
    });

    Serial.println("✔️ Initial scan complete");
}

void DeviceScanner::loop() {
    static uint32_t last = 0;
    if (millis() - last < 30000) return;
    last = millis();

    scanI2CBus();
    scanAnalogPins();
    scanDigitalPins();
    scanUARTInterfaces();
    collectUnknownSensorSignatures();

    Serial.println("✔️ Periodic scan complete");
}

String DeviceScanner::getScanResults() {
    String out;
    serializeJsonPretty(scanResults, out);
    return out;
}
