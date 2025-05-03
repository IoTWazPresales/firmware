#include <NPKSensor.h>

#define RE_DE 4  // RS485 transceiver control pin
#define RXD1 16
#define TXD1 17
#define SENSOR_CONNECTED false // Set to true when sensor is connected
HardwareSerial SerialPort(2);  // Change this if you're using a different serial port

// Define the queries to send to the NPK sensor
uint8_t npkQuery[] = {0x01, 0x03, 0x00, 0x1E, 0x00, 0x03, 0x65, 0xCD};
uint8_t phQuery[] = {0x01, 0x03, 0x00, 0x06, 0x00, 0x01, 0x64, 0x0B};
uint8_t soilMoistureQuery[] = {0x01, 0x03, 0x00, 0x12, 0x00, 0x01, 0x24, 0x0F};
uint8_t soilTemperatureQuery[] = {0x01, 0x03, 0x00, 0x13, 0x00, 0x01, 0x75, 0xCF};
uint8_t conductivityQuery[] = {0x01, 0x03, 0x00, 0x15, 0x00, 0x01, 0x95, 0xCE};

// Constructor
NPKSensor::NPKSensor(AsyncWebServer* server) {
    _state.nitrogen = -1;
    _state.potassium = -1;
    _state.phosphorus = -1;
    _state.phsoil = -1;
    _sensorAvailable = false; // Initialize as false
    
}
float NPKSensor::getNitrogen() const {
    return _state.nitrogen;
}
float NPKSensor::getPotassium() const {
    return _state.potassium;
}
float NPKSensor::getPhosphorus() const {
    return _state.phosphorus;
}
float NPKSensor::getPHSoil() const {
    return _state.phsoil;
}

// Initialize RS485 communication (called once)
void NPKSensor::begin() {
    // Initialize UART1 only once
    Serial.println("Starting NPK serial");
    Serial.printf("Free heap before: %d\n", ESP.getFreeHeap());
    Serial.println("Starting Port serial");
    SerialPort.begin(9600, SERIAL_8N1, RXD1, TXD1);
    Serial.printf("Free heap after: %d\n", ESP.getFreeHeap());
    pinMode(RE_DE, OUTPUT);
    digitalWrite(RE_DE, LOW);
_sensorAvailable = SENSOR_CONNECTED;
Serial.println(_sensorAvailable ? "NPK sensor assumed connected" : "No NPK sensor detected (manual config)");
Serial.println("RS485 NPK Sensor Initialized - FireBeetle 2");

}
void NPKSensor::sendAndRead(uint8_t* query, size_t querySize, uint8_t* responseBuffer, size_t responseSize) {
    if (!_sensorAvailable) {
        Serial.println("NPK sensor not available, skipping UART operation...");
        return;
    }
    if (!SerialPort) {
        Serial.println("Error: UART1 not initialized!");
        return;
    }
    Serial.println("Writing Port to high");
    digitalWrite(RE_DE, HIGH);
    delay(100);  // Increased from 50ms
    Serial.println("Writing query size");
    SerialPort.write(query, querySize);
    Serial.println("Flushing serial");
    SerialPort.flush();  // Ensure transmission completes
    delay(100);  // Increased from 50ms
    Serial.println("Writing Port to low");
    digitalWrite(RE_DE, LOW);
    delay(200);  // Increased from 100ms
    Serial.println("checking response");
    if (SerialPort.available() >= responseSize) {
        SerialPort.readBytes(responseBuffer, responseSize);
    } else {
        Serial.println("No response or insufficient data from sensor");
    }
}
// Logging functions
void NPKSensor::printNPKValues() {
    Serial.println("Setting NPK response to 0");
    uint8_t npkResponse[11] = {0};
    Serial.println("initiate send and read");

    sendAndRead(npkQuery, sizeof(npkQuery), npkResponse, sizeof(npkResponse));
    Serial.println("setting NPK responses");
    uint16_t nitrogen   = (npkResponse[3] << 8) | npkResponse[4];
    uint16_t phosphorus = (npkResponse[5] << 8) | npkResponse[6];
    uint16_t potassium  = (npkResponse[7] << 8) | npkResponse[8];
    Serial.println("NPK Values:");
    Serial.print("Nitrogen: ");
    Serial.println(nitrogen);
    Serial.print("Phosphorus: ");
    Serial.println(phosphorus);
    Serial.print("Potassium: ");
    Serial.println(potassium);
}
  
void NPKSensor::printPHValue() {
    uint8_t phResponse[7] = {0};
    sendAndRead(phQuery, sizeof(phQuery), phResponse, sizeof(phResponse));
    uint16_t phValue = (phResponse[3] << 8) | phResponse[4];
    float pH = phValue / 100.0;
    Serial.println("pH Value:");
    Serial.println(pH);
}
  
void NPKSensor::printSoilMoisture() {
    uint8_t moistureResponse[7] = {0};
    sendAndRead(soilMoistureQuery, sizeof(soilMoistureQuery), moistureResponse, sizeof(moistureResponse));
    uint16_t moistureValue = (moistureResponse[3] << 8) | moistureResponse[4];
    float moisture = moistureValue / 10.0;
    Serial.println("Soil Moisture:");
    Serial.println(moisture);
}
  
void NPKSensor::printSoilTemperature() {
    uint8_t temperatureResponse[7] = {0};
    sendAndRead(soilTemperatureQuery, sizeof(soilTemperatureQuery), temperatureResponse, sizeof(temperatureResponse));
    uint16_t temperatureValue = (temperatureResponse[3] << 8) | temperatureResponse[4];
    float temperature = temperatureValue / 10.0;
    Serial.println("Soil Temperature:");
    Serial.println(temperature);
}
  
void NPKSensor::printConductivity() {
    uint8_t conductivityResponse[7] = {0};
    sendAndRead(conductivityQuery, sizeof(conductivityQuery), conductivityResponse, sizeof(conductivityResponse));
    uint16_t conductivityValue = (conductivityResponse[3] << 8) | conductivityResponse[4];
    Serial.println("Conductivity:");
    Serial.println(conductivityValue);
}
  
// Called in your main loop (with an appropriate period)
void NPKSensor::loop() {
    if (!_sensorAvailable) {
        Serial.println("NPK sensor not available, skipping read...");
        return;
    }
    readNPK();
    
}
  
// Read NPK sensor values by calling the logging functions in sequence
void NPKSensor::readNPK() {
    Serial.println("Beginning NPK Read Sequence...");
    printNPKValues();
    delay(1000);
    printPHValue();
    delay(1000);
    printSoilMoisture();
    delay(1000);
    printSoilTemperature();
    delay(1000);
    printConductivity();
    delay(1000);
}