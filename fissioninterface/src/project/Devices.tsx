// src/components/Devices.tsx

import React, { FC, useCallback, useEffect, useState } from 'react';
import '../App.css';
import { SnackbarProvider, useSnackbar } from 'notistack';
import CustomTheme from '../CustomTheme';
import {
  Table, TableBody, TableCell, TableContainer,
  TableHead, TableRow, Paper, Select, MenuItem,
  Button, Box, Typography, Chip, Tooltip, Dialog,
  DialogTitle, DialogContent, DialogActions, DialogContentText,
  TextField, InputAdornment,
} from '@mui/material';
import SearchIcon from '@mui/icons-material/Search';

import {
  fetchScannedDevices,
  triggerScan,
  ScanResults,
  DigitalPin,
  AnalogPin,
} from '../api/DeviceScannerService';

import {
  fetchSensorAssignments,
  saveAllSensorAssignments,
  resetSensorAssignments,
} from '../api/DeviceConfigService';
import DriverPackageService, { DriverPackageSummary } from '../api/DriverPackageService';

const ANALOG_PORTS = [
  { label: "J1", pin: 36, key: "A0" },
  { label: "J2", pin: 39, key: "A1" },
  { label: "J3", pin: 34, key: "A2" },
  { label: "J4", pin: 35, key: "A3" },
];

const DIGITAL_PORTS = [
  { label: "J5",  pin: 25, key: "D2"  },
  { label: "J6",  pin: 26, key: "D3"  },
  { label: "J7",  pin:  0, key: "D5"  },
  { label: "J8",  pin: 14, key: "D6"  },
  { label: "J9",  pin: 13, key: "D7"  },
  { label: "J10", pin:  2, key: "D9"  },
  { label: "J11", pin:  4, key: "D12" },
  { label: "J12", pin: 12, key: "D13" },
];

const I2C_PORTS = [
  { label: 'J1', key: 'J1' },
  { label: 'J2', key: 'J2' },
  { label: 'J3', key: 'J3' },
  { label: 'J4', key: 'J4' },
];

const SENSOR_TYPES = [
  { value: '',                label: 'None' },
  { value: 'airTempHumidity', label: 'DHT11 (Air Temp & Humidity)' },
  { value: 'soilMoisture',    label: 'Soil Moisture' },
  { value: 'ph',              label: 'pH Sensor' },
  { value: 'tds',             label: 'TDS Sensor' },
  { value: 'temperature',     label: 'DS18B20 Temp' },
  { value: 'npk',             label: 'NPK Sensor' },
  { value: 'airQuality',      label: 'ENS160 Air Quality' },
  { value: 'spectral',        label: 'AS7341 Spectral' },
  { value: 'rtc',             label: 'DS3231 RTC' },
];

