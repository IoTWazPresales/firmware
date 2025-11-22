# Boot Loop Analysis & Fix Plan

## Current Status
- Device is in a boot loop with **no Serial output**
- Crash happens during global object construction (before `setup()` runs)
- Boot count: 83+ (indicates persistent loop)

## Root Causes Identified

### 1. **WiFi Conflict (PRIMARY ISSUE)**
- **Two WiFi managers fighting:**
  - `WiFi_Task.cpp` - FreeRTOS task with hardcoded credentials
  - `WiFiSettingsService` - AP mode manager
- **Status:** WiFi_Task disabled, but issue persists

### 2. **TCP/IP Stack Not Initialized**
- Error: `assert failed: tcpip_api_call ... (Invalid mbox)`
- `server.begin()` called before TCP/IP stack initialized
- TCP/IP stack initializes when WiFi starts
- **Status:** Fixed by initializing WiFi before server

### 3. **Global Constructor Issues**
- Many global objects constructed before `setup()`
- Some may access LittleFS/WiFi before initialization
- **Status:** Moved WiFi init out of constructors

## Files That May Be Problematic

### Global Objects (constructed before setup):
1. `AsyncWebServer server(80)` - Should be OK
2. `ESP32React esp32React(&server)` - Registers routes in constructor
3. `DeviceScanner scanner(&server)` - May access hardware
4. `SensorManager sensorManager(&server)` - May access I2C
5. `WiFiSettingsService wifiSettingsService(&server)` - Now empty constructor
6. `DriverPackageManager driverPackageManager(&LittleFS)` - Stores pointer only
7. `FactoryResetService factoryResetService(&server, &LittleFS)` - Stores pointers

### Potential Issues:
- `ESP32React` constructor registers routes - could be problematic
- `DeviceScanner` constructor might access hardware
- `SensorManager` constructor might access I2C

## Recommended Fix Strategy

### Option 1: Revert to Last Working State
- Go back to commit before WiFi AP mode changes
- Add AP mode more incrementally
- Test after each small change

### Option 2: Minimal WiFi Init
- Initialize WiFi in `setup()` BEFORE any other services
- Don't use WiFiSettingsService initially
- Just start AP mode directly in setup()
- Add WiFiSettingsService later once basic AP works

### Option 3: Check Each Global Constructor
- Systematically check each global object constructor
- Ensure none access hardware/filesystem before setup()
- Move any problematic initialization to `begin()` methods

## Next Steps

1. **Immediate:** Try Option 2 - minimal WiFi init in setup()
2. **If that fails:** Revert to last known working commit
3. **Then:** Add features incrementally with testing

