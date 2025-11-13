import React, { FC, RefObject, useEffect, useState } from 'react';
import '../App.css';
import { SnackbarProvider, useSnackbar } from 'notistack';
import { IconButton, Box, Alert } from '@mui/material';
import CloseIcon from '@mui/icons-material/Close';
import SensorCard from '../components/SensorCard';
import CustomTheme from '../CustomTheme';
import SensorService from '../api/SensorService';
import { fetchThresholds, getDeviceStates } from '../api/ControllerService';
import { SensorValues } from '../types/sensors';

const Dashboard: FC = () => {
  const { enqueueSnackbar } = useSnackbar();
  const [sensorData, setSensorData] = useState<SensorValues>({});
  const [thresholds, setThresholds] = useState<any | null>(null);
  const [deviceStates, setDeviceStates] = useState<any | null>(null);
  const [loading, setLoading] = useState<boolean>(true);

  // Data for charts
  const [waterTempData, setWaterTempData] = useState<number[]>([]);
  const [airTempData, setAirTempData] = useState<number[]>([]);
  const [humidityData, setHumidityData] = useState<number[]>([]);
  const [moistureData, setMoistureData] = useState<number[]>([]);
  const [TDSData, setTDSData] = useState<number[]>([]);
  const [CO2Data, setCO2Data] = useState<number[]>([]);
  const [TVOCData, setTVOCData] = useState<number[]>([]);
  const [phData, setphData] = useState<number[]>([]);
  const [airQualityData, setAirQualityData] = useState<number[]>([]);

  useEffect(() => {
    const fetchData = async () => {
      try {
        const [sensorResponse, thresholdsResponse, devicesResponse] = await Promise.all([
          SensorService.getSensorData(),
          fetchThresholds(),
          getDeviceStates(),
        ]);

        const values = sensorResponse?.values ?? {};
        setSensorData(values);
        setThresholds(thresholdsResponse);
        setDeviceStates(devicesResponse);

        const numeric = (key: string) => {
          const value = values[key];
          if (typeof value === 'number') {
            return value;
          }
          if (typeof value === 'string') {
            const parsed = parseFloat(value);
            return Number.isFinite(parsed) ? parsed : 0;
          }
          return 0;
        };

        if (values) {
          setAirTempData((prev) => [...prev.slice(-19), numeric('airtemp')]);
          setHumidityData((prev) => [...prev.slice(-19), numeric('humidity')]);
          setMoistureData((prev) => [...prev.slice(-19), numeric('moisture')]);
          setTDSData((prev) => [...prev.slice(-19), numeric('tdsSens')]);
          setCO2Data((prev) => [...prev.slice(-19), numeric('CO2')]);
          setTVOCData((prev) => [...prev.slice(-19), numeric('TVOC')]);
          setWaterTempData((prev) => [...prev.slice(-19), numeric('temperature')]);
          setphData((prev) => [...prev.slice(-19), numeric('ph')]);
          setAirQualityData((prev) => [...prev.slice(-19), numeric('airquality')]);
        }
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
    notistackRef.current.closeSnackbar(key);
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
          {/* Sensor Cards */}
          <SensorCard
            title="Air Temperature"
            value={sensorData?.airtemp ?? null}
            unit="°C"
            loading={loading}
            chartType="sparkline"
            chartData={airTempData}
            minThreshold={thresholds?.minTemp}
            maxThreshold={thresholds?.maxTemp}
            deviceStatus={deviceStates?.extractorFanState ?? false}
          />

          <SensorCard
            title="Water Temperature"
            value={sensorData?.temperature ?? null}
            unit="°C"
            loading={loading}
            chartType="sparkline"
            chartData={waterTempData}
            minThreshold={thresholds?.waterTemp?.min}
            maxThreshold={thresholds?.waterTemp?.max}
            deviceStatus={deviceStates?.extractorFanState ?? false}
          />

          <SensorCard
            title="Air Humidity"
            value={sensorData?.humidity ?? null}
            unit="%"
            loading={loading}
            chartType="sparkline"
            chartData={humidityData}
            minThreshold={thresholds?.humidity?.min}
            maxThreshold={thresholds?.humidity?.max}
            deviceStatus={deviceStates?.extractorFanState ?? false}
          />

          <SensorCard
            title="Soil Moisture"
            value={sensorData?.moisture ?? sensorData?.soilMoisture ?? null}
            unit="%"
            loading={loading}
            chartType="sparkline"
            chartData={moistureData}
            minThreshold={thresholds?.minMoisture}
            maxThreshold={thresholds?.maxMoisture}
            deviceStatus={deviceStates?.moistureDevice ?? false}
          />

          <SensorCard
            title="Water Particulates"
            value={sensorData?.tdsSens ?? null}
            unit="%"
            loading={loading}
            chartType="sparkline"
            chartData={TDSData}
            minThreshold={thresholds?.tdsSens?.min}
            maxThreshold={thresholds?.tdsSens?.max}
            deviceStatus={deviceStates?.tdsSensDevice ?? false}
          />

          <SensorCard
            title="CO2 Levels"
            value={sensorData?.CO2 ?? null}
            unit="ppm"
            loading={loading}
            chartType="sparkline"
            chartData={CO2Data}
            minThreshold={thresholds?.minCO2}
            maxThreshold={thresholds?.maxCO2}
            deviceStatus={deviceStates?.intakeFanState ?? false}
          />

          <SensorCard
            title="TVOC Levels"
            value={sensorData?.TVOC ?? null}
            unit="ppb"
            loading={loading}
            chartType="sparkline"
            chartData={TVOCData}
            minThreshold={thresholds?.TVOC?.min}
            maxThreshold={thresholds?.TVOC?.max}
            deviceStatus={deviceStates?.intakeFanState ?? false}
          />

          <SensorCard
            title="Air Quality Index"
            value={sensorData?.airquality ?? null}
            unit=""
            loading={loading}
            chartType="sparkline"
            chartData={airQualityData}
            minThreshold={thresholds?.airquality?.min}
            maxThreshold={thresholds?.airquality?.max}
            deviceStatus={deviceStates?.intakeFanState ?? false}
          />

          <SensorCard
            title="Soil PH"
            value={sensorData?.soilph ?? null}
            unit="pH"
            loading={loading}
            chartType="sparkline"
            chartData={phData}
            minThreshold={thresholds?.soilph?.min}
            maxThreshold={thresholds?.soilph?.max}
            deviceStatus={deviceStates?.phDevice ?? false}
          />
          <SensorCard
            title="Nitrogen"
            value={sensorData?.nitro ?? null}
            unit="mg/kg"
            loading={loading}
            chartType="sparkline"
            chartData={phData}
            minThreshold={thresholds?.nitro?.min}
            maxThreshold={thresholds?.nitro?.max}
            deviceStatus={deviceStates?.phDevice ?? false}
          />
          <SensorCard
            title="Phosphorus"
            value={sensorData?.phos ?? null}
            unit="mg/kg"
            loading={loading}
            chartType="sparkline"
            chartData={phData}
            minThreshold={thresholds?.phos?.min}
            maxThreshold={thresholds?.phos?.max}
            deviceStatus={deviceStates?.phDevice ?? false}
          />
            <SensorCard
            title="Potassium"
            value={sensorData?.potas ?? null}
            unit="mg/kg"
            loading={loading}
            chartType="sparkline"
            chartData={phData}
            minThreshold={thresholds?.potas?.min}
            maxThreshold={thresholds?.potas?.max}
            deviceStatus={deviceStates?.phDevice ?? false}
          />

          <SensorCard
            title="PAR"
            value={sensorData?.total ?? null}
            unit="µmol/m²/s"
            loading={loading}
            chartType="pie"
            chartData={{
              Blue:sensorData?.blue,
              Green: sensorData?.green,
              Red: sensorData?.red,
              FarRed: sensorData?.FarRed,
            }}
            deviceStatus={deviceStates?.lights ?? false}
          />

          <SensorCard
            title="Plant Health (NDVI)"
            value={sensorData?.ndvi ?? null}
            unit=""
            loading={loading}
            chartType="circular"
            progressValue={sensorData?.ndvi ? sensorData.ndvi * 100 : 0}
            healthColor={(ndvi?) => {
              if (ndvi == null) return 'gray';
              if (ndvi < 20) return 'red';
              if (ndvi < 40) return 'orange';
              if (ndvi < 60) return 'yellow';
              if (ndvi < 80) return 'lightgreen';
              return 'green';
            }}
            minThreshold={thresholds?.airquality?.min}
            maxThreshold={thresholds?.airquality?.max}
            deviceStatus={deviceStates?.lights ?? false}
          />
          <SensorCard
            title="Greenlight Intensity"
            value={sensorData?.greenIntensity ?? null}
            unit="lux"
            loading={loading}
            chartType="circular"
            progressValue={sensorData?.greenIntensity}
            minThreshold={thresholds?.greenlight?.min}
            maxThreshold={thresholds?.greenlight?.max}
            deviceStatus={deviceStates?.greenlightDevice ?? false}
          />
        </Box>
      </SnackbarProvider>
    </CustomTheme>
  );
};

export default Dashboard;
