// src/components/ControlPanel.tsx
import React, { FC, useCallback, useEffect, useState } from 'react';
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
  Tooltip,
  Menu,
  Dialog,
  DialogTitle,
  DialogContent,
  DialogActions,
  TextField,
  IconButton,
} from '@mui/material';
import SaveIcon from '@mui/icons-material/Save';
import FolderIcon from '@mui/icons-material/Folder';
import DeleteIcon from '@mui/icons-material/Delete';
import DownloadIcon from '@mui/icons-material/Download';
import UploadIcon from '@mui/icons-material/Upload';
import AddIcon from '@mui/icons-material/Add';
import CustomTheme from '../CustomTheme';
import RelayWizard from '../components/RelayWizard';
import { PresetManager, Preset } from '../utils/presetManager';
import {
  fetchRelayConfig,
  getDeviceStates,
  saveConfig,
  fetchSensorParameters,
  SensorParameterOption,
  DeviceStates,
} from '../api/ControllerService';

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

const deviceStateMap: Record<DeviceType, keyof DeviceStates | null> = {
  none: null,
  waterPump: 'pumpState',
  extractorFan: 'extractorFanState',
  intakeFan: 'intakeFanState',
  lights: 'lightsState',
};

const DEFAULT_UNIT: Record<string, string> = {
  soilMoisture: '%',
  temperature: '°C',
  CO2: 'ppm',
  humidity: '%',
  ph: '',
  TVOC: 'ppb',
  airquality: '',
  nitro: 'mg/kg',
  phos: 'mg/kg',
  potas: 'mg/kg',
  total: 'µmol/m²/s',
  ndvi: '',
  greenIntensity: 'lux',
  tdsSens: '%',
};

