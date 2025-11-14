#include "AuthMiddleware.h"
#include <Arduino.h>

String AuthMiddleware::_apiKey = "";
String AuthMiddleware::_adminPassword = "";
Preferences AuthMiddleware::_prefs;

bool AuthMiddleware::checkApiKey(AsyncWebServerRequest* request) {
  // Allow public endpoints
  String url = request->url();
  if (url.startsWith("/api/wifiStatus") || 
      url.startsWith("/api/systemStatus") ||
      url.startsWith("/api/telemetry") ||
      url.startsWith("/api/manifests") ||
      url.startsWith("/api/sensor") ||
      url.startsWith("/api/scan")) {
    return true; // Public read-only endpoints
  }
  
  // Check for API key in headers
  if (request->hasHeader("x-device-api-key")) {
    String provided = request->header("x-device-api-key");
    if (provided.length() > 0 && provided == _apiKey) {
      return true;
    }
  }
  
  // Check in Authorization header
  if (request->hasHeader("Authorization")) {
    String auth = request->header("Authorization");
    if (auth.startsWith("Bearer ")) {
      String token = auth.substring(7);
      if (token == _apiKey) {
        return true;
      }
    }
  }
  
  // Load from preferences if not set
  if (_apiKey.length() == 0) {
    _prefs.begin("auth", true);
    _apiKey = _prefs.getString("api_key", "");
    _prefs.end();
  }
  
  // If no API key configured, allow access (initial setup)
  if (_apiKey.length() == 0) {
    return true;
  }
  
  return false;
}

bool AuthMiddleware::checkAdminAuth(AsyncWebServerRequest* request) {
  if (request->hasHeader("Authorization")) {
    String auth = request->header("Authorization");
    if (auth.startsWith("Basic ")) {
      // Basic auth check (simplified - in production use proper base64 decode)
      String provided = auth.substring(6);
      if (provided == _adminPassword) {
        return true;
      }
    }
  }
  
  if (_adminPassword.length() == 0) {
    _prefs.begin("auth", true);
    _adminPassword = _prefs.getString("admin_pwd", "");
    _prefs.end();
  }
  
  // If no password set, allow (initial setup)
  if (_adminPassword.length() == 0) {
    return true;
  }
  
  return false;
}

void AuthMiddleware::setApiKey(const String& key) {
  _apiKey = key;
  _prefs.begin("auth", false);
  _prefs.putString("api_key", key);
  _prefs.end();
}

void AuthMiddleware::setAdminPassword(const String& password) {
  _adminPassword = password;
  _prefs.begin("auth", false);
  _prefs.putString("admin_pwd", password);
  _prefs.end();
}

