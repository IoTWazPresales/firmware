#include "NetworkResilience.h"

NetworkResilience::NetworkResilience(unsigned long initialBackoffMs,
                                     unsigned long maxBackoffMs,
                                     float backoffMultiplier)
    : _initialBackoffMs(initialBackoffMs),
      _maxBackoffMs(maxBackoffMs),
      _backoffMultiplier(backoffMultiplier),
      _failureCount(0),
      _currentBackoffMs(initialBackoffMs),
      _lastFailureTime(0) {}

bool NetworkResilience::shouldRetry() {
  if (_failureCount == 0) {
    return true;
  }
  
  if (_failureCount >= MAX_FAILURES) {
    return false; // Give up after max failures
  }
  
  unsigned long now = millis();
  return (now - _lastFailureTime) >= _currentBackoffMs;
}

void NetworkResilience::recordSuccess() {
  _failureCount = 0;
  _currentBackoffMs = _initialBackoffMs;
  _lastFailureTime = 0;
}

void NetworkResilience::recordFailure() {
  _failureCount++;
  _lastFailureTime = millis();
  
  // Exponential backoff
  _currentBackoffMs = (unsigned long)(_currentBackoffMs * _backoffMultiplier);
  if (_currentBackoffMs > _maxBackoffMs) {
    _currentBackoffMs = _maxBackoffMs;
  }
}

unsigned long NetworkResilience::getNextDelay() {
  return _currentBackoffMs;
}

void NetworkResilience::reset() {
  _failureCount = 0;
  _currentBackoffMs = _initialBackoffMs;
  _lastFailureTime = 0;
}

