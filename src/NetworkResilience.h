#pragma once

#include <Arduino.h>

class NetworkResilience {
public:
  NetworkResilience(unsigned long initialBackoffMs = 1000, 
                    unsigned long maxBackoffMs = 60000,
                    float backoffMultiplier = 2.0f);
  
  bool shouldRetry();
  void recordSuccess();
  void recordFailure();
  unsigned long getNextDelay();
  void reset();
  
  unsigned int getFailureCount() const { return _failureCount; }
  bool isHealthy() const { return _failureCount == 0; }
  
private:
  unsigned long _initialBackoffMs;
  unsigned long _maxBackoffMs;
  float _backoffMultiplier;
  unsigned int _failureCount;
  unsigned long _currentBackoffMs;
  unsigned long _lastFailureTime;
  static const unsigned int MAX_FAILURES = 10;
};

