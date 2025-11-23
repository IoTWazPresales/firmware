#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>

class DriverPackageManager;

class DriverPackageService {
 public:
  DriverPackageService(AsyncWebServer* server, DriverPackageManager* manager, fs::FS* fs);
  void begin();  // Initialize filesystem-dependent operations (call after LittleFS is mounted)

 private:
  void handleUpload(AsyncWebServerRequest* request,
                    const String& filename,
                    size_t index,
                    uint8_t* data,
                    size_t len,
                    bool final);

  DriverPackageManager* _manager;
  fs::FS* _fs;
  File _uploadFile;
  bool _uploadOk = false;
  String _uploadTarget;
};