const Devices: FC = () => {
  const { enqueueSnackbar } = useSnackbar();
  const [scan, setScan] = useState<ScanResults>({
    i2cDevices: [], digitalPins: [], analogPins: []
  });
  const [assignments, setAssignments] = useState<Record<string,string>>(() => {
    const init: Record<string,string> = {};
    I2C_PORTS.forEach(p => { init[p.key] = ''; });
    return init;
  });
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [packages, setPackages] = useState<DriverPackageSummary[]>([]);
  const [packageBusy, setPackageBusy] = useState(false);
  const [resetConfirmOpen, setResetConfirmOpen] = useState(false);
  const [searchFilter, setSearchFilter] = useState<string>('');

  // load saved assignments once
  useEffect(() => {
    (async () => {
      try {
        const saved = await fetchSensorAssignments();
        setAssignments(prev => ({ ...prev, ...saved }));
      } catch {
        enqueueSnackbar('Could not load saved assignments.', { variant: 'warning' });
      }
    })();
  }, [enqueueSnackbar]);

  // keep assignment keys in sync
  useEffect(() => {
    setAssignments(prev => {
      const upd = { ...prev };
      scan.digitalPins.forEach((p: DigitalPin) => {
        if (!(p.key in upd)) upd[p.key] = '';
      });
      scan.analogPins.forEach((p: AnalogPin) => {
        if (!(p.key in upd)) upd[p.key] = '';
      });
      return upd;
    });
  }, [scan.digitalPins, scan.analogPins]);

  // just fetch the *last* scan (fast)
  const doScan = useCallback(async () => {
    setLoading(true);
    try {
      const data = await fetchScannedDevices();
      setScan(data);
      setError(null);
    } catch {
      setError('Could not fetch scan results.');
      enqueueSnackbar('Failed to fetch devices.', { variant: 'error' });
    } finally {
      setLoading(false);
    }
  }, [enqueueSnackbar]);

  // POST + fetch for manual “Scan Now”
  const handleTrigger = async () => {
    setLoading(true);
    try {
      await triggerScan();
      const data = await fetchScannedDevices();
      setScan(data);
      enqueueSnackbar('Scan complete!', { variant: 'success' });
    } catch {
      enqueueSnackbar('Scan failed.', { variant: 'error' });
    } finally {
      setLoading(false);
    }
  };

  // save all assignments
  const handleSaveAll = async () => {
    try {
      await saveAllSensorAssignments(assignments);
      enqueueSnackbar('All assignments saved!', { variant: 'success' });
    } catch {
      enqueueSnackbar('Failed to save assignments.', { variant: 'error' });
    }
  };

  // reset config
  const handleReset = async () => {
    setResetConfirmOpen(false);
    setLoading(true);
    try {
      await resetSensorAssignments();
      // wipe out all except I²C
      setAssignments(() => {
        const init: Record<string,string> = {};
        I2C_PORTS.forEach(p => { init[p.key] = ''; });
        return init;
      });
      enqueueSnackbar('Configuration reset!', { variant: 'info' });
    } catch (e: any) {
      enqueueSnackbar(`Reset failed: ${e.message}`, { variant: 'error' });
    } finally {
      setLoading(false);
    }
  };

  const refreshPackages = useCallback(async () => {
    try {
      const list = await DriverPackageService.listPackages();
      setPackages(list);
    } catch (e) {
      console.error(e);
      enqueueSnackbar('Failed to load driver packages', { variant: 'warning' });
    }
  }, [enqueueSnackbar]);

  const handlePackageUpload = async (event: React.ChangeEvent<HTMLInputElement>) => {
    if (!event.target.files || event.target.files.length === 0) {
      return;
    }
    const file = event.target.files[0];
    setPackageBusy(true);
    try {
      await DriverPackageService.uploadPackage(file);
      enqueueSnackbar('Driver package uploaded', { variant: 'success' });
      await refreshPackages();
    } catch (e: any) {
      enqueueSnackbar(`Upload failed: ${e?.message ?? 'unknown error'}`, { variant: 'error' });
    } finally {
      event.target.value = '';
      setPackageBusy(false);
    }
  };

  // on mount & every 30s poll the last scan
  useEffect(() => {
    doScan();
    const iv = setInterval(doScan, 30000);
    return () => clearInterval(iv);
  }, [doScan]);

  useEffect(() => {
    refreshPackages();
  }, [refreshPackages]);

  const handleChange = (key: string, type: string) =>
    setAssignments(prev => ({ ...prev, [key]: type }));
  

  return (
    <CustomTheme>
      <SnackbarProvider maxSnack={3} anchorOrigin={{ vertical:'bottom', horizontal:'left' }}>
        <Box sx={{ m:2, overflowX:'auto' }}>
        <Box sx={{ display:'flex', justifyContent:'space-between', mb:2, gap:1, flexWrap: 'wrap', alignItems: 'center' }}>
          <TextField
            size="small"
            placeholder="Search ports or sensors..."
            value={searchFilter}
            onChange={(e) => setSearchFilter(e.target.value)}
            InputProps={{
              startAdornment: (
                <InputAdornment position="start">
                  <SearchIcon />
                </InputAdornment>
              ),
            }}
            sx={{ minWidth: 250, flexGrow: { xs: 1, sm: 0 } }}
          />
          <Box sx={{ display:'flex', gap:1, flexWrap: 'wrap' }}>
            <Tooltip title="Scan for connected sensors on all ports">
              <Button variant="outlined" onClick={handleTrigger} disabled={loading}>
                Scan Now
              </Button>
            </Tooltip>
            <Tooltip title="Save all sensor assignments to device">
              <Button variant="contained" onClick={handleSaveAll} disabled={loading}>
                Save All
              </Button>
            </Tooltip>
            <Tooltip title="Reset all sensor assignments">
              <Button variant="outlined" color="error" onClick={() => setResetConfirmOpen(true)} disabled={loading}>
                Reset
              </Button>
            </Tooltip>
          </Box>
        </Box>

        {error && (
          <Paper sx={{ p: 2, mb: 2, backgroundColor: 'error.light', color: 'error.contrastText' }}>
            <Typography color="inherit" align="center">{error}</Typography>
          </Paper>
        )}

        <TableContainer 
          component={Paper} 
          sx={{ 
            boxShadow: 3,
            overflowX: 'auto',
            maxWidth: '100%',
          }}
        >
          <Table sx={{ minWidth: 650 }}>
            <TableHead>
              <TableRow sx={{ backgroundColor: 'primary.main' }}>
                <TableCell sx={{ color:'white', fontWeight:'bold' }}>Port</TableCell>
                <TableCell sx={{ color:'white', fontWeight:'bold' }}>Pin #</TableCell>
                <TableCell sx={{ color:'white', fontWeight:'bold' }}>Detected?</TableCell>
                <TableCell sx={{ color:'white', fontWeight:'bold' }}>Sensor</TableCell>
              </TableRow>
            </TableHead>
            <TableBody>
              {/* Analog JST ports */}
              {ANALOG_PORTS.filter(({ label, key }) => {
                if (!searchFilter) return true;
                const filter = searchFilter.toLowerCase();
                const assignment = assignments[key] || '';
                return label.toLowerCase().includes(filter) || 
                       key.toLowerCase().includes(filter) ||
                       assignment.toLowerCase().includes(filter);
              }).map(({ label, pin, key }) => {
                const entry = scan.analogPins.find(p => p.pin === pin);
                const detected = entry?.detected ?? false;
                return (
                  <TableRow key={key} sx={{ opacity: detected ? 1 : 0.4 }}>
                    <TableCell>{label}</TableCell>
                    <TableCell>{pin}</TableCell>
                    <TableCell>
                      <Chip label={detected ? 'Yes' : 'No'}
                            color={detected ? 'success' : 'default'} />
                    </TableCell>
                    <TableCell>
                      <Select
                        value={assignments[key] || ''}
                        onChange={e => handleChange(key, e.target.value as string)}
                        disabled={!detected}
                        fullWidth
                      >
                        {SENSOR_TYPES.map(t => (
                          <MenuItem key={t.value} value={t.value}>
                            {t.label}
                          </MenuItem>
                        ))}
                      </Select>
                    </TableCell>
                  </TableRow>
                );
              })}

              {/* Digital JST ports */}
              {DIGITAL_PORTS.filter(({ label, key }) => {
                if (!searchFilter) return true;
                const filter = searchFilter.toLowerCase();
                const assignment = assignments[key] || '';
                return label.toLowerCase().includes(filter) || 
                       key.toLowerCase().includes(filter) ||
                       assignment.toLowerCase().includes(filter);
              }).map(({ label, pin, key }) => {
                const entry = scan.digitalPins.find(p => p.pin === pin);
                const detected = entry?.detected ?? false;
                return (
                  <TableRow key={key} sx={{ opacity: detected ? 1 : 0.4 }}>
                    <TableCell>{label}</TableCell>
                    <TableCell>{pin}</TableCell>
                    <TableCell>
                      <Chip label={detected ? 'Yes' : 'No'}
                            color={detected ? 'success' : 'default'} />
                    </TableCell>
                    <TableCell>
                      <Select
                        value={assignments[key] || ''}
                        onChange={e => handleChange(key, e.target.value as string)}
                        disabled={!detected}
                        fullWidth
                      >
                        {SENSOR_TYPES.map(t => (
                          <MenuItem key={t.value} value={t.value}>
                            {t.label}
                          </MenuItem>
                        ))}
                      </Select>
                    </TableCell>
                  </TableRow>
                );
              })}

              {/* I²C sockets */}
              <TableRow>
                <TableCell colSpan={4}
                           sx={{ backgroundColor:'#eee', fontStyle:'italic' }}>
                  I²C Sockets
                </TableCell>
              </TableRow>
              {I2C_PORTS.filter(({ label, key }) => {
                if (!searchFilter) return true;
                const filter = searchFilter.toLowerCase();
                const assignment = assignments[key] || '';
                return label.toLowerCase().includes(filter) || 
                       key.toLowerCase().includes(filter) ||
                       assignment.toLowerCase().includes(filter);
              }).map(({label, key}, idx) => {
                const detected = scan.i2cDevices.length > idx;
                return (
                  <TableRow key={key}>
                    <TableCell>{label}</TableCell>
                    <TableCell>
                      {detected ? scan.i2cDevices[idx] : '--'}
                    </TableCell>
                    <TableCell>
                      <Chip
                        label={detected ? 'Yes' : 'No'}
                        color={detected ? 'success' : 'default'}
                      />
                    </TableCell>
                    <TableCell>
                      <Select
                        value={assignments[key] || ''}
                        onChange={e => handleChange(key, e.target.value as string)}
                        disabled={!detected}
                        fullWidth
                      >
                        {SENSOR_TYPES.map(t => (
                          <MenuItem key={t.value} value={t.value}>
                            {t.label}
                          </MenuItem>
                        ))}
                      </Select>
                    </TableCell>
                  </TableRow>
                );
              })}
            </TableBody>
          </Table>
        </TableContainer>

        {loading && (
          <Typography align="center" sx={{ mt:2 }}>
            Scanning…
          </Typography>
        )}

        <Paper sx={{ mt:4, p:2 }}>
          <Box sx={{ display:'flex', justifyContent:'space-between', alignItems:'center', mb:2 }}>
            <Typography variant="h6">Driver Packages</Typography>
            <Tooltip title="Upload a driver package JSON file to install new sensor drivers">
              <Button component="label" variant="contained" disabled={packageBusy}>
                Upload Package
                <input type="file" hidden accept=".json" onChange={handlePackageUpload} />
              </Button>
            </Tooltip>
          </Box>
          {packages.length === 0 ? (
            <Typography color="text.secondary">No packages installed yet.</Typography>
          ) : (
            <Table size="small">
              <TableHead>
                <TableRow>
                  <TableCell>Name</TableCell>
                  <TableCell align="right">Size (bytes)</TableCell>
                </TableRow>
              </TableHead>
              <TableBody>
                {packages.map((pkg) => (
                  <TableRow key={pkg.name}>
                    <TableCell>{pkg.name}</TableCell>
                    <TableCell align="right">{pkg.size.toLocaleString()}</TableCell>
                  </TableRow>
                ))}
              </TableBody>
            </Table>
          )}
        </Paper>

        {/* Reset Confirmation Dialog */}
        <Dialog
          open={resetConfirmOpen}
          onClose={() => setResetConfirmOpen(false)}
        >
          <DialogTitle>Confirm Reset</DialogTitle>
          <DialogContent>
            <DialogContentText>
              Are you sure you want to reset all sensor assignments? This action cannot be undone.
            </DialogContentText>
          </DialogContent>
          <DialogActions>
            <Button onClick={() => setResetConfirmOpen(false)}>Cancel</Button>
            <Button onClick={handleReset} color="error" variant="contained">
              Reset
            </Button>
          </DialogActions>
        </Dialog>
      </Box>
      </SnackbarProvider>
    </CustomTheme>
  );
};

export default Devices;
