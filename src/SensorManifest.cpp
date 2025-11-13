#include "SensorManifest.h"

#include <ArduinoJson.h>

static bool loadManifestFile(fs::FS& fs, const char* path, SensorManifest& out) {
  File file = fs.open(path, "r");
  if (!file) {
    Serial.printf("⚠️ Unable to open manifest: %s\n", path);
    return false;
  }

  DynamicJsonDocument doc(1024);
  DeserializationError err = deserializeJson(doc, file);
  file.close();
  if (err) {
    Serial.printf("⚠️ Failed to parse manifest %s: %s\n", path, err.c_str());
    return false;
  }

  JsonVariant root = doc.as<JsonVariant>();
  if (!root.is<JsonObject>()) {
    Serial.printf("⚠️ Manifest %s is not a JSON object\n", path);
    return false;
  }

  out.driverId = root["driver"].as<String>();
  out.label = root["label"].as<String>();
  out.unit = root["unit"].as<String>();
  out.autoDetect = root["autoDetect"].as<bool>();

  out.capabilities.clear();
  out.capabilityLabels.clear();
  out.capabilityUnits.clear();
  if (root["capabilities"].is<JsonArray>()) {
    for (JsonVariant v : root["capabilities"].as<JsonArray>()) {
      String id;
      String label;
      String unit;

      if (v.is<JsonObject>()) {
        JsonObject obj = v.as<JsonObject>();
        id = obj["id"].as<String>();
        label = obj["label"].as<String>();
        unit = obj["unit"].as<String>();
      } else {
        id = v.as<String>();
      }

      if (id.length() > 0) {
        out.capabilities.push_back(id);
        if (label.length() > 0) {
          out.capabilityLabels[id] = label;
        }
        if (unit.length() > 0) {
          out.capabilityUnits[id] = unit;
        }
      }
    }
  }

  out.tags.clear();
  if (root["tags"].is<JsonArray>()) {
    for (JsonVariant v : root["tags"].as<JsonArray>()) {
      String tag = v.as<String>();
      if (tag.length() > 0) {
        out.tags.push_back(tag);
      }
    }
  }

  return true;
}

std::vector<SensorManifest> SensorManifest::loadAll(fs::FS& fs, const char* directory) {
  std::vector<SensorManifest> manifests;
  File dir = fs.open(directory, "r");
  if (!dir || !dir.isDirectory()) {
    Serial.printf("ℹ️ Manifest directory '%s' not found\n", directory);
    return manifests;
  }

  File entry = dir.openNextFile();
  while (entry) {
    if (!entry.isDirectory()) {
      String path = String(directory) + "/" + entry.name();
      SensorManifest manifest;
      if (loadManifestFile(fs, path.c_str(), manifest)) {
        manifests.push_back(std::move(manifest));
      }
    }
    entry = dir.openNextFile();
  }
  dir.close();
  return manifests;
}

const SensorManifest* findManifestForCapability(const std::vector<SensorManifest>& manifests,
                                                const String& capabilityId) {
  for (const auto& manifest : manifests) {
    if (manifest.capabilityLabels.count(capabilityId) ||
        manifest.capabilityUnits.count(capabilityId)) {
      return &manifest;
    }
    for (const auto& cap : manifest.capabilities) {
      if (cap == capabilityId) {
        return &manifest;
      }
    }
  }
  return nullptr;
}
