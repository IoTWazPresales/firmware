#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <vector>

class DriverPackageManager {
 public:
  explicit DriverPackageManager(fs::FS* fs);

  bool applyPackage(const String& packagePath);
  bool verifyManifest(const String& manifestPath) const;
  void listPackages(JsonArray& out) const;

 private:
  fs::FS* _fs;
};
