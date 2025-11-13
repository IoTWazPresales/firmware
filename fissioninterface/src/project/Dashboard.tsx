import React, { FC, RefObject, useEffect, useState } from 'react';
import '../App.css';
import { SnackbarProvider, useSnackbar } from 'notistack';
import { IconButton, Box } from '@mui/material';
import CloseIcon from '@mui/icons-material/Close';
import SensorCard from '../components/SensorCard';
import CustomTheme from '../CustomTheme';
import SensorService from '../api/SensorService';
import { fetchThresholds, getDeviceStates, Thresholds, DeviceStates } from '../api/ControllerService';
import { SensorCapabilityMeta, SensorResponse, SensorValues } from '../types/sensors';

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
    thresholdSelector: (thr) => ({ min: thr?.minTemp ?? null, max: thr?.maxTemp ?? null }),
  },
  {
    id: 'temperature',
    title: 'Water Temperature',
    unit: '°C',
    chartType: 'sparkline',
    deviceStateKey: 'extractorFanState',
    thresholdSelector: (thr) => ({ min: thr?.waterTemp?.min ?? null, max: thr?.waterTemp?.max ?? null }),
  },
  {
    id: 'humidity',
    title: 'Air Humidity',
    unit: '%',
    chartType: 'sparkline',
    deviceStateKey: 'extractorFanState',
    thresholdSelector: (thr) => ({ min: thr?.humidity?.min ?? null, max: thr?.humidity?.max ?? null }),
  },
  {
    id: 'soilMoisture',
    title: 'Soil Moisture',
    unit: '%',
    chartType: 'sparkline',
    deviceStateKey: 'pumpState',
    historyKey: 'soilMoisture',
    thresholdSelector: (thr) => ({ min: thr?.minMoisture ?? null, max: thr?.maxMoisture ?? null }),
  },
  {
    id: 'tdsSens',
    title: 'Water Particulates',
    unit: '%',
    chartType: 'sparkline',
    deviceStateKey: 'pumpState',
    thresholdSelector: (thr) => ({ min: thr?.tdsSens?.min ?? null, max: thr?.tdsSens?.max ?? null }),
  },
  {
    id: 'CO2',
    title: 'CO₂ Levels',
    unit: 'ppm',
    chartType: 'sparkline',
    deviceStateKey: 'intakeFanState',
    thresholdSelector: (thr) => ({ min: thr?.minCO2 ?? null, max: thr?.maxCO2 ?? null }),
  },
  {
    id: 'TVOC',
    title: 'TVOC Levels',
    unit: 'ppb',
    chartType: 'sparkline',
    deviceStateKey: 'intakeFanState',
    thresholdSelector: (thr) => ({ min: thr?.TVOC?.min ?? null, max: thr?.TVOC?.max ?? null }),
  },
  {
    id: 'airquality',
    title: 'Air Quality Index',
    unit: '',
    chartType: 'sparkline',
    deviceStateKey: 'intakeFanState',
    thresholdSelector: (thr) => ({ min: thr?.airquality?.min ?? null, max: thr?.airquality?.max ?? null }),
  },
  {
    id: 'soilph',
    title: 'Soil PH',
    unit: 'pH',
    chartType: 'sparkline',
    deviceStateKey: 'pumpState',
    thresholdSelector: (thr) => ({ min: thr?.soilph?.min ?? null, max: thr?.soilph?.max ?? null }),
  },
  {
    id: 'nitro',
    title: 'Nitrogen',
    unit: 'mg/kg',
    chartType: 'sparkline',
    deviceStateKey: 'pumpState',
    thresholdSelector: (thr) => ({ min: thr?.nitro?.min ?? null, max: thr?.nitro?.max ?? null }),
  },
  {
    id: 'phos',
    title: 'Phosphorus',
    unit: 'mg/kg',
    chartType: 'sparkline',
    deviceStateKey: 'pumpState',
    thresholdSelector: (thr) => ({ min: thr?.phos?.min ?? null, max: thr?.phos?.max ?? null }),
  },
  {
    id: 'potas',
    title: 'Potassium',
    unit: 'mg/kg',
    chartType: 'sparkline',
    deviceStateKey: 'pumpState',
    thresholdSelector: (thr) => ({ min: thr?.potas?.min ?? null, max: thr?.potas?.max ?? null }),
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
    thresholdSelector: (thr) => ({ min: thr?.greenlight?.min ?? null, max: thr?.greenlight?.max ?? null }),
  },
];

const Dashboard: FC = () => {
  const { enqueueSnackbar } = useSnackbar();
  const [sensorData, setSensorData] = useState<SensorValues>({});
  const [sensorMeta, setSensorMeta] = useState<SensorCapabilityMeta[]>([]);
  const [thresholds, setThresholds] = useState<Thresholds | null>(null);
  const [deviceStates, setDeviceStates] = useState<DeviceStates | null>(null);
  const [cardConfigs, setCardConfigs] = useState<DashboardCardConfig[]>(CARD_DEFINITIONS);
  const [sensorHistory, setSensorHistory] = useState<Record<string, number[]>>({});
  const [loading, setLoading] = useState<boolean>(true);

  useEffect(() => {
    const fetchData = async () => {
      try {
        const [sensorResponse, thresholdsResponse, devicesResponse] = await Promise.all<[
          SensorResponse,
          Thresholds,
          DeviceStates
        ]>([
          SensorService.getSensorData(),
          fetchThresholds(),
          getDeviceStates(),
        ]);

        const values = sensorResponse?.values ?? {};
        const metaList = sensorResponse?.meta ?? [];
        setSensorData(values);
        setSensorMeta(metaList);
        setThresholds(thresholdsResponse ?? null);
        setDeviceStates(devicesResponse ?? null);

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
    const interval = setInterval(fetchData, 5000);
    return () => clearInterval(interval);
  }, [enqueueSnackbar]);

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
        <Box
          sx={{
            display: 'grid',
            gridTemplateColumns: 'repeat(auto-fit, minmax(250px, 1fr))',
            gap: 2,
            padding: 1,
          }}
        >
          {cardConfigs.map(renderCard)}
        </Box>
      </SnackbarProvider>
    </CustomTheme>
  );
};

export default Dashboard;
