// src/layout/Layout.tsx

import React, { FC, useState, useEffect } from 'react';
import { useLocation } from 'react-router-dom';
import {
  Box,
  Toolbar,
  Dialog,
  DialogTitle,
  DialogContent,
  DialogActions,
  TextField,
  Button,
  Alert,
  Snackbar,
  Typography,
} from '@mui/material';

import LayoutDrawer from './LayoutDrawer';
import LayoutAppBar from './LayoutAppBar';
import { LayoutContext } from './context';

export const DRAWER_WIDTH = 280;

// — mDNS + path constants —
const MDNS_HOSTNAME    = 'neurogrow.local';
const WIFI_STATUS_PATH = '/api/wifiStatus';

/**
 * Hit http://neurogrow.local/api/wifiStatus,
 * parse its JSON and return `local_ip`.
 */
async function discoverDeviceIP(): Promise<string> {
  const url = `http://${MDNS_HOSTNAME}${WIFI_STATUS_PATH}`;
  const resp = await fetch(url, { mode: 'cors' });
  if (!resp.ok) throw new Error(`HTTP ${resp.status}`);
  const contentType = resp.headers.get('content-type') || '';
  if (!contentType.includes('application/json')) {
    const text = await resp.text();
    throw new Error('Expected JSON, got:\n' + text);
  }
  const body = await resp.json() as { local_ip?: string };
  if (!body.local_ip) throw new Error('No local_ip in response');
  return body.local_ip;
}

interface Props {
  children: React.ReactNode;
}

