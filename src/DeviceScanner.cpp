#include "DeviceScanner.h"
#include <Wire.h>
#include <ESPAsyncWebServer.h>

// --- Configuration based on FireBeetle 2 ---
#define SDA_PIN 21
#define SCL_PIN 22

// Analog pins
const int analogPins[] = {34, 35, 36, 39};

// Digital pins for D2, D3, D5, D6, D7, D9
const int digitalPins[] = {25, 26, 0, 14, 13, 2};
const char* digitalPinLabels[] = {"D2", "D3", "D5", "D6", "D7", "D9"};

// UART pins
#define UART1_RX 16
#define UART1_TX 17
#define UART2_RX 4  // Unused, placeholder if needed
#define UART2_TX 5  // Unused, placeholder if needed

// --- Global Logging ---
String logBuffer = "";

// --- Methods of DeviceScanner ---

DeviceScanner::DeviceScanner() {
    // Clear or initialize the JSON document if needed.
}

DeviceScanner::~DeviceScanner() {
    // Nothing specific to free.
}

// Set all (non-reserved) pins to a safe state (INPUT)
void DeviceScanner::initializeAllPinsToSafeState() {
    Serial.println("Initializing pins to safe state...");
    int unsafePins[] = {1, 3, 6, 7, 8, 9, 10, 11, 16, 17, 20, 24, 28, 29, 30, 31, 37, 38};
    for (int pin = 0; pin < 40; pin++) {
        bool isUnsafe = false;
        for (int i = 0; i < sizeof(unsafePins) / sizeof(unsafePins[0]); i++) {
            if (pin == unsafePins[i]) {
                isUnsafe = true;
                break;
            }
        }
        if (!isUnsafe) {
            pinMode(pin, INPUT);
            delay(1);
        }
    }
    Serial.println("Pins initialized to INPUT state.");
}

// Scan the I2C bus
void DeviceScanner::scanI2CBus() {
    Serial.println("🔍 Scanning I2C Bus...");
    delay(100);
    
    JsonArray i2cArray = scanResults["i2cDevices"].to<JsonArray>();
    for (uint8_t address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        uint8_t error = Wire.endTransmission();
        if (error == 0) {
            String addrStr = "0x" + String(address, HEX);
            i2cArray.add(addrStr);
            Serial.print("✅ I2C device at: ");
            Serial.println(addrStr);
        }
    }
    if (i2cArray.size() == 0) {
        Serial.println("⚠️ No I2C devices found.");
    }
    Wire.end(); // Release I2C bus
}

// Scan analog pins
void DeviceScanner::scanAnalogPins() {
    Serial.println("Scanning Analog Pins...");
    JsonArray analogArray = scanResults["analogPins"].to<JsonArray>();
    for (int i = 0; i < sizeof(analogPins) / sizeof(analogPins[0]); i++) {
        int value = analogRead(analogPins[i]);
        if (value > 50 || value < 4090) { // Filter noise
            JsonObject pinData = analogArray.createNestedObject();
            pinData["pin"] = analogPins[i];
            pinData["value"] = value;
            Serial.printf("📏 Analog GPIO %d: %d\n", analogPins[i], value);
        }
    }
}

// Scan digital pins
void DeviceScanner::scanDigitalPins() {
    Serial.println("Scanning Digital Pins...");
    JsonArray digitalArray = scanResults["digitalPins"].to<JsonArray>();
    int uartPins[] = {16, 17}; // UART1 only
    for (int i = 0; i < sizeof(digitalPins) / sizeof(digitalPins[0]); i++) {
        bool skip = false;
        for (int j = 0; j < sizeof(uartPins) / sizeof(uartPins[0]); j++) {
            if (digitalPins[i] == uartPins[j]) {
                skip = true;
                break;
            }
        }
        if (skip) continue;
        pinMode(digitalPins[i], INPUT_PULLDOWN);
        delay(10);
        int value = digitalRead(digitalPins[i]);
        JsonObject pinData = digitalArray.createNestedObject();
        pinData["pin"] = digitalPinLabels[i]; // D2, D3, D5, etc.
        pinData["value"] = value;
        Serial.printf("🔌 Digital %s (GPIO %d): %d\n", digitalPinLabels[i], digitalPins[i], value);
        pinMode(digitalPins[i], INPUT);
    }
}

// Scan UART interfaces
void DeviceScanner::scanUARTInterfaces() {
    /*Serial.println("🔍 Scanning UART Interfaces...");
    JsonObject uartResults = scanResults["uartInterfaces"].to<JsonObject>();
    
    HardwareSerial uart1(1);
    uart1.begin(9600, SERIAL_8N1, UART1_RX, UART1_TX);
    uart1.println("TEST\n");
    delay(100);
    int avail1 = uart1.available();
    if (avail1 > 0) {
        uartResults["UART1"] = avail1;
        Serial.printf("📡 UART1 (RX:%d, TX:%d) bytes: %d\n", UART1_RX, UART1_TX, avail1);
    }
    uart1.end();
    
    HardwareSerial uart2(2);
    uart2.begin(9600, SERIAL_8N1, UART2_RX, UART2_TX);
    uart2.println("TEST\n");
    delay(100);
    int avail2 = uart2.available();
    if (avail2 > 0) {
        uartResults["UART2"] = avail2;
        Serial.printf("📡 UART2 (RX:%d, TX:%d) bytes: %d\n", UART2_RX, UART2_TX, avail2);
    }
    uart2.end();*/
}

// Perform initial scans
void DeviceScanner::begin() {
    Serial.println("Starting device scan...");
    Wire.begin(SDA_PIN, SCL_PIN);

    initializeAllPinsToSafeState();
    scanI2CBus();
    scanAnalogPins();
    scanDigitalPins();
    scanUARTInterfaces();
    Serial.println("Scan complete.");
}

// Periodic rescans
void DeviceScanner::loop() {
    static unsigned long lastScanTime = 0;
    const unsigned long RESCAN_INTERVAL = 30000; // 30 seconds
    if (millis() - lastScanTime >= RESCAN_INTERVAL) {
        scanI2CBus();
        scanAnalogPins();
        scanDigitalPins();
        scanUARTInterfaces();
        lastScanTime = millis();
    }
}

// Return JSON results
String DeviceScanner::getScanResults() {
    String jsonString;
    if (scanResults.isNull()) {
        jsonString = "{}";
    } else {
        serializeJsonPretty(scanResults, jsonString);
    }
    return jsonString;
}