// src/components/ControlPanel.tsx
import React, { FC, useEffect, useState } from 'react';
import { SnackbarProvider, useSnackbar } from 'notistack';
import {
  Table,
  TableBody,
  TableCell,
  TableContainer,
  TableHead,
  TableRow,
  Paper,
  Button,
  Box,
  Typography,
  Chip,
  Select,
  MenuItem,
  Slider,
} from '@mui/material';
import CustomTheme from '../CustomTheme';
import {
  fetchRelayConfig,
  getDeviceStates,
  saveConfig,
  fetchSensorParameters,
} from '../api/ControllerService';

interface ControllerData {
  pumpState: boolean;
  intakeFanState: boolean;
  extractorFanState: boolean;
  lightsState: boolean;
}

type DeviceType = 'none' | 'waterPump' | 'extractorFan' | 'intakeFan' | 'lights';

interface RelayAssignment {
  deviceType: DeviceType;
  parameter: string;
  unit: string;
  min: number;
  max: number;
}

const PORTS = ['18', '19', '23', '25'] as const;


const DEVICE_TYPES = [
  { value: 'none', label: 'None' },
  { value: 'waterPump', label: 'Water Pump' },
  { value: 'extractorFan', label: 'Extractor Fan' },
  { value: 'intakeFan', label: 'Intake Fan' },
  { value: 'lights', label: 'Lights' },
];

const deviceStateMap: Record<DeviceType, keyof ControllerData | null> = {
  none: null,
  waterPump: 'pumpState',
  extractorFan: 'extractorFanState',
  intakeFan: 'intakeFanState',
  lights: 'lightsState',
};

const DEFAULT_UNIT: Record<string, string> = {
  soilMoisture: '%',
  temperature: '°C',
  co2: 'ppm',
  humidity: '%',
  ph: '',
};

const ControlPanel: FC = () => {
  const { enqueueSnackbar } = useSnackbar();
  const [controllerData, setControllerData] = useState<ControllerData | null>(null);
  const [loading, setLoading] = useState(false);
  const [parameters, setParameters] = useState<string[]>([]);
  const [assignments, setAssignments] = useState<Record<string, RelayAssignment>>(
    Object.fromEntries(
      PORTS.map((pin) => [
        pin,
        { deviceType: 'none', parameter: '', unit: '', min: 0, max: 100 },
      ])
    ) as Record<string, RelayAssignment>
  );

  // Load available sensor parameters
  useEffect(() => {
    fetchSensorParameters()
      .then(setParameters)
      .catch((e) => console.error('fetchSensorParameters error', e));
  }, []);

  // Load merged relay+threshold config
  const loadConfig = async () => {
    setLoading(true);
    try {
      const { relays } = await fetchRelayConfig();
      setAssignments((prev) => {
        const upd = { ...prev };
        PORTS.forEach((pin) => {
          const pinNum = parseInt(pin.replace('D', ''), 10);
          const row = relays.find((r) => r.pin === pinNum);
          if (row) {
            upd[pin] = {
              deviceType: row.id as DeviceType,
              parameter: row.parameter,
              unit: DEFAULT_UNIT[row.parameter] || '',
              min: row.min,
              max: row.max,
            };
          } else {
            upd[pin] = { deviceType: 'none', parameter: '', unit: '', min: 0, max: 100 };
          }
        });
        return upd;
      });
    } catch (e: any) {
      enqueueSnackbar('Error loading config: ' + e.message, { variant: 'error' });
    } finally {
      setLoading(false);
    }
  };

  // Initial load + poll device states
  useEffect(() => {
    loadConfig();
    const poll = async () => {
      const ds = await getDeviceStates();
      setControllerData(ds);
    };
    poll();
    const interval = setInterval(poll, 5000);
    return () => clearInterval(interval);
  }, [enqueueSnackbar]);

  // Handlers
  const setDevType = (pin: string, dt: DeviceType) => {
    setAssignments((a) => ({ ...a, [pin]: { ...a[pin], deviceType: dt } }));
  };
  const setParam = (pin: string, param: string) => {
    setAssignments((a) => ({
      ...a,
      [pin]: {
        deviceType: a[pin].deviceType,
        parameter: param,
        unit: DEFAULT_UNIT[param] || '',
        min: 0,
        max: 100,
      },
    }));
  };
  const setThresh = (pin: string, min: number, max: number) => {
    setAssignments((a) => ({ ...a, [pin]: { ...a[pin], min, max } }));
  };

  // Save All
 const onSaveAll = async () => {
  setLoading(true);
  try {
    // Build an array of properly typed relay entries:
   const relays = PORTS.map((pinLabel) => {
  const a = assignments[pinLabel];
  return {
    id:        a.deviceType,
    pin:       pinLabel,       // ← send the string
    parameter: a.parameter,
    min:       a.min,
    max:       a.max,
  };
}).filter(r => r.id !== 'none' && r.parameter);

    // And send exactly { relays: SaveRelay[] }
    const { status, message } = await saveConfig({ relays });
    if (status === 'error') throw new Error(message);

    enqueueSnackbar('All settings saved!', { variant: 'success' });
    await loadConfig();
  } catch (e: any) {
    enqueueSnackbar('Save failed: ' + e.message, { variant: 'error' });
  } finally {
    setLoading(false);
  }
};

  return (
    <CustomTheme>
      <SnackbarProvider maxSnack={3} anchorOrigin={{ vertical: 'bottom', horizontal: 'left' }}>
        <Box sx={{ m: 2, overflowX: 'auto' }}>
          <Button
            variant="contained"
            onClick={onSaveAll}
            disabled={loading}
            sx={{ mb: 2 }}
          >
            Save All Settings
          </Button>

          <TableContainer component={Paper} sx={{ boxShadow: 3 }}>
            <Table>
              <TableHead>
                <TableRow sx={{ backgroundColor: '#1976d2' }}>
                  <TableCell sx={{ color: 'white', fontWeight: 'bold' }}>Pin</TableCell>
                  <TableCell sx={{ color: 'white', fontWeight: 'bold' }}>Device Type</TableCell>
                  <TableCell sx={{ color: 'white', fontWeight: 'bold' }}>Status</TableCell>
                  <TableCell sx={{ color: 'white', fontWeight: 'bold' }}>Parameter</TableCell>
                  <TableCell sx={{ color: 'white', fontWeight: 'bold' }}>Thresholds</TableCell>
                </TableRow>
              </TableHead>

              <TableBody>
                {PORTS.map((pin) => {
                  const a = assignments[pin];
                  const stateKey = deviceStateMap[a.deviceType];
                  const isOn = stateKey && controllerData ? controllerData[stateKey] : false;
                  return (
                    <TableRow key={pin}>
                      <TableCell>{pin}</TableCell>
                      <TableCell>
                        <Select
                          value={a.deviceType}
                          onChange={(e) => setDevType(pin, e.target.value as DeviceType)}
                          disabled={loading}
                          fullWidth
                        >
                          {DEVICE_TYPES.map((t) => (
                            <MenuItem key={t.value} value={t.value}>
                              {t.label}
                            </MenuItem>
                          ))}
                        </Select>
                      </TableCell>
                      <TableCell>
                        <Chip label={isOn ? 'On' : 'Off'} color={isOn ? 'success' : 'error'} />
                      </TableCell>
                      <TableCell>
                        <Select
                          value={a.parameter}
                          onChange={(e) => setParam(pin, e.target.value as string)}
                          disabled={loading}
                          displayEmpty
                          fullWidth
                        >
                          <MenuItem value="">
                            <em>None</em>
                          </MenuItem>
                          {parameters.map((p) => (
                            <MenuItem key={p} value={p}>
                              {p}
                            </MenuItem>
                          ))}
                        </Select>
                      </TableCell>
                      <TableCell>
                        {a.parameter ? (
                          <>
                            <Slider
                              value={[a.min, a.max]}
                              onChange={(_, v) =>
                                Array.isArray(v) && setThresh(pin, v[0], v[1])
                              }
                              valueLabelDisplay="auto"
                              min={0}
                              max={200}
                              step={a.parameter === 'co2' ? 10 : 0.1}
                            />
                            <Box display="flex" justifyContent="space-between">
                              <Typography variant="body2">
                                Min: {a.min}
                                {a.unit}
                              </Typography>
                              <Typography variant="body2">
                                Max: {a.max}
                                {a.unit}
                              </Typography>
                            </Box>
                          </>
                        ) : (
                          <Typography variant="body2">N/A</Typography>
                        )}
                      </TableCell>
                    </TableRow>
                  );
                })}
              </TableBody>
            </Table>
          </TableContainer>
        </Box>
      </SnackbarProvider>
    </CustomTheme>
  );
};

export default ControlPanel;
