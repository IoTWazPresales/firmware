#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>

class FileSystem {
public:
    FileSystem(AsyncWebServer* server);

    void appendFile(fs::FS &fs, const char * path, const char * message);
    void readFile(fs::FS &fs, const char * path);
    void writeFile(fs::FS &fs, const char * path, const char * message);
    void logFileSystemStats();  // Method for logging filesystem stats

private:
    AsyncWebServer* server;
};

#endif