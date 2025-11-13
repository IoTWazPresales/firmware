#pragma once

#include <Arduino.h>
#include <FS.h>
#include <map>
#include <vector>

struct SensorManifest {
  String driverId;
  String label;
  String unit;
  std::vector<String> capabilities;
  std::map<String, String> capabilityLabels;
  std::map<String, String> capabilityUnits;
  std::vector<String> tags;
  bool autoDetect = false;

  static std::vector<SensorManifest> loadAll(fs::FS& fs, const char* directory = "/manifests");
};

const SensorManifest* findManifestForCapability(const std::vector<SensorManifest>& manifests,
                                                const String& capabilityId);
