#ifndef DATALOGGER_H
#define DATALOGGER_H

#include <Arduino.h>
#include "FileSystem.h"
#include "SensorDataCollector.h"

class DataLogger {
public:
    // Constructor to initialize the DataLogger with FileSystem and SensorDataCollector
    DataLogger(FileSystem* fs, SensorDataCollector* collector, AsyncWebServer* server);

    // Method to log sensor data (collect data and save it to a file)
    void logSensorData();

private:
    // Pointer to the FileSystem for file operations
    FileSystem* _fileSystem;

    // Pointer to the SensorDataCollector to gather sensor data
    SensorDataCollector* _collector;

    AsyncWebServer* _server;
};

#endif
