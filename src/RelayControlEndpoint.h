#pragma once
#include <ESPAsyncWebServer.h>
#include "SupabaseConnector.h"
class RelayControl;

class RelayControlEndpoint {
public:
    RelayControlEndpoint(AsyncWebServer* server, RelayControl* relayControl, SupabaseConnector* supabase = nullptr);
private:
  AsyncWebServer* _server;
  RelayControl*   _relayControl;
      SupabaseConnector* _supabase;

  void handleRelayData();
      void syncRelayStateToSupabase();

};
