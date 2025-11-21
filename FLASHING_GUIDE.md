# Flashing NeuroGrow Firmware to ESP32

## Quick Steps

### Option 1: VS Code + PlatformIO (Recommended)

1. **Open VS Code**
   - Make sure PlatformIO extension is installed

2. **Open the Project**
   - File → Open Folder
   - Navigate to: `C:\Projects\Cannabox\firmware`
   - Or if you need to pull latest: Clone/pull the repo first

3. **Pull Latest Code (if needed)**
   ```bash
   git pull origin phase-1-2-4-wireless-integration
   ```

4. **Connect ESP32**
   - Plug ESP32 into USB port
   - Note the COM port (e.g., COM3, COM4)
   - Update `platformio.ini` if your COM port is different:
     ```ini
     upload_port = COM3  # Change to your port
     ```

5. **Build the Firmware**
   - Click PlatformIO icon in sidebar
   - Click "Build" (✓ icon) or press `Ctrl+Alt+B`
   - Wait for build to complete

6. **Upload Firmware**
   - Click "Upload" (→ icon) or press `Ctrl+Alt+U`
   - Firmware will be compiled and uploaded to ESP32

7. **Upload Filesystem (LittleFS)**
   - Click PlatformIO → Project Tasks → dfrobot_firebeetle2_esp32e
   - Click "Upload Filesystem Image"
   - This uploads the web interface and manifests

8. **Monitor Serial Output**
   - Click "Monitor" (plug icon) or press `Ctrl+Alt+S`
   - You should see boot messages and WiFi connection status

### Option 2: Command Line (PlatformIO CLI)

If you prefer command line:

```bash
# Navigate to project directory
cd C:\Projects\Cannabox\firmware

# Pull latest code (if needed)
git pull origin phase-1-2-4-wireless-integration

# Build firmware
pio run -e dfrobot_firebeetle2_esp32e

# Upload firmware
pio run -e dfrobot_firebeetle2_esp32e -t upload

# Upload filesystem
pio run -e dfrobot_firebeetle2_esp32e -t uploadfs

# Monitor serial
pio device monitor
```

## Important Notes

### Before Flashing

1. **Check COM Port**
   - Windows: Device Manager → Ports (COM & LPT)
   - Update `upload_port` in `platformio.ini` if needed

2. **Check Board Type**
   - Current config: `dfrobot_firebeetle2_esp32e`
   - If you have a different ESP32 board, update `platformio.ini`:
     ```ini
     board = your_board_name
     ```

3. **First Time Setup**
   - After flashing, ESP32 will create WiFi Access Point
   - Connect to "NeuroGrow" network
   - Go to http://192.168.4.1
   - Configure WiFi settings

### After Flashing

1. **Check Serial Monitor**
   - Should see: "NeuroGrow initialized"
   - WiFi connection status
   - IP address when connected

2. **Access Web Interface**
   - If WiFi connected: http://[ESP32_IP]
   - If AP mode: http://192.168.4.1
   - Or use mDNS: http://neurogrow.local

3. **Verify Features**
   - Dashboard loads
   - Sensors detected
   - WebSocket connection works

## Troubleshooting

### Upload Fails
- Check COM port is correct
- Try different USB cable/port
- Hold BOOT button during upload
- Check drivers are installed

### Build Errors
- Run `pio lib install` to update libraries
- Check `platformio.ini` configuration
- Verify all files are present

### Filesystem Upload Fails
- Make sure LittleFS is selected in `platformio.ini`
- Check partition table (`partitions.csv`)
- Try erasing flash first: `pio run -t erase`

### Device Not Found
- Check USB connection
- Install ESP32 drivers (CP2102 or CH340)
- Try different USB port

## Current Configuration

- **Board**: DFRobot FireBeetle 2 ESP32-E
- **Flash Size**: 16MB
- **PSRAM**: Enabled
- **Filesystem**: LittleFS
- **Upload Port**: COM3 (update if different)

## Next Steps After Flashing

1. Connect to WiFi
2. Access web interface
3. Configure sensors
4. Test relay controls
5. Verify Supabase connection (if configured)

