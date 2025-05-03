#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <LittleFS.h>
#include "SensorManager.h"
#include "SensorPH.h"
#include "TemperatureSensor.h"
#include "SensorMoisture.h"
#include "SensorTDS.h"
#include "SensorRTC.h"
#include "AtmosphereSensor.h"
#include "SpectralSensor.h"
#include "NPKSensor.h"
#include "DeviceScanner.h"
#include "ScannerEndpoints.h"
#include "SensorEndpoints.h"
#include "RelayControl.h"
#include "RelayControlEndpoint.h"
#include "WiFiStatus.h"
#include "WiFiScanner.h"
#include "WiFiSettingsService.h"
#include "SystemStatus.h"
#include "OTAHandler.h"
#include "SensorDataCollector.h"
#include "GrowLight.h"
#include "FileSystem.h"
#include "FactoryResetService.h"
#include "RestartService.h"
#include "DataLogger.h"
#include "ESP32React.h"

#define I2C_SDA 21
#define I2C_SCL 22

AsyncWebServer server(80);
ESP32React esp32React(&server);
DeviceScanner scanner;
ScannerEndpoints scannerEndpoints(&server, &scanner);
SensorManager sensorManager(&server);

// Sensor instances (non-dynamic for now)
SensorPH phSensor(&server);
NPKSensor NPK(&server);
TemperatureSensor tempSensor(&server);
SensorMoisture soilMoisture(&server);
SensorTDS tdsSensor(&server);
SensorRTC realtime(&server);
SpectralSensor Spectral(&server);
AtmosphereSensor atmos(&server);

// Relay controls
RelayControl waterPump(&server, &soilMoisture, sensorManager.getFirstDHT11(), &phSensor, &atmos);
RelayControl intakeFan(&server, &soilMoisture, sensorManager.getFirstDHT11(), &phSensor, &atmos);
RelayControl exhaustFan(&server, &soilMoisture, sensorManager.getFirstDHT11(), &phSensor, &atmos);
RelayControl setPumpThreshold(&server, &soilMoisture, sensorManager.getFirstDHT11(), &phSensor, &atmos);
RelayControl setIntakeThreshold(&server, &soilMoisture, sensorManager.getFirstDHT11(), &phSensor, &atmos);
RelayControl setExhaustTempThreshold(&server, &soilMoisture, sensorManager.getFirstDHT11(), &phSensor, &atmos);
RelayControl setExhaustHumidityThreshold(&server, &soilMoisture, sensorManager.getFirstDHT11(), &phSensor, &atmos);
RelayControlEndpoint relayEndpoints(&server, &waterPump, &intakeFan, &exhaustFan, &setPumpThreshold,
                                   &setIntakeThreshold, &setExhaustTempThreshold, &setExhaustHumidityThreshold);

// Other services
GrowLight Growlight(&server);
WiFiSettingsService wifiSettingsService(&server);
FileSystem fileSystem(&server);
WiFiStatus Wifistatus(&server);
WiFiScanner Wifiscanner(&server);
SystemStatus systemstatus(&server);
FactoryResetService factoryResetService(&server, &LittleFS);
SensorEndpoints endpoints(&server, &phSensor, &tempSensor, sensorManager.getFirstDHT11(), &soilMoisture, &realtime, &tdsSensor, &atmos, &Spectral, &NPK);
SensorDataCollector dataCollector(&phSensor, &tempSensor, sensorManager.getFirstDHT11(), &soilMoisture, &realtime, &tdsSensor, &atmos);
DataLogger dataLogger(&fileSystem, &dataCollector, &server);
OTAHandler otaHandler;

// WiFi credentials
const char* ssid = "9532828 [2Ghz]";
const char* password = "0611401627";
const char* otaPassword = "0611401627";
#define FORMAT_LITTLEFS_IF_FAILED true

