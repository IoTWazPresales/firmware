#include <WiFiStatus.h>
WiFiStatus::WiFiStatus(AsyncWebServer* server) {
  server->on(WIFI_STATUS_SERVICE_PATH, HTTP_GET, [this](AsyncWebServerRequest *request) {
    wifiStatus(request);
});
  WiFi.onEvent(onStationModeConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
  WiFi.onEvent(onStationModeDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  WiFi.onEvent(onStationModeGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
}

void WiFiStatus::onStationModeConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println(F("WiFi Connected."));
}

void WiFiStatus::onStationModeDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println(F("WiFi Disconnected. Reason code="));
  Serial.println(info.wifi_sta_disconnected.reason);
}

void WiFiStatus::onStationModeGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("WiFi Got IP. localIP=%s, hostName=%s\r\n");
      Serial.println(WiFi.localIP().toString().c_str());
      Serial.println(WiFi.getHostname());
}

void WiFiStatus::wifiStatus(AsyncWebServerRequest* request) {
  AsyncJsonResponse* response = new AsyncJsonResponse(false, MAX_WIFI_STATUS_SIZE);
  JsonObject root = response->getRoot();
  wl_status_t status = WiFi.status();
  root["status"] = (uint8_t)status;
  if (status == WL_CONNECTED) {
    root["local_ip"] = WiFi.localIP().toString();
    root["mac_address"] = WiFi.macAddress();
    root["rssi"] = WiFi.RSSI();
    root["ssid"] = WiFi.SSID();
    root["bssid"] = WiFi.BSSIDstr();
    root["channel"] = WiFi.channel();
    root["subnet_mask"] = WiFi.subnetMask().toString();
    root["gateway_ip"] = WiFi.gatewayIP().toString();
    IPAddress dnsIP1 = WiFi.dnsIP(0);
    IPAddress dnsIP2 = WiFi.dnsIP(1);
    if (IPUtils::isSet(dnsIP1)) {
      root["dns_ip_1"] = dnsIP1.toString();
    }
    if (IPUtils::isSet(dnsIP2)) {
      root["dns_ip_2"] = dnsIP2.toString();
    }
  }
  response->setLength();
  request->send(response);
}
