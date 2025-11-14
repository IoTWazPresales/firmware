#include "ErrorRecovery.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

Preferences ErrorRecovery::_prefs;
unsigned int ErrorRecovery::_criticalErrorCount = 0;
unsigned long ErrorRecovery::_lastCriticalError = 0;

void ErrorRecovery::logError(ErrorSeverity severity, const String& component, const String& message) {
  Serial.printf("[%s] %s: %s\n", 
                severity == ErrorSeverity::CRITICAL ? "CRITICAL" :
                severity == ErrorSeverity::ERROR ? "ERROR" :
                severity == ErrorSeverity::WARNING ? "WARN" : "INFO",
                component.c_str(), message.c_str());
  
  if (severity >= ErrorSeverity::ERROR) {
    _prefs.begin("errors", false);
    unsigned int count = _prefs.getUInt("error_count", 0);
    _prefs.putUInt("error_count", count + 1);
    _prefs.end();
  }
  
  if (severity == ErrorSeverity::CRITICAL) {
    _criticalErrorCount++;
    _lastCriticalError = millis();
    
    // Check if we should restart
    if (shouldRestart()) {
      Serial.println("🔄 Too many critical errors, restarting...");
      delay(1000);
      ESP.restart();
    }
  }
}

void ErrorRecovery::saveCriticalState() {
  // Save critical configuration to Preferences for recovery
  _prefs.begin("recovery", false);
  _prefs.putULong("last_save", millis());
  _prefs.putUInt("boot_count", _prefs.getUInt("boot_count", 0) + 1);
  _prefs.end();
  
  // Save to LittleFS as backup
  DynamicJsonDocument doc(512);
  doc["last_save"] = millis();
  doc["boot_count"] = _prefs.getUInt("boot_count", 0);
  
  File f = LittleFS.open("/recovery.json", "w");
  if (f) {
    serializeJson(doc, f);
    f.close();
  }
}

void ErrorRecovery::restoreCriticalState() {
  // Restore from Preferences or LittleFS
  _prefs.begin("recovery", true);
  unsigned long lastSave = _prefs.getULong("last_save", 0);
  unsigned int bootCount = _prefs.getUInt("boot_count", 0);
  _prefs.end();
  
  Serial.printf("📦 Recovery state: boot_count=%u, last_save=%lu\n", bootCount, lastSave);
  
  // If multiple rapid restarts, might indicate a problem
  if (bootCount > 5) {
    Serial.println("⚠️ High boot count detected - possible boot loop");
  }
}

bool ErrorRecovery::shouldRestart() {
  unsigned long now = millis();
  
  // Reset counter if window expired
  if (now - _lastCriticalError > CRITICAL_ERROR_WINDOW_MS) {
    _criticalErrorCount = 0;
  }
  
  return _criticalErrorCount >= MAX_CRITICAL_ERRORS;
}

void ErrorRecovery::handleCriticalError(const String& component, const String& message) {
  logError(ErrorSeverity::CRITICAL, component, message);
  saveCriticalState();
  
  // Give time for state to save
  delay(500);
}

unsigned int ErrorRecovery::getErrorCount(ErrorSeverity minSeverity) {
  _prefs.begin("errors", true);
  unsigned int count = _prefs.getUInt("error_count", 0);
  _prefs.end();
  return count;
}

void ErrorRecovery::clearErrorLog() {
  _prefs.begin("errors", false);
  _prefs.putUInt("error_count", 0);
  _prefs.end();
  _criticalErrorCount = 0;
}

