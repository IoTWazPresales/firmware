#pragma once

#include <Arduino.h>

enum class LogLevel {
  DEBUG = 0,
  INFO = 1,
  WARN = 2,
  ERROR = 3
};

class Logger {
public:
  static void setLevel(LogLevel level);
  static void debug(const char* format, ...);
  static void info(const char* format, ...);
  static void warn(const char* format, ...);
  static void error(const char* format, ...);
  
private:
  static LogLevel _level;
  static void log(LogLevel level, const char* prefix, const char* format, va_list args);
};

