import React, { FC, RefObject, useEffect, useState } from 'react';
import '../App.css';
import { SnackbarProvider, useSnackbar } from 'notistack';
import { IconButton, Box, Paper, Typography, Chip, Stack, FormControlLabel, Switch, LinearProgress } from '@mui/material';
import CloseIcon from '@mui/icons-material/Close';
import SensorCard from '../components/SensorCard';
import CustomTheme from '../CustomTheme';
import SensorService from '../api/SensorService';
import { fetchThresholds, getDeviceStates, Thresholds, DeviceStates } from '../api/ControllerService';
import { SensorCapabilityMeta, SensorValues } from '../types/sensors';
import TelemetryService, { TelemetryResponse } from '../api/TelemetryService';
import { useSensorWebSocket } from '../hooks/useSensorWebSocket';
import SensorHistory, { SensorLogEntry } from '../api/SensorHistory';
import HistoricalTrends from '../components/HistoricalTrends';
import SensorDetailDialog from '../components/SensorDetailDialog';

const clamp = (value: number, min: number, max: number) => Math.min(Math.max(value, min), max);

const formatSensorLabel = (id: string) =>
  id
    .replace(/([A-Z])/g, ' $1')
    .replace(/_/g, ' ')
    .replace(/\s+/g, ' ')
    .replace(/^./, (s) => s.toUpperCase())
    .trim();

const parseNumeric = (value: unknown): number | null => {
  if (typeof value === 'number' && Number.isFinite(value)) {
    return value;
  }
  if (typeof value === 'string') {
    const parsed = parseFloat(value);
    return Number.isFinite(parsed) ? parsed : null;
  }
  return null;
};

const toPositiveNumber = (value: unknown): number => {
  const numeric = parseNumeric(value);
  return numeric !== null && Number.isFinite(numeric) ? numeric : 0;
};

const extractRange = (thresholds: Thresholds | null, key: string) => {
  const range = thresholds?.[key];
  return {
    min: range?.min ?? null,
    max: range?.max ?? null,
  };
};

const formatUptime = (ms: number | undefined) => {
  if (!ms || ms <= 0) {
    return '—';
  }
  const totalSeconds = Math.floor(ms / 1000);
  const days = Math.floor(totalSeconds / 86400);
  const hours = Math.floor((totalSeconds % 86400) / 3600);
  const minutes = Math.floor((totalSeconds % 3600) / 60);
  if (days > 0) {
    return `${days}d ${hours}h ${minutes}m`;
  }
  if (hours > 0) {
    return `${hours}h ${minutes}m`;
  }
  return `${minutes}m`;
};

const formatBytes = (bytes: number | undefined) => {
  if (!bytes || bytes <= 0) {
    return '0 B';
  }
  const units = ['B', 'KB', 'MB', 'GB'];
  let size = bytes;
  let unitIndex = 0;
  while (size >= 1024 && unitIndex < units.length - 1) {
    size /= 1024;
    unitIndex += 1;
  }
  return `${size.toFixed(1)} ${units[unitIndex]}`;
};

type DeviceStateKey = keyof DeviceStates;

type ThresholdSelector = (thresholds: Thresholds | null) => { min?: number | null; max?: number | null };

type ProgressBuilder = (value: number | null) => {
  progressValue?: number;
  healthColor?: (value: number) => string;
};

type ChartDataBuilder = (values: SensorValues) => any;

interface DashboardCardConfig {
  id: string;
  title: string;
  unit?: string;
  chartType: 'sparkline' | 'pie' | 'circular';
  historyKey?: string;
  deviceStateKey?: DeviceStateKey;
  thresholdSelector?: ThresholdSelector;
  chartDataBuilder?: ChartDataBuilder;
  progressBuilder?: ProgressBuilder;
}

