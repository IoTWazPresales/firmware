#pragma once
#include "SupabaseConnector.h"

// Call this once after your SupabaseConnector is up:
//    initSupabaseSyncTask(&supabaseConnector);
void initSupabaseSyncTask(SupabaseConnector* connector);
