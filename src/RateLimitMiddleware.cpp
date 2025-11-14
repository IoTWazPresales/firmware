#include "RateLimitMiddleware.h"
#include <WiFi.h>

String RateLimitMiddleware::getClientIdentifier(AsyncWebServerRequest* request) {
  // Use IP address as identifier
  return request->client()->remoteIP().toString();
}

bool RateLimitMiddleware::checkRateLimit(AsyncWebServerRequest* request,
                                         RateLimiter& limiter,
                                         unsigned int maxRequests,
                                         unsigned long windowMs) {
  String identifier = getClientIdentifier(request);
  
  if (!limiter.checkLimit(identifier)) {
    // Rate limit exceeded
    AsyncResponseStream* response = request->beginResponseStream("application/json");
    response->setCode(429);
    response->addHeader("Retry-After", String(windowMs / 1000));
    response->printf("{\"status\":\"error\",\"message\":\"Rate limit exceeded\",\"retry_after\":%lu}",
                     windowMs / 1000);
    request->send(response);
    return false;
  }
  
  return true;
}