const CARD_DEFINITIONS: DashboardCardConfig[] = [
  {
    id: 'airtemp',
    title: 'Air Temperature',
    unit: '°C',
    chartType: 'sparkline',
    deviceStateKey: 'extractorFanState',
    thresholdSelector: (thr) => extractRange(thr, 'minTemp'),
  },
  {
    id: 'temperature',
    title: 'Water Temperature',
    unit: '°C',
    chartType: 'sparkline',
    deviceStateKey: 'extractorFanState',
    thresholdSelector: (thr) => extractRange(thr, 'waterTemp'),
  },
  {
    id: 'humidity',
    title: 'Air Humidity',
    unit: '%',
    chartType: 'sparkline',
    deviceStateKey: 'extractorFanState',
    thresholdSelector: (thr) => extractRange(thr, 'humidity'),
  },
  {
    id: 'soilMoisture',
    title: 'Soil Moisture',
    unit: '%',
    chartType: 'sparkline',
    deviceStateKey: 'pumpState',
    historyKey: 'soilMoisture',
    thresholdSelector: (thr) => extractRange(thr, 'soilMoisture'),
  },
  {
    id: 'tdsSens',
    title: 'Water Particulates',
    unit: '%',
    chartType: 'sparkline',
    deviceStateKey: 'pumpState',
    thresholdSelector: (thr) => extractRange(thr, 'tdsSens'),
  },
  {
    id: 'CO2',
    title: 'CO₂ Levels',
    unit: 'ppm',
    chartType: 'sparkline',
    deviceStateKey: 'intakeFanState',
    thresholdSelector: (thr) => extractRange(thr, 'CO2'),
  },
  {
    id: 'TVOC',
    title: 'TVOC Levels',
    unit: 'ppb',
    chartType: 'sparkline',
    deviceStateKey: 'intakeFanState',
    thresholdSelector: (thr) => extractRange(thr, 'TVOC'),
  },
  {
    id: 'airquality',
    title: 'Air Quality Index',
    unit: '',
    chartType: 'sparkline',
    deviceStateKey: 'intakeFanState',
    thresholdSelector: (thr) => extractRange(thr, 'airquality'),
  },
  {
    id: 'soilph',
    title: 'Soil PH',
    unit: 'pH',
    chartType: 'sparkline',
    deviceStateKey: 'pumpState',
    thresholdSelector: (thr) => extractRange(thr, 'soilph'),
  },
  {
    id: 'nitro',
    title: 'Nitrogen',
    unit: 'mg/kg',
    chartType: 'sparkline',
    deviceStateKey: 'pumpState',
    thresholdSelector: (thr) => extractRange(thr, 'nitro'),
  },
  {
    id: 'phos',
    title: 'Phosphorus',
    unit: 'mg/kg',
    chartType: 'sparkline',
    deviceStateKey: 'pumpState',
    thresholdSelector: (thr) => extractRange(thr, 'phos'),
  },
  {
    id: 'potas',
    title: 'Potassium',
    unit: 'mg/kg',
    chartType: 'sparkline',
    deviceStateKey: 'pumpState',
    thresholdSelector: (thr) => extractRange(thr, 'potas'),
  },
  {
    id: 'total',
    title: 'PAR',
    unit: 'µmol/m²/s',
    chartType: 'pie',
    deviceStateKey: 'lightsState',
    chartDataBuilder: (values) => ({
      Blue: toPositiveNumber(values.blue),
      Green: toPositiveNumber(values.green),
      Red: toPositiveNumber(values.red),
      FarRed: toPositiveNumber(values.farRed ?? values.FarRed),
    }),
  },
  {
    id: 'ndvi',
    title: 'Plant Health (NDVI)',
    unit: '',
    chartType: 'circular',
    deviceStateKey: 'lightsState',
    progressBuilder: (value) => {
      const normalized = clamp((value ?? 0) * 100, 0, 100);
      return {
        progressValue: normalized,
        healthColor: (val: number) => {
          if (val < 20) return 'red';
          if (val < 40) return 'orange';
          if (val < 60) return 'yellow';
          if (val < 80) return 'lightgreen';
          return 'green';
        },
      };
    },
  },
  {
    id: 'greenIntensity',
    title: 'Greenlight Intensity',
    unit: 'lux',
    chartType: 'circular',
    deviceStateKey: 'lightsState',
    progressBuilder: (value) => ({ progressValue: Math.max(0, value ?? 0) }),
    thresholdSelector: (thr) => extractRange(thr, 'greenlight'),
  },
];

