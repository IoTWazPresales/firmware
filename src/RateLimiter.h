#pragma once

#include <Arduino.h>
#include <map>

struct RateLimitEntry {
  unsigned long lastRequest;
  unsigned int requestCount;
  unsigned long windowStart;
};

class RateLimiter {
public:
  RateLimiter(unsigned int maxRequests, unsigned long windowMs);
  
  bool checkLimit(const String& identifier);
  void reset(const String& identifier);
  unsigned int getRemaining(const String& identifier) const;
  
private:
  unsigned int _maxRequests;
  unsigned long _windowMs;
  mutable std::map<String, RateLimitEntry> _entries;
  mutable unsigned long _lastCleanup;
  
  void cleanup() const;
};