const ControlPanel: FC = () => {
  const { enqueueSnackbar } = useSnackbar();
  const [controllerData, setControllerData] = useState<DeviceStates | null>(null);
  const [loading, setLoading] = useState(false);
  const [parameters, setParameters] = useState<SensorParameterOption[]>([]);
  const [assignments, setAssignments] = useState<Record<string, RelayAssignment>>(
    Object.fromEntries(
      PORTS.map((pin) => [
        pin,
        { deviceType: 'none', parameter: '', unit: '', min: 0, max: 100 },
      ])
    ) as Record<string, RelayAssignment>
  );
  const [presetMenuAnchor, setPresetMenuAnchor] = useState<null | HTMLElement>(null);
  const [presets, setPresets] = useState<Preset[]>([]);
  const [presetDialogOpen, setPresetDialogOpen] = useState(false);
  const [presetName, setPresetName] = useState('');
  const [presetDescription, setPresetDescription] = useState('');
  const [relayWizardOpen, setRelayWizardOpen] = useState(false);

  // Load available sensor parameters
  useEffect(() => {
    fetchSensorParameters()
      .then(setParameters)
      .catch((e) => console.error('fetchSensorParameters error', e));
  }, []);

  // Load presets
  useEffect(() => {
    setPresets(PresetManager.loadAllPresets());
  }, []);

  // Load merged relay+threshold config
  const loadConfig = useCallback(async () => {
    if (parameters.length === 0) {
      return;
    }
    setLoading(true);
    try {
      const { relays } = await fetchRelayConfig();
      setAssignments((prev) => {
        const upd = { ...prev };
        PORTS.forEach((pin) => {
          const pinNum = parseInt(pin.replace('D', ''), 10);
          const row = relays.find((r) => r.pin === pinNum);
          if (row) {
            const option = parameters.find((p) => p.id === row.parameter);
            upd[pin] = {
              deviceType: row.id as DeviceType,
              parameter: row.parameter,
              unit: option?.unit || DEFAULT_UNIT[row.parameter] || '',
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
  }, [enqueueSnackbar, parameters]);

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
  }, [loadConfig]);

  // Handlers
  const setDevType = (pin: string, dt: DeviceType) => {
    setAssignments((a) => ({ ...a, [pin]: { ...a[pin], deviceType: dt } }));
  };
  const setParam = (pin: string, param: string) => {
    const option = parameters.find((p) => p.id === param);
    setAssignments((a) => ({
      ...a,
      [pin]: {
        deviceType: a[pin].deviceType,
        parameter: param,
        unit: option?.unit || DEFAULT_UNIT[param] || '',
        min: 0,
        max: 100,
      },
    }));
  };
  const setThresh = (pin: string, min: number, max: number) => {
    // Validate: min should be <= max
    const validMin = Math.min(min, max);
    const validMax = Math.max(min, max);
    setAssignments((a) => ({ ...a, [pin]: { ...a[pin], min: validMin, max: validMax } }));
  };

  // Preset management
  const handleSavePreset = () => {
    const thresholds: Record<string, { min: number; max: number }> = {};
    PORTS.forEach((pin) => {
      const a = assignments[pin];
      if (a.deviceType !== 'none' && a.parameter) {
        thresholds[`${pin}_${a.parameter}`] = { min: a.min, max: a.max };
      }
    });

    PresetManager.savePreset({
      name: presetName || `Preset ${presets.length + 1}`,
      description: presetDescription,
      thresholds,
    });
    setPresets(PresetManager.loadAllPresets());
    setPresetDialogOpen(false);
    setPresetName('');
    setPresetDescription('');
    enqueueSnackbar('Preset saved!', { variant: 'success' });
  };

  const handleLoadPreset = (preset: Preset) => {
    setPresetMenuAnchor(null);
    const updated = { ...assignments };
    PORTS.forEach((pin) => {
      const a = assignments[pin];
      if (a.parameter) {
        const key = `${pin}_${a.parameter}`;
        const threshold = preset.thresholds[key];
        if (threshold) {
          updated[pin] = { ...a, min: threshold.min, max: threshold.max };
        }
      }
    });
    setAssignments(updated);
    enqueueSnackbar(`Preset "${preset.name}" loaded!`, { variant: 'success' });
  };

  const handleDeletePreset = (id: string) => {
    PresetManager.deletePreset(id);
    setPresets(PresetManager.loadAllPresets());
    enqueueSnackbar('Preset deleted', { variant: 'info' });
  };

  const handleExportPresets = () => {
    const json = PresetManager.exportPresets();
    const blob = new Blob([json], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `sensor-presets-${Date.now()}.json`;
    a.click();
    URL.revokeObjectURL(url);
    enqueueSnackbar('Presets exported!', { variant: 'success' });
  };

  const handleImportPresets = (event: React.ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0];
    if (!file) return;
    const reader = new FileReader();
    reader.onload = (e) => {
      const text = e.target?.result as string;
      const result = PresetManager.importPresets(text);
      if (result.success) {
        setPresets(PresetManager.loadAllPresets());
        enqueueSnackbar(`Imported ${result.count} preset(s)!`, { variant: 'success' });
      } else {
        enqueueSnackbar(`Import failed: ${result.error}`, { variant: 'error' });
      }
    };
    reader.readAsText(file);
    event.target.value = '';
  };

  const handleExportConfig = () => {
    const snapshot = {
      exportedAt: new Date().toISOString(),
      assignments,
    };
    const blob = new Blob([JSON.stringify(snapshot, null, 2)], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `relay-config-${Date.now()}.json`;
    a.click();
    URL.revokeObjectURL(url);
    enqueueSnackbar('Configuration exported!', { variant: 'success' });
  };

  const handleImportConfig = (event: React.ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0];
    if (!file) return;
    const reader = new FileReader();
    reader.onload = (e) => {
      try {
        const text = e.target?.result as string;
        const parsed = JSON.parse(text);
        if (!parsed || typeof parsed !== 'object' || typeof parsed.assignments !== 'object') {
          throw new Error('Invalid configuration file');
        }
        const incoming = parsed.assignments as Record<string, RelayAssignment>;
        const normalized: Record<string, RelayAssignment> = { ...assignments };
        PORTS.forEach((pin) => {
          if (incoming[pin]) {
            normalized[pin] = { ...incoming[pin] };
          }
        });
        setAssignments(normalized);
        enqueueSnackbar('Configuration imported. Click "Save All" to push to the device.', { variant: 'info' });
      } catch (err: any) {
        enqueueSnackbar(`Import failed: ${err?.message || 'Unknown error'}`, { variant: 'error' });
      }
    };
    reader.readAsText(file);
    event.target.value = '';
  };

  const handleRelayWizardComplete = (config: {
    pin: string;
    deviceType: string;
    parameter: string;
    min: number;
    max: number;
  }) => {
    setAssignments(prev => ({
      ...prev,
      [config.pin]: {
        deviceType: config.deviceType as DeviceType,
        parameter: config.parameter,
        unit: parameters.find(p => p.id === config.parameter)?.unit || DEFAULT_UNIT[config.parameter] || '',
        min: config.min,
        max: config.max,
      },
    }));
    enqueueSnackbar('Relay control added successfully!', { variant: 'success' });
  };

  // Save All
 const onSaveAll = async () => {
  // Validate all assignments before saving
  const invalidAssignments = PORTS.filter((pinLabel) => {
    const a = assignments[pinLabel];
    return a.deviceType !== 'none' && a.parameter && a.min >= a.max;
  });

  if (invalidAssignments.length > 0) {
    enqueueSnackbar('Please fix invalid thresholds (Min must be less than Max)', { variant: 'error' });
    return;
  }

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
          <Box sx={{ display: 'flex', gap: 1, mb: 2, flexWrap: 'wrap' }}>
            <Tooltip title="Add a new relay control using the setup wizard">
              <Button
                variant="contained"
                color="primary"
                onClick={() => setRelayWizardOpen(true)}
                startIcon={<AddIcon />}
              >
                Add Relay Control
              </Button>
            </Tooltip>
            <Tooltip title="Save all relay configurations and thresholds to device">
              <Button
                variant="contained"
                onClick={onSaveAll}
                disabled={loading}
                startIcon={<SaveIcon />}
              >
                Save All Settings
              </Button>
            </Tooltip>
            <Button
              variant="outlined"
              onClick={(e) => setPresetMenuAnchor(e.currentTarget)}
              startIcon={<FolderIcon />}
            >
              Presets
            </Button>
            <Menu
              anchorEl={presetMenuAnchor}
              open={Boolean(presetMenuAnchor)}
              onClose={() => setPresetMenuAnchor(null)}
            >
              <MenuItem onClick={() => { setPresetDialogOpen(true); setPresetMenuAnchor(null); }}>
                <SaveIcon sx={{ mr: 1 }} /> Save Current as Preset
              </MenuItem>
              {presets.length > 0 && (
                <>
                  <MenuItem disabled>Load Preset:</MenuItem>
                  {presets.map((preset) => (
                    <MenuItem key={preset.id} onClick={() => handleLoadPreset(preset)}>
                      <Box sx={{ display: 'flex', justifyContent: 'space-between', width: '100%', alignItems: 'center' }}>
                        <Box>
                          <Typography variant="body2">{preset.name}</Typography>
                          {preset.description && (
                            <Typography variant="caption" color="text.secondary">
                              {preset.description}
                            </Typography>
                          )}
                        </Box>
                        <IconButton
                          size="small"
                          onClick={(e) => {
                            e.stopPropagation();
                            handleDeletePreset(preset.id);
                          }}
                          sx={{ ml: 1 }}
                        >
                          <DeleteIcon fontSize="small" />
                        </IconButton>
                      </Box>
                    </MenuItem>
                  ))}
                </>
              )}
              <MenuItem onClick={handleExportPresets}>
                <DownloadIcon sx={{ mr: 1 }} /> Export Presets
              </MenuItem>
              <MenuItem component="label">
                <UploadIcon sx={{ mr: 1 }} /> Import Presets
                <input type="file" hidden accept=".json" onChange={handleImportPresets} />
              </MenuItem>
            </Menu>
            <Tooltip title="Download the current relay configuration as JSON">
              <Button variant="outlined" startIcon={<DownloadIcon />} onClick={handleExportConfig}>
                Export Config
              </Button>
            </Tooltip>
            <Tooltip title="Load relay configuration from JSON">
              <Button variant="outlined" component="label" startIcon={<UploadIcon />}>
                Import Config
                <input type="file" hidden accept=".json" onChange={handleImportConfig} />
              </Button>
            </Tooltip>
          </Box>

          <TableContainer component={Paper} sx={{ boxShadow: 3 }}>
            <Table>
              <TableHead>
                <TableRow sx={{ backgroundColor: 'primary.main' }}>
                  <TableCell sx={{ color: 'white', fontWeight: 'bold' }}>GPIO Pin</TableCell>
                  <TableCell sx={{ color: 'white', fontWeight: 'bold' }}>Controlled Device</TableCell>
                  <TableCell sx={{ color: 'white', fontWeight: 'bold' }}>Current State</TableCell>
                  <TableCell sx={{ color: 'white', fontWeight: 'bold' }}>Control Parameter</TableCell>
                  <TableCell sx={{ color: 'white', fontWeight: 'bold' }}>Activation Range</TableCell>
                </TableRow>
              </TableHead>

              <TableBody>
                {PORTS.map((pin) => {
                  const a = assignments[pin];
                  const stateKey = deviceStateMap[a.deviceType];
                  const isOn = stateKey && controllerData ? controllerData[stateKey] : false;
                  return (
                    <TableRow 
                      key={pin}
                      sx={{ 
                        '&:hover': { backgroundColor: 'action.hover' },
                        transition: 'background-color 0.2s',
                      }}
                    >
                      <TableCell sx={{ fontFamily: 'monospace', fontWeight: 500, color: 'text.secondary' }}>
                        GPIO {pin}
                      </TableCell>
                      <TableCell>
                        <Select
                          value={a.deviceType}
                          onChange={(e) => setDevType(pin, e.target.value as DeviceType)}
                          disabled={loading}
                          fullWidth
                          size="small"
                        >
                          {DEVICE_TYPES.map((t) => (
                            <MenuItem key={t.value} value={t.value}>
                              {t.label}
                            </MenuItem>
                          ))}
                        </Select>
                      </TableCell>
                      <TableCell>
                        <Chip 
                          label={isOn ? 'Active' : 'Inactive'} 
                          color={isOn ? 'success' : 'default'}
                          size="small"
                          variant={isOn ? 'filled' : 'outlined'}
                        />
                      </TableCell>
                      <TableCell>
                        <Select
                          value={a.parameter}
                          onChange={(e) => setParam(pin, e.target.value as string)}
                          disabled={loading}
                          displayEmpty
                          fullWidth
                          size="small"
                        >
                          <MenuItem value="">
                            <em>None</em>
                          </MenuItem>
                          {parameters.map((p) => (
                            <MenuItem key={p.id} value={p.id}>
                              {p.label}
                              {p.unit ? ` (${p.unit})` : ''}
                            </MenuItem>
                          ))}
                        </Select>
                      </TableCell>
                      <TableCell>
                        {a.parameter ? (
                          <Box sx={{ px: 1 }}>
                            <Slider
                              value={[a.min, a.max]}
                              onChange={(_, v) =>
                                Array.isArray(v) && setThresh(pin, v[0], v[1])
                              }
                              valueLabelDisplay="auto"
                              min={0}
                              max={a.parameter.toUpperCase() === 'CO2' ? 2000 : a.parameter.toUpperCase() === 'PH' ? 14 : 200}
                              step={a.parameter.toUpperCase() === 'CO2' ? 10 : a.parameter.toUpperCase() === 'PH' ? 0.1 : 1}
                              sx={{ mb: 1 }}
                              size="small"
                            />
                            <Box display="flex" justifyContent="space-between" alignItems="center" gap={1}>
                              <Typography variant="caption" color={a.min >= a.max ? 'error' : 'text.secondary'}>
                                Min: {a.min.toFixed(a.parameter.toUpperCase() === 'PH' ? 1 : 0)} {a.unit}
                              </Typography>
                              {a.min >= a.max && (
                                <Typography variant="caption" color="error" sx={{ fontStyle: 'italic', fontSize: '0.7rem' }}>
                                  Invalid range
                                </Typography>
                              )}
                              <Typography variant="caption" color="text.secondary">
                                Max: {a.max.toFixed(a.parameter.toUpperCase() === 'PH' ? 1 : 0)} {a.unit}
                              </Typography>
                            </Box>
                          </Box>
                        ) : (
                          <Typography variant="body2" color="text.disabled" sx={{ fontStyle: 'italic' }}>
                            Not configured
                          </Typography>
                        )}
                      </TableCell>
                    </TableRow>
                  );
                })}
              </TableBody>
            </Table>
          </TableContainer>

          {/* Save Preset Dialog */}
          <Dialog open={presetDialogOpen} onClose={() => setPresetDialogOpen(false)}>
            <DialogTitle>Save Preset</DialogTitle>
            <DialogContent>
              <TextField
                autoFocus
                margin="dense"
                label="Preset Name"
                fullWidth
                value={presetName}
                onChange={(e) => setPresetName(e.target.value)}
                sx={{ mb: 2 }}
              />
              <TextField
                margin="dense"
                label="Description (optional)"
                fullWidth
                multiline
                rows={3}
                value={presetDescription}
                onChange={(e) => setPresetDescription(e.target.value)}
              />
            </DialogContent>
            <DialogActions>
              <Button onClick={() => setPresetDialogOpen(false)}>Cancel</Button>
              <Button onClick={handleSavePreset} variant="contained" disabled={!presetName.trim()}>
                Save
              </Button>
            </DialogActions>
          </Dialog>

          <RelayWizard
            open={relayWizardOpen}
            onClose={() => setRelayWizardOpen(false)}
            onComplete={handleRelayWizardComplete}
            availablePins={PORTS}
            parameters={parameters}
          />
        </Box>
      </SnackbarProvider>
    </CustomTheme>
  );
};

export default ControlPanel;
