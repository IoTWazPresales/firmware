#pragma once

#include <Arduino.h>
#include <Preferences.h>

enum class ErrorSeverity {
  INFO = 0,
  WARNING = 1,
  ERROR = 2,
  CRITICAL = 3
};

struct ErrorRecord {
  ErrorSeverity severity;
  unsigned long timestamp;
  String message;
  String component;
};

class ErrorRecovery {
public:
  static void logError(ErrorSeverity severity, const String& component, const String& message);
  static void saveCriticalState();
  static void restoreCriticalState();
  static bool shouldRestart();
  static void handleCriticalError(const String& component, const String& message);
  
  static unsigned int getErrorCount(ErrorSeverity minSeverity = ErrorSeverity::ERROR);
  static void clearErrorLog();
  
private:
  static Preferences _prefs;
  static unsigned int _criticalErrorCount;
  static unsigned long _lastCriticalError;
  static const unsigned int MAX_CRITICAL_ERRORS = 3;
  static const unsigned long CRITICAL_ERROR_WINDOW_MS = 300000; // 5 minutes
};

