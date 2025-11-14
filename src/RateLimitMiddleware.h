#pragma once

#include <ESPAsyncWebServer.h>
#include "RateLimiter.h"

class RateLimitMiddleware {
public:
  static bool checkRateLimit(AsyncWebServerRequest* request, 
                             RateLimiter& limiter,
                             unsigned int maxRequests = 60,
                             unsigned long windowMs = 60000);
  
  static String getClientIdentifier(AsyncWebServerRequest* request);
};

