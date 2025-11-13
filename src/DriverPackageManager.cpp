#include "DriverPackageManager.h"

DriverPackageManager::DriverPackageManager(fs::FS* fs)
    : _fs(fs) {}

bool DriverPackageManager::applyPackage(const String& packagePath) {
  if (!_fs) {
    Serial.println("⚠️ DriverPackageManager has no filesystem");
    return false;
  }
  File pkg = _fs->open(packagePath, "r");
  if (!pkg) {
    Serial.printf("⚠️ Driver package %s not found\n", packagePath.c_str());
    return false;
  }
  // Placeholder for future package verification and installation
  Serial.printf("ℹ️ Driver package %s queued for installation\n", packagePath.c_str());
  pkg.close();
  return true;
}

bool DriverPackageManager::verifyManifest(const String& manifestPath) const {
  if (!_fs) {
    return false;
  }
  File file = _fs->open(manifestPath, "r");
  if (!file) {
    Serial.printf("⚠️ Manifest %s missing\n", manifestPath.c_str());
    return false;
  }
  DynamicJsonDocument doc(512);
  auto err = deserializeJson(doc, file);
  file.close();
  if (err) {
    Serial.printf("⚠️ Manifest %s failed to parse: %s\n", manifestPath.c_str(), err.c_str());
    return false;
  }
  if (!doc.containsKey("driver") || !doc.containsKey("capabilities")) {
    Serial.printf("⚠️ Manifest %s missing required fields\n", manifestPath.c_str());
    return false;
  }
  return true;
}

void DriverPackageManager::listPackages(JsonArray& out) const {
  if (!_fs) {
    return;
  }
  File dir = _fs->open("/drivers", "r");
  if (!dir || !dir.isDirectory()) {
    return;
  }
  File entry = dir.openNextFile();
  while (entry) {
    if (!entry.isDirectory()) {
      JsonObject obj = out.createNestedObject();
      obj["name"] = entry.name();
      obj["size"] = static_cast<uint32_t>(entry.size());
    }
    entry = dir.openNextFile();
  }
  dir.close();
}
