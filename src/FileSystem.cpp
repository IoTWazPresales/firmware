#include "FileSystem.h"
FileSystem::FileSystem(AsyncWebServer* server) {
    this->server = server;
}

void FileSystem::appendFile(fs::FS &fs, const char * path, const char * message) {
    Serial.println("Appending to file: %s\r\n");
    Serial.println(path);

    File file = fs.open(path, FILE_APPEND);
    if (!file) {
        Serial.println("- failed to open file for appending");
        return;
    }

    if (file.print(message)) {
        Serial.println("- message appended");
    } else {
        Serial.println("- append failed");
    }
    file.close();
}

void FileSystem::writeFile(fs::FS &fs, const char * path, const char * message) {
    Serial.println("Writing file: %s\r\n");
    Serial.println(path);

    File file = fs.open(path, FILE_WRITE);
    if (!file) {
        Serial.println("- failed to open file for writing");
        return;
    }

    if (file.print(message)) {
        Serial.println("- file written");
    } else {
        Serial.println("- write failed");
    }
    file.close();
}

void FileSystem::readFile(fs::FS &fs, const char * path) {
    Serial.println("Reading file: %s\r\n");
    Serial.println(path);

    File file = fs.open(path);
    if (!file || file.isDirectory()) {
        Serial.println("- failed to open file for reading");
        return;
    }

    Serial.println("- read from file:");
    while (file.available()) {
        Serial.write(file.read());
    }
    file.close();
}

void FileSystem::logFileSystemStats() {
    size_t totalBytes = LittleFS.totalBytes();
    size_t usedBytes = LittleFS.usedBytes();
    size_t freeBytes = totalBytes - usedBytes;

    Serial.println("File System - Total: %d bytes, Used: %d bytes, Free: %d bytes\n");
    Serial.println(totalBytes);
    Serial.println(usedBytes);
    Serial.println(freeBytes);

}