#pragma once

#include <ESPAsyncWebServer.h>
#include <Preferences.h>

class AuthMiddleware {
public:
  static bool checkApiKey(AsyncWebServerRequest* request);
  static bool checkAdminAuth(AsyncWebServerRequest* request);
  static void setApiKey(const String& key);
  static void setAdminPassword(const String& password);
  
private:
  static String _apiKey;
  static String _adminPassword;
  static Preferences _prefs;
};

