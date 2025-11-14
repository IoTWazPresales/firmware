#include "RateLimiter.h"

RateLimiter::RateLimiter(unsigned int maxRequests, unsigned long windowMs)
    : _maxRequests(maxRequests), _windowMs(windowMs), _lastCleanup(0) {}

bool RateLimiter::checkLimit(const String& identifier) {
  cleanup();
  
  unsigned long now = millis();
  auto it = _entries.find(identifier);
  
  if (it == _entries.end()) {
    // First request from this identifier
    RateLimitEntry entry;
    entry.lastRequest = now;
    entry.requestCount = 1;
    entry.windowStart = now;
    _entries[identifier] = entry;
    return true;
  }
  
  RateLimitEntry& entry = it->second;
  
  // Check if window has expired
  if (now - entry.windowStart >= _windowMs) {
    // Reset window
    entry.windowStart = now;
    entry.requestCount = 1;
    entry.lastRequest = now;
    return true;
  }
  
  // Check if limit exceeded
  if (entry.requestCount >= _maxRequests) {
    return false;
  }
  
  // Increment counter
  entry.requestCount++;
  entry.lastRequest = now;
  return true;
}

void RateLimiter::reset(const String& identifier) {
  _entries.erase(identifier);
}

unsigned int RateLimiter::getRemaining(const String& identifier) const {
  cleanup();
  
  auto it = _entries.find(identifier);
  if (it == _entries.end()) {
    return _maxRequests;
  }
  
  const RateLimitEntry& entry = it->second;
  unsigned long now = millis();
  
  if (now - entry.windowStart >= _windowMs) {
    return _maxRequests;
  }
  
  return _maxRequests - entry.requestCount;
}

void RateLimiter::cleanup() const {
  unsigned long now = millis();
  
  // Cleanup every 60 seconds to prevent memory leaks
  if (now - _lastCleanup < 60000) {
    return;
  }
  _lastCleanup = now;
  
  // Remove entries older than 2 windows
  auto it = _entries.begin();
  while (it != _entries.end()) {
    if (now - it->second.windowStart > (_windowMs * 2)) {
      it = _entries.erase(it);
    } else {
      ++it;
    }
  }
}

