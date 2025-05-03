#include <FactoryResetService.h>
#include <LittleFS.h>

using namespace std::placeholders;

FactoryResetService::FactoryResetService(AsyncWebServer* server, fs::FS* fileSystem) {
 server->on("/factory-reset", HTTP_POST, [fileSystem](AsyncWebServerRequest* request) {
        // Handle the factory reset logic here
       bool success = false;
       if (fileSystem == &LittleFS) {
            success = LittleFS.format();
        }

        if (success) {
            request->send(200, "application/json", "{\"success\":true}");
        } else {
            request->send(500, "application/json", "{\"success\":false}");
        }


    });
}

void FactoryResetService::handleRequest(AsyncWebServerRequest* request) {
  request->onDisconnect(std::bind(&FactoryResetService::factoryReset, this));
  request->send(200);
}

/**
 * Delete function assumes that all files are stored flat, within the config directory.
 */
void FactoryResetService::factoryReset() {
  File root = fs->open(FS_CONFIG_DIRECTORY);
  File file;
  while (file = root.openNextFile()) {
    fs->remove(file.name());
  }
  RestartService::restartNow();
}
