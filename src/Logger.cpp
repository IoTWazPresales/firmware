#include "Logger.h"
#include <cstdarg>

LogLevel Logger::_level = LogLevel::INFO;

void Logger::setLevel(LogLevel level) {
  _level = level;
}

void Logger::log(LogLevel level, const char* prefix, const char* format, va_list args) {
  if (level < _level) return;
  
  char buffer[256];
  vsnprintf(buffer, sizeof(buffer), format, args);
  Serial.printf("[%s] %s\n", prefix, buffer);
}

void Logger::debug(const char* format, ...) {
  va_list args;
  va_start(args, format);
  log(LogLevel::DEBUG, "DEBUG", format, args);
  va_end(args);
}

void Logger::info(const char* format, ...) {
  va_list args;
  va_start(args, format);
  log(LogLevel::INFO, "INFO", format, args);
  va_end(args);
}

void Logger::warn(const char* format, ...) {
  va_list args;
  va_start(args, format);
  log(LogLevel::WARN, "WARN", format, args);
  va_end(args);
}

void Logger::error(const char* format, ...) {
  va_list args;
  va_start(args, format);
  log(LogLevel::ERROR, "ERROR", format, args);
  va_end(args);
}

