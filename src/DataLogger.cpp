#include "DataLogger.h"

DataLogger::DataLogger(FileSystem* fs, SensorDataCollector* collector, AsyncWebServer* server)
    : _fileSystem(fs), _collector(collector), _server(server) {}

void DataLogger::logSensorData() {
    // Collect sensor data as a JSON string
    String sensorData = _collector->collectSensorData();

    // Append the data to the file (you can use appendFile or another method)
    _fileSystem->appendFile(LittleFS, "/sensor_data.json", (sensorData + "\r\n").c_str());
}