const Layout: FC<Props> = ({ children }) => {
  const [mobileOpen, setMobileOpen]         = useState(false);
  const [title, setTitle]                   = useState('ESP32 Control');
  const { pathname }                        = useLocation();

  // ← track cloud-connected state
  const [isCloudConnected, setIsCloudConnected] = useState(false);

  // — dialog state —
  const [connectOpen, setConnectOpen]      = useState(false);
  const [isDetectingIP, setIsDetectingIP]  = useState(false);
  const [esp32Ip, setEsp32Ip]              = useState('');
  const [apiKey, setApiKey] = useState(''); // Changed from setupCode to apiKey
  const [isConnecting, setIsConnecting]    = useState(false);
  const [message, setMessage]              = useState('');
  const [messageType, setMessageType]      = useState<'success'|'error'>('success');
  const [showMessage, setShowMessage]      = useState(false);

  // Close drawer on every route change
  useEffect(() => setMobileOpen(false), [pathname]);

  // When the dialog opens, kick off mDNS discovery
  useEffect(() => {
    if (!connectOpen) return;
    setEsp32Ip('');
    setIsDetectingIP(true);

    discoverDeviceIP()
      .then(ip => {
        setEsp32Ip(ip);
      })
      .catch(err => {
        console.error('Discovery error:', err);
        setMessage('Could not find ESP32 at neurogrow.local');
        setMessageType('error');
        setShowMessage(true);
      })
      .finally(() => setIsDetectingIP(false));
  }, [connectOpen]);

  const handleConnectClick = () => setConnectOpen(true);
  const handleConnectClose = () => {
    setConnectOpen(false);
    setApiKey('');
    setEsp32Ip('');
    setIsConnecting(false);
  };

  const handleRegister = async () => {
    if (!esp32Ip || !apiKey.trim()) {
      setMessage('Enter setup code after IP detection');
      setMessageType('error');
      setShowMessage(true);
      return;
    }

    setIsConnecting(true);
    let success = false;
    let lastError = '';

    const urls = [
      `http://${esp32Ip}/connect?api_key=${encodeURIComponent(apiKey)}`,
      `http://${esp32Ip}:80/connect?api_key=${encodeURIComponent(apiKey)}`,
      `http://${esp32Ip}:8080/connect?api_key=${encodeURIComponent(apiKey)}`,
    ];

    for (const url of urls) {
      try {

         console.log('Attempting API key registration at:', url);
        const resp = await fetch(url, {
          method: 'POST',
          headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
         
        });

         console.log('Registration response status:', resp.status);
        const responseText = await resp.text();
        console.log('Registration response body:', responseText);


        if (resp.ok) {
          setMessage('Device connected to cloud successfully!');
          setMessageType('success');
          setIsCloudConnected(true);
          setConnectOpen(false);
          success = true;
           setApiKey('');
          setEsp32Ip('');
          success = true;
          break;
        } else {
           lastError = `${resp.status} ${resp.statusText}: ${responseText}`;
        }
      } catch (err: any) {
         lastError = err.message || 'Connection failed';
        console.error('Registration error:', err);
      }
    }

    if (!success) {
      setMessage(`Registration failed: ${lastError}`);
      setMessageType('error');
      setShowMessage(true);
    }

    setIsConnecting(false);
  };
    // ← new: clear creds on device
  const handleDisconnect = async () => {
    if (!esp32Ip) {
      setMessage('No device IP available for disconnect');
      setMessageType('error');
      setShowMessage(true);
      return;
    }

    try {
      const urls = [
        `http://${esp32Ip}/disconnect`,
        `http://${esp32Ip}:80/disconnect`,
        `http://${esp32Ip}:8080/disconnect`,
      ];

      let success = false;
      for (const url of urls) {
        try {
          const resp = await fetch(url, { method: 'POST' });
          if (resp.ok) {
            success = true;
            break;
          }
        } catch (err) {
          console.error('Disconnect attempt failed:', err);
        }
      }

      if (success) {
        setIsCloudConnected(false);
        setMessage('Device disconnected; credentials cleared.');
        setMessageType('success');
        setShowMessage(true);
      } else {
        throw new Error('Could not reach device');
      }
    } catch (err: any) {
      setMessage('Disconnect failed: ' + err.message);
      setMessageType('error');
      setShowMessage(true);
    }
  };
  return (
    <LayoutContext.Provider value={{ title, setTitle }}>
      {/* Top AppBar (now receives isConnected) */}
      <LayoutAppBar
        title={title}
        onToggleDrawer={() => setMobileOpen(o => !o)}
        onConnect={handleConnectClick}
        onDisconnect={handleDisconnect}
        isConnected={isCloudConnected}
      />

      {/* Side Drawer */}
      <LayoutDrawer
        mobileOpen={mobileOpen}
        onClose={() => setMobileOpen(false)}
      />

      {/* Main Content */}
      <Box
        component="main"
        sx={{
          ml: { xs: 0, md: `${DRAWER_WIDTH}px` },
          p: 2,
        }}
      >
        <Toolbar />
        {children}
      </Box>

      {/* Connect Dialog */}
      <Dialog
        open={connectOpen}
        onClose={handleConnectClose}
        maxWidth="sm"
        fullWidth
      >
        <DialogTitle>Connect ESP32 to Cloud</DialogTitle>
        <DialogContent>
          <Box sx={{ display: 'flex', flexDirection: 'column', gap: 2, mt: 1 }}>
            {isDetectingIP ? (
              <Typography>Detecting device on neurogrow.local…</Typography>
            ) : esp32Ip ? (
              <TextField
                label="ESP32 IP Address"
                value={esp32Ip}
                fullWidth
                InputProps={{ readOnly: true }}
                helperText="Discovered via mDNS"
              />
            ) : (
              <Typography color="text.secondary">
                No device found. Check your Wi-Fi or mDNS settings.
              </Typography>
            )}

            <TextField
             label="Device API Key (from Device Registration)"
              placeholder="Enter your device API key (gapi_...)"
              value={apiKey}
              onChange={e => setApiKey(e.target.value)}
              fullWidth
              disabled={isConnecting || isDetectingIP}
              helperText="Get this API key from the Device Registration page in your dashboard"
            />
          </Box>
        </DialogContent>
        <DialogActions>
          <Button onClick={handleConnectClose} disabled={isConnecting}>
            Cancel
          </Button>
          <Button
            onClick={handleRegister}
            variant="contained"
            disabled={
              isConnecting ||
              isDetectingIP ||
              !esp32Ip ||
              !apiKey.trim()
            }
          >
            {isConnecting ? 'Registering…' : 'Register'}
          </Button>
        </DialogActions>
      </Dialog>

      {/* Snackbar for feedback */}
      <Snackbar
        open={showMessage}
        autoHideDuration={6000}
        onClose={() => setShowMessage(false)}
        anchorOrigin={{ vertical: 'bottom', horizontal: 'right' }}
      >
        <Alert
          onClose={() => setShowMessage(false)}
          severity={messageType}
          sx={{ width: '100%' }}
        >
          {message}
        </Alert>
      </Snackbar>
    </LayoutContext.Provider>
  );
};

export default Layout;