void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    delay(1000);

    // Initialize SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
        return;
    }

    // Initialize LittleFS
    if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
        Serial.println("LittleFS Mount Failed");
        return;
    }
    Serial.println("LittleFS Mounted Successfully");

    // Check/create sensor data file
    if (!LittleFS.exists("/sensor_data.json")) {
        Serial.println("Creating /sensor_data.json...");
        fileSystem.writeFile(LittleFS, "/sensor_data.json", "MY ESP32 DATA \r\n");
    } else {
        Serial.println("File /sensor_data.json exists");
    }

    // Connect to WiFi
    Serial.println("Connecting to WiFi...");
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(ssid, password);
    unsigned long wifiTimeout = millis() + 10000;
    while (WiFi.status() != WL_CONNECTED && millis() < wifiTimeout) {
        delay(500);
        Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi Connected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.print("RSSI: ");
        Serial.println(WiFi.RSSI());
    } else {
        Serial.println("\nWiFi connection failed!");
        wifiSettingsService.begin();
    }

    // Serve static files
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    server.serveStatic("/static/", LittleFS, "/");

    // Initialize DeviceScanner
    scanner.begin();
    String results = scanner.getScanResults();
    Serial.println("Initial Scan Results:");
    Serial.println(results);

    // Initialize framework and services
    esp32React.begin();
    Wire.begin(I2C_SDA, I2C_SCL, 100000);
    Serial.println("Starting sensors...");

    // Initialize SensorManager
    sensorManager.begin();

    // Initialize other sensors
    Serial.println("Beginning Spectral");
    Spectral.begin();
    Serial.println("Beginning Timer");
    realtime.begin();
    Serial.println("Beginning Atmos");
    atmos.begin();
    Serial.println("Beginning Light");
    Growlight.begin();
    Serial.println("Beginning TempSensor");
    tempSensor.begin();
    Serial.println("Beginning phSensor");
    phSensor.begin();
    Serial.println("Beginning SoilMoisture");
    soilMoisture.begin();
    Serial.println("Beginning TDS");
    tdsSensor.begin();
    Serial.println("Beginning NPK");
    NPK.begin();

    // Initialize relays
    Serial.println("Beginning Water Pump");
    waterPump.begin();
    Serial.println("Beginning Intake");
    intakeFan.begin();
    Serial.println("Beginning Exhaust");
    exhaustFan.begin();
    setPumpThreshold.begin();
    setIntakeThreshold.begin();
    setExhaustTempThreshold.begin();
    setExhaustHumidityThreshold.begin();

    // Initialize relay endpoints
    Serial.println("Beginning Relay Endpoints");
    relayEndpoints.handleRelayData();

    // Start server
    server.begin();
    Serial.println("Server Begun");

    // Fallback to WiFi settings if not connected
    if (WiFi.status() != WL_CONNECTED) {
        wifiSettingsService.begin();
    }
}

void loop() {
    // WiFi reconnect logic
    static unsigned long lastReconnect = 0;
    static int reconnectAttempts = 0;
    const int maxAttempts = 5;
    const unsigned long reconnectBackoff = 30000;
    if (WiFi.status() != WL_CONNECTED && millis() - lastReconnect >= 5000 && reconnectAttempts < maxAttempts) {
        Serial.println("WiFi disconnected, reconnecting...");
        WiFi.begin(ssid, password);
        unsigned long wifiTimeout = millis() + 5000;
        while (WiFi.status() != WL_CONNECTED && millis() < wifiTimeout) {
            delay(500);
            Serial.print(".");
        }
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nWiFi reconnected!");
            Serial.print("IP Address: ");
            Serial.println(WiFi.localIP());
            Serial.print("RSSI: ");
            Serial.println(WiFi.RSSI());
            reconnectAttempts = 0;
        } else {
            reconnectAttempts++;
            lastReconnect = millis();
            if (reconnectAttempts >= maxAttempts) {
                Serial.println("Max reconnect attempts reached, backing off...");
                lastReconnect = millis() - 5000 + reconnectBackoff;
            }
        }
        return;
    }

    // Reset reconnect attempts if connected
    if (WiFi.status() == WL_CONNECTED && reconnectAttempts > 0) {
        reconnectAttempts = 0;
    }

    // OTA handling
    otaHandler.handle();

    // Resource monitoring
    static unsigned long lastResourceCheck = 0;
    if (millis() - lastResourceCheck >= 5000) {
        lastResourceCheck = millis();
        Serial.print("Free heap: ");
        Serial.println(ESP.getFreeHeap());
        Serial.print("Min free heap: ");
        Serial.println(ESP.getMinFreeHeap());
        Serial.print("RSSI: ");
        Serial.println(WiFi.RSSI());
    }

    // Stagger sensor and service loops
    static unsigned long lastSensorRead = 0;
    static int sensorIndex = 0;
    if (millis() - lastSensorRead >= 2000) {
        lastSensorRead = millis();
        switch (sensorIndex) {
            case 0:
                Serial.println("DeviceScanner");
                scanner.loop();
                break;
            case 1:
                Serial.println("ESP32React");
                esp32React.loop();
                break;
            case 2:
                Serial.println("GrowLight");
                Growlight.loop();
                break;
            case 3:
                Serial.println("AtmosphereSensor");
                atmos.loop();
                break;
            case 4:
                Serial.println("SpectralSensor");
                Spectral.loop();
                break;
            case 5:
                Serial.println("TemperatureSensor");
                tempSensor.loop();
                break;
            case 6:
                Serial.println("SensorPH");
                phSensor.loop();
                break;
            case 7:
                Serial.println("SensorMoisture");
                soilMoisture.loop();
                break;
            case 8:
                Serial.println("SensorRTC");
                realtime.loop();
                break;
            case 9:
                Serial.println("SensorTDS");
                tdsSensor.loop();
                break;
            case 10:
                Serial.println("NPKSensor");
                NPK.loop();
                break;
            case 11:
                Serial.println("SensorManager");
                sensorManager.loop();
                break;
        }
        sensorIndex = (sensorIndex + 1) % 12;
    }

    // Prevent tight looping
    delay(200);
    yield();
}