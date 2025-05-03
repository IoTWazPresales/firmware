import React, { FC, RefObject, useEffect, useState } from 'react';
import {
  SnackbarProvider,
  useSnackbar,
} from 'notistack';
import {
  Table,
  TableBody,
  TableCell,
  TableContainer,
  TableHead,
  TableRow,
  Paper,
  IconButton,
  Slider,
  Button,
  Box,
  Typography,
  Chip,
} from '@mui/material';
import CloseIcon from '@mui/icons-material/Close';
import CustomTheme from '../CustomTheme';
import ControllerService, { ControllerData } from '../api/ControllerService';

const ControlPanel: FC = () => {
  const { enqueueSnackbar } = useSnackbar();

  const [controllerData, setControllerData] = useState<ControllerData | null>(
    null
  );
  const [loading, setLoading] = useState<boolean>(true);
  const [initialLoad, setInitialLoad] = useState<boolean>(true);

  const [thresholds, setThresholds] = useState<Record<
    string,
    { min: number; max: number; unit: string }
  >>({
    waterPump: { min: 30, max: 70, unit: '%' },
    extractorFan: { min: 30, max: 70, unit: '°C' },
    intakeFan: { min: 200, max: 3000, unit: 'ppm' },
    lights: { min: 30, max: 70, unit: '' },
  });

  useEffect(() => {
    const fetchControlData = async () => {
      try {
        const thresholdsData = await ControllerService.getThresholds();

        if (initialLoad) {
          setThresholds({
            waterPump: { min: thresholdsData.minMoisture, max: thresholdsData.maxMoisture, unit: '%' },
            extractorFan: { min: thresholdsData.minTemp, max: thresholdsData.maxTemp, unit: '°C' },
            intakeFan: { min: thresholdsData.minCO2, max: thresholdsData.maxCO2, unit: 'ppm' },
            lights: { min: thresholdsData.minTemp, max: thresholdsData.maxTemp, unit: '' },
          });

          setInitialLoad(false);
        }

        const deviceData = await ControllerService.getDeviceStates();
        setControllerData(deviceData);
      } catch (error) {
        console.error('Failed to fetch controller data:', error);
        enqueueSnackbar('Failed to fetch controller data.', { variant: 'error' });
      } finally {
        setLoading(false);
      }
    };

    fetchControlData();
    const interval = setInterval(fetchControlData, 5000);

    return () => clearInterval(interval);
  }, [enqueueSnackbar, initialLoad]);

  const handleThresholdChange = (
    device: keyof typeof thresholds,
    min: number,
    max: number
  ) => {
    setThresholds((prev) => ({
      ...prev,
      [device]: { ...prev[device], min, max },
    }));
  };

  const handleSaveThresholds = async () => {
    try {
      await ControllerService.setThresholds(
        thresholds.waterPump.min,
        thresholds.waterPump.max,
        thresholds.intakeFan.min,
        thresholds.intakeFan.max,
        thresholds.extractorFan.min,
        thresholds.extractorFan.max,
        thresholds.lights.min,
        thresholds.lights.max
      );
      enqueueSnackbar('Thresholds set successfully!', { variant: 'success' });
    } catch (error) {
      console.error('Failed to set thresholds:', error);
      enqueueSnackbar('Failed to set thresholds.', { variant: 'error' });
    }
  };

  const notistackRef: RefObject<any> = React.createRef();
  const onClickDismiss = (key: string | number | undefined) => () => {
    notistackRef.current.closeSnackbar(key);
  };

  return (
    <CustomTheme>
      <SnackbarProvider
        maxSnack={3}
        anchorOrigin={{ vertical: 'bottom', horizontal: 'left' }}
        ref={notistackRef}
        action={(key) => (
          <IconButton onClick={onClickDismiss(key)} size="small">
            <CloseIcon />
          </IconButton>
        )}
      >
        <TableContainer component={Paper} sx={{ margin: 2, boxShadow: 3 }}>
          <Table>
            <TableHead>
              <TableRow sx={{ backgroundColor: '#1976d2' }}>
                <TableCell sx={{ color: 'white', fontWeight: 'bold' }}>Device Name</TableCell>
                <TableCell sx={{ color: 'white', fontWeight: 'bold' }}>Last Seen</TableCell>
                <TableCell sx={{ color: 'white', fontWeight: 'bold' }}>Last On</TableCell>
                <TableCell sx={{ color: 'white', fontWeight: 'bold' }}>Current Status</TableCell>
                <TableCell sx={{ color: 'white', fontWeight: 'bold' }}>Monitored Parameter</TableCell>
                <TableCell sx={{ color: 'white', fontWeight: 'bold' }}>Threshold Settings</TableCell>
                <TableCell sx={{ color: 'white', fontWeight: 'bold' }}>Actions</TableCell>
              </TableRow>
            </TableHead>
            <TableBody>
              {[
                { id: 'waterPump', name: 'Water Pump', status: controllerData?.pumpState, monitored: 'Soil Moisture' },
                { id: 'extractorFan', name: 'Extractor Fan', status: controllerData?.extractorFanState, monitored: 'Temperature' },
                { id: 'intakeFan', name: 'Intake Fan', status: controllerData?.intakeFanState, monitored: 'CO2 Levels' },
                { id: 'lights', name: 'Lights', status: controllerData?.lightsState, monitored: 'Temperature' },
              ].map((device) => (
                <TableRow key={device.id}>
                  <TableCell>{device.name}</TableCell>
                  <TableCell>Last Seen Timestamp</TableCell>
                  <TableCell>Last On Timestamp</TableCell>
                  <TableCell>
                    <Chip
                      label={device.status ? 'On' : 'Off'}
                      color={device.status ? 'success' : 'error'}
                      sx={{
                        fontWeight: 'bold',
                        fontSize: '0.9rem',
                        padding: '5px',
                      }}
                    />
                  </TableCell>
                  <TableCell>{device.monitored}</TableCell>
                  <TableCell>
                    <Slider
                      value={[
                        thresholds[device.id as keyof typeof thresholds].min,
                        thresholds[device.id as keyof typeof thresholds].max,
                      ]}
                      onChange={(_, newValue) => {
                        if (Array.isArray(newValue)) {
                          handleThresholdChange(
                            device.id as keyof typeof thresholds,
                            newValue[0],
                            newValue[1]
                          );
                        }
                      }}
                      valueLabelDisplay="auto"
                      min={0}
                      max={device.id === 'intakeFan' ? 5000 : 100} // Adjust range for CO2 PPM
                    />
                    <Box display="flex" justifyContent="space-between">
                      <Typography variant="body2">
                        Min: {thresholds[device.id as keyof typeof thresholds].min}
                        {thresholds[device.id as keyof typeof thresholds].unit}
                      </Typography>
                      <Typography variant="body2">
                        Max: {thresholds[device.id as keyof typeof thresholds].max}
                        {thresholds[device.id as keyof typeof thresholds].unit}
                      </Typography>
                    </Box>
                  </TableCell>
                  <TableCell>
                    <Button
                      variant="contained"
                      color="primary"
                      onClick={handleSaveThresholds}
                      sx={{ fontWeight: 'bold' }}
                    >
                      Save
                    </Button>
                  </TableCell>
                </TableRow>
              ))}
            </TableBody>
          </Table>
        </TableContainer>
      </SnackbarProvider>
    </CustomTheme>
  );
};

export default ControlPanel;
