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
  
  // Read package (assumes JSON format with manifest + files)
  DynamicJsonDocument doc(2048);
  DeserializationError err = deserializeJson(doc, pkg);
  pkg.close();
  
  if (err) {
    Serial.printf("⚠️ Failed to parse package %s: %s\n", packagePath.c_str(), err.c_str());
    return false;
  }
  
  // Extract manifest
  if (!doc.containsKey("manifest")) {
    Serial.println("⚠️ Package missing manifest");
    return false;
  }
  
  JsonObject manifest = doc["manifest"].as<JsonObject>();
  String driverId = manifest["driver"].as<String>();
  
  // Verify manifest
  String manifestPath = String("/manifests/") + driverId + ".json";
  if (!verifyManifest(manifestPath)) {
    // Create manifest from package
    File mf = _fs->open(manifestPath, "w");
    if (mf) {
      serializeJson(manifest, mf);
      mf.close();
      Serial.printf("✅ Created manifest: %s\n", manifestPath.c_str());
    }
  }
  
  // Extract driver files if present
  if (doc.containsKey("files")) {
    JsonObject files = doc["files"].as<JsonObject>();
    for (JsonPair kv : files) {
      String filePath = String("/drivers/") + kv.key().c_str();
      String content = kv.value().as<String>();
      File f = _fs->open(filePath, "w");
      if (f) {
        f.print(content);
        f.close();
        Serial.printf("  📄 Extracted: %s\n", filePath.c_str());
      }
    }
  }
  
  Serial.printf("✅ Package %s installed successfully\n", packagePath.c_str());
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
