#include "DriverPackageService.h"

#include <ArduinoJson.h>
#include <AsyncJson.h>
#include "DriverPackageManager.h"

DriverPackageService::DriverPackageService(AsyncWebServer* server,
                                           DriverPackageManager* manager,
                                           fs::FS* fs)
    : _manager(manager), _fs(fs) {
  // Don't access filesystem in constructor - do it in begin()
  
  server->on("/api/driver-packages", HTTP_GET, [this](AsyncWebServerRequest* request) {
    AsyncJsonResponse* response = new AsyncJsonResponse;
    JsonObject root = response->getRoot();
    JsonArray items = root.createNestedArray("items");
    if (_manager) {
      _manager->listPackages(items);
    }
    response->setLength();
    request->send(response);
  });

  server->on("/api/driver-packages", HTTP_POST,
             [this](AsyncWebServerRequest* request) {
               if (!_uploadOk) {
                 request->send(500, "application/json",
                               "{\"status\":\"error\",\"message\":\"Upload failed\"}");
               } else {
                 request->send(200, "application/json",
                               "{\"status\":\"ok\",\"message\":\"Package stored\"}");
               }
               _uploadOk = false;
               _uploadTarget = "";
             },
             [this](AsyncWebServerRequest* request, const String& filename, size_t index,
                    uint8_t* data, size_t len, bool final) {
               handleUpload(request, filename, index, data, len, final);
             });
}

void DriverPackageService::begin() {
  // Initialize filesystem-dependent operations (called after LittleFS is mounted)
  if (_fs && !_fs->exists("/drivers")) {
    _fs->mkdir("/drivers");
  }
}

void DriverPackageService::handleUpload(AsyncWebServerRequest* request,
                                        const String& filename,
                                        size_t index,
                                        uint8_t* data,
                                        size_t len,
                                        bool final) {
  if (!_fs) {
    _uploadOk = false;
    return;
  }

  if (index == 0) {
    _uploadTarget = String("/drivers/") + filename;
    _uploadFile = _fs->open(_uploadTarget, "w");
    _uploadOk = _uploadFile ? true : false;
    if (!_uploadOk) {
      Serial.printf("❌ Failed to open %s for writing\n", _uploadTarget.c_str());
    }
  }

  if (!_uploadOk) {
    return;
  }

  if (len) {
    size_t written = _uploadFile.write(data, len);
    if (written != len) {
      Serial.printf("❌ Failed to write chunk to %s\n", _uploadTarget.c_str());
      _uploadOk = false;
    }
  }

  if (final) {
    if (_uploadFile) {
      _uploadFile.close();
    }
    if (_uploadOk && _manager) {
      _uploadOk = _manager->applyPackage(_uploadTarget);
    }
    if (!_uploadOk && _fs->exists(_uploadTarget)) {
      _fs->remove(_uploadTarget);
    }
  }
}