const Dashboard: FC = () => {
  const { enqueueSnackbar } = useSnackbar();
  const [sensorData, setSensorData] = useState<SensorValues>({});
  const [thresholds, setThresholds] = useState<Thresholds | null>(null);
  const [deviceStates, setDeviceStates] = useState<DeviceStates | null>(null);
  const [cardConfigs, setCardConfigs] = useState<DashboardCardConfig[]>(CARD_DEFINITIONS);
  const [sensorHistory, setSensorHistory] = useState<Record<string, number[]>>({});
  const [loading, setLoading] = useState<boolean>(true);
  const [telemetry, setTelemetry] = useState<TelemetryResponse | null>(null);
  const [useWebSocket, setUseWebSocket] = useState<boolean>(true);
  const [historyLogs, setHistoryLogs] = useState<SensorLogEntry[]>([]);
  const [historyLoading, setHistoryLoading] = useState<boolean>(false);
  const [historyError, setHistoryError] = useState<string | null>(null);
  const [selectedCard, setSelectedCard] = useState<DashboardCardConfig | null>(null);
  
  // WebSocket integration
  const { connected: wsConnected, sensorData: wsSensorData } = useSensorWebSocket(useWebSocket);
  
  // Merge WebSocket data with polled data (WebSocket takes priority)
  useEffect(() => {
    if (useWebSocket && wsConnected && wsSensorData && Object.keys(wsSensorData).length > 0) {
      setSensorData(wsSensorData);
    }
  }, [wsSensorData, wsConnected, useWebSocket]);

  useEffect(() => {
    const fetchData = async () => {
      try {
        const [sensorResponse, thresholdsResponse, devicesResponse, telemetryResponse] = await Promise.all([
          SensorService.getSensorData(),
          fetchThresholds(),
        getDeviceStates(),
          TelemetryService.fetchTelemetry().catch(() => null),
        ]);

        const values = sensorResponse?.values ?? {};
        const metaList: SensorCapabilityMeta[] = sensorResponse?.meta ?? [];
        setSensorData(values);
        setThresholds(thresholdsResponse ?? null);
        setDeviceStates(devicesResponse ?? null);
        setTelemetry((telemetryResponse as TelemetryResponse | null) ?? null);

        const metaIds = new Set(metaList.map((meta) => meta.id));
        const baseConfigs = metaList.length > 0
          ? CARD_DEFINITIONS.filter((def) => metaIds.has(def.id))
          : CARD_DEFINITIONS;
        const fallbackConfigs: DashboardCardConfig[] = metaList
          .filter((meta) => meta.kind === 'numeric' && !CARD_DEFINITIONS.some((def) => def.id === meta.id))
          .map((meta) => ({
            id: meta.id,
            title: meta.label || formatSensorLabel(meta.id),
            unit: meta.unit ?? '',
            chartType: 'sparkline',
          }));

        const allConfigs = [...baseConfigs, ...fallbackConfigs];
        setCardConfigs(allConfigs);

        setSensorHistory((prev) => {
          const next = { ...prev };

          const updateHistory = (id: string, key: string) => {
            const numeric = parseNumeric(values[id]);
            if (numeric === null) {
              return;
            }
            const existing = next[key] ?? [];
            next[key] = [...existing.slice(-19), numeric];
          };

          allConfigs.forEach((def) => {
            const historyKey = def.historyKey ?? def.id;
            updateHistory(def.id, historyKey);
          });

          return next;
        });
      } catch (error) {
        enqueueSnackbar('Failed to fetch sensor data', { variant: 'error' });
      } finally {
        setLoading(false);
      }
    };

    fetchData();
    // Only poll if WebSocket is not connected or disabled
    const interval = useWebSocket && wsConnected ? null : setInterval(fetchData, 5000);
    return () => {
      if (interval) clearInterval(interval);
    };
  }, [enqueueSnackbar, useWebSocket, wsConnected]);

  useEffect(() => {
    let cancelled = false;
    const fetchHistoryLogs = async () => {
      setHistoryLoading(true);
      setHistoryError(null);
      try {
        const logs = await SensorHistory.getSensorLogs();
        if (!cancelled) {
          setHistoryLogs(logs);
        }
      } catch (error) {
        console.error('Failed to load historical logs', error);
        if (!cancelled) {
          setHistoryError('Unable to load historical sensor data right now.');
        }
      } finally {
        if (!cancelled) {
          setHistoryLoading(false);
        }
      }
    };

    fetchHistoryLogs();
    const interval = setInterval(fetchHistoryLogs, 60000);
    return () => {
      cancelled = true;
      clearInterval(interval);
    };
  }, []);

  const notistackRef: RefObject<any> = React.createRef();
  const onClickDismiss = (key: string | number | undefined) => () => {
    notistackRef.current?.closeSnackbar(key);
  };

  const renderCard = (config: DashboardCardConfig) => {
    const rawValue = sensorData[config.id];
    const numericValue = parseNumeric(rawValue);
    const displayValue = numericValue ?? null;
    const thresholdsResult = config.thresholdSelector ? config.thresholdSelector(thresholds) : {};

    let chartData: any = undefined;
    if (config.chartType === 'sparkline') {
      chartData = sensorHistory[config.historyKey ?? config.id] ?? [];
    } else if (config.chartType === 'pie' && config.chartDataBuilder) {
      chartData = config.chartDataBuilder(sensorData);
    }

    const progressProps = config.progressBuilder ? config.progressBuilder(numericValue) : undefined;
    const deviceStatus = config.deviceStateKey ? deviceStates?.[config.deviceStateKey] ?? false : undefined;

    return (
      <SensorCard
        key={config.id}
        title={config.title}
        value={displayValue}
        unit={config.unit ?? ''}
        loading={loading}
        chartType={config.chartType}
        chartData={chartData}
        minThreshold={thresholdsResult?.min ?? undefined}
        maxThreshold={thresholdsResult?.max ?? undefined}
        deviceStatus={deviceStatus}
        progressValue={progressProps?.progressValue}
        healthColor={progressProps?.healthColor}
        onClick={() => setSelectedCard(config)}
      />
    );
  };

  return (
    <CustomTheme>
      <SnackbarProvider
        maxSnack={2}
        anchorOrigin={{ vertical: 'bottom', horizontal: 'left' }}
        ref={notistackRef}
        action={(key) => (
          <IconButton onClick={onClickDismiss(key)} size="small">
            <CloseIcon />
          </IconButton>
        )}
      >
        <Box sx={{ display: 'flex', flexDirection: 'column', gap: 2, padding: 1 }}>
          <Stack direction={{ xs: 'column', sm: 'row' }} spacing={2} alignItems={{ xs: 'flex-start', sm: 'center' }} justifyContent="space-between">
            <Chip
              label={wsConnected ? 'Realtime stream active' : 'Realtime stream paused'}
              color={wsConnected ? 'success' : 'default'}
            />
            <FormControlLabel
              control={<Switch checked={useWebSocket} onChange={(_, checked) => setUseWebSocket(checked)} />}
              label="Realtime WebSocket"
            />
          </Stack>

          {telemetry && (
            <Paper 
              sx={{ 
                p: 2,
                backgroundColor: 'background.paper',
                borderRadius: 2,
                boxShadow: 2,
              }}
            >
              <Typography variant="h6" gutterBottom sx={{ fontWeight: 600, mb: 2 }}>
                Device Health
              </Typography>
              <Stack direction={{ xs: 'column', sm: 'row' }} spacing={3} useFlexGap flexWrap="wrap">
                <Stack spacing={0.5} sx={{ minWidth: 150 }}>
                  <Typography variant="caption" color="text.secondary" sx={{ textTransform: 'uppercase', letterSpacing: 0.5, fontSize: '0.7rem' }}>
                    Uptime
                  </Typography>
                  <Typography variant="body1" sx={{ fontWeight: 500 }}>{formatUptime(telemetry.uptime_ms)}</Typography>
                </Stack>
                <Stack spacing={0.5} sx={{ minWidth: 150 }}>
                  <Typography variant="caption" color="text.secondary" sx={{ textTransform: 'uppercase', letterSpacing: 0.5, fontSize: '0.7rem' }}>
                    Memory
                  </Typography>
                  <Typography variant="body2" sx={{ fontWeight: 500 }}>
                    {formatBytes(telemetry.free_heap)} free
                  </Typography>
                  <Typography variant="caption" color="text.secondary">
                    Max: {formatBytes(telemetry.max_alloc_heap)}
                  </Typography>
                </Stack>
                <Stack spacing={0.5} sx={{ minWidth: 150 }}>
                  <Typography variant="caption" color="text.secondary" sx={{ textTransform: 'uppercase', letterSpacing: 0.5, fontSize: '0.7rem' }}>
                    Wi-Fi Status
                  </Typography>
                  <Stack direction="row" spacing={1} alignItems="center">
                    <Chip
                      size="small"
                      label={telemetry.wifi_connected ? 'Connected' : 'Offline'}
                      color={telemetry.wifi_connected ? 'success' : 'default'}
                      sx={{ fontWeight: 500 }}
                    />
                    {typeof telemetry.wifi_rssi === 'number' && (
                      <Typography variant="caption" color="text.secondary">
                        {telemetry.wifi_rssi} dBm
                      </Typography>
                    )}
                  </Stack>
                </Stack>
                <Stack spacing={0.5} sx={{ minWidth: 150 }}>
                  <Typography variant="caption" color="text.secondary" sx={{ textTransform: 'uppercase', letterSpacing: 0.5, fontSize: '0.7rem' }}>
                    Message Queues
                  </Typography>
                  <Stack direction="row" spacing={1}>
                    <Chip 
                      size="small" 
                      label={`Hybrid: ${telemetry.queues?.hybrid_available ?? '—'}`}
                      variant="outlined"
                    />
                    <Chip 
                      size="small" 
                      label={`Cloud: ${telemetry.queues?.cloud_available ?? '—'}`}
                      variant="outlined"
                    />
                  </Stack>
                </Stack>
              </Stack>
            </Paper>
          )}

          {cardConfigs.length === 0 ? (
            <Paper sx={{ p: 4, textAlign: 'center' }}>
              <Typography variant="h6" color="text.secondary" gutterBottom>
                No Sensors Configured
              </Typography>
              <Typography variant="body2" color="text.secondary">
                Go to the Devices page to configure your sensors
              </Typography>
            </Paper>
          ) : (
        <Box
          sx={{
            display: 'grid',
                gridTemplateColumns: {
                  xs: '1fr',
                  sm: 'repeat(2, 1fr)',
                  md: 'repeat(3, 1fr)',
                  lg: 'repeat(4, 1fr)',
                },
            gap: 2,
              }}
            >
              {cardConfigs.map(renderCard)}
            </Box>
          )}

          <Box>
            {historyLoading && <LinearProgress sx={{ mb: 1 }} />}
            {historyError && (
              <Typography variant="body2" color="error" sx={{ mb: 1 }}>
                {historyError}
              </Typography>
            )}
            <HistoricalTrends logs={historyLogs} />
          </Box>
        </Box>
        <SensorDetailDialog
          open={Boolean(selectedCard)}
          onClose={() => setSelectedCard(null)}
          sensorName={selectedCard?.title ?? ''}
          unit={selectedCard?.unit ?? ''}
          value={selectedCard ? sensorData[selectedCard.id] ?? null : null}
          history={
            selectedCard
              ? sensorHistory[selectedCard.historyKey ?? selectedCard.id] ?? []
              : []
          }
          thresholds={
            selectedCard?.thresholdSelector
              ? selectedCard.thresholdSelector(thresholds)
              : undefined
          }
          deviceStatus={
            selectedCard?.deviceStateKey
              ? deviceStates?.[selectedCard.deviceStateKey]
              : undefined
          }
        />
      </SnackbarProvider>
    </CustomTheme>
  );
};

export default Dashboard;
