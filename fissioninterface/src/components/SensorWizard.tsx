import React, { FC, useState } from 'react';
import {
  Dialog,
  DialogTitle,
  DialogContent,
  DialogActions,
  Button,
  Stepper,
  Step,
  StepLabel,
  Box,
  Typography,
  TextField,
  Select,
  MenuItem,
  FormControl,
  InputLabel,
  RadioGroup,
  FormControlLabel,
  Radio,
  Chip,
  Alert,
} from '@mui/material';
import { ScanResults, DigitalPin, AnalogPin } from '../api/DeviceScannerService';

interface SensorWizardProps {
  open: boolean;
  onClose: () => void;
  onComplete: (assignment: { port: string; sensorType: string; pin?: number }) => void;
  scanResults: ScanResults;
}

const steps = ['Select Interface', 'Choose Port', 'Select Sensor', 'Confirm'];

const SensorWizard: FC<SensorWizardProps> = ({ open, onClose, onComplete, scanResults }) => {
  const [activeStep, setActiveStep] = useState(0);
  const [interfaceType, setInterfaceType] = useState<'analog' | 'digital' | 'i2c' | 'wireless' | ''>('');
  const [selectedPort, setSelectedPort] = useState<string>('');
  const [selectedSensor, setSelectedSensor] = useState<string>('');
  const [sensorName, setSensorName] = useState<string>('');

  const SENSOR_TYPES = [
    { value: 'airTempHumidity', label: 'DHT11 (Air Temp & Humidity)' },
    { value: 'soilMoisture', label: 'Soil Moisture' },
    { value: 'ph', label: 'pH Sensor' },
    { value: 'tds', label: 'TDS Sensor' },
    { value: 'temperature', label: 'DS18B20 Temperature' },
    { value: 'npk', label: 'NPK Sensor' },
    { value: 'airQuality', label: 'ENS160 Air Quality' },
    { value: 'spectral', label: 'AS7341 Spectral' },
    { value: 'rtc', label: 'DS3231 RTC' },
    { value: 'unknown', label: 'Unknown Sensor (Submit for Integration)' },
  ];

  const handleNext = () => {
    if (activeStep === steps.length - 1) {
      handleComplete();
    } else {
      setActiveStep((prev) => prev + 1);
    }
  };

  const handleBack = () => {
    setActiveStep((prev) => prev - 1);
  };

  const handleComplete = () => {
    onComplete({
      port: selectedPort,
      sensorType: selectedSensor,
    });
    handleReset();
    onClose();
  };

  const handleReset = () => {
    setActiveStep(0);
    setInterfaceType('');
    setSelectedPort('');
    setSelectedSensor('');
    setSensorName('');
  };

  const handleClose = () => {
    handleReset();
    onClose();
  };

  const getAvailablePorts = () => {
    if (interfaceType === 'analog') {
      return scanResults.analogPins.filter((p: AnalogPin) => p.detected);
    } else if (interfaceType === 'digital') {
      return scanResults.digitalPins.filter((p: DigitalPin) => p.detected);
    } else if (interfaceType === 'i2c') {
      return scanResults.i2cDevices.map((addr, idx) => ({ key: `J${idx + 1}`, address: addr }));
    }
    return [];
  };

  const canProceed = () => {
    switch (activeStep) {
      case 0:
        return interfaceType !== '';
      case 1:
        return selectedPort !== '';
      case 2:
        return selectedSensor !== '';
      case 3:
        return true;
      default:
        return false;
    }
  };

  const renderStepContent = () => {
    switch (activeStep) {
      case 0:
        return (
          <Box sx={{ mt: 2 }}>
            <Typography variant="body1" gutterBottom>
              How is your sensor connected?
            </Typography>
            <FormControl component="fieldset" sx={{ mt: 2, width: '100%' }}>
              <RadioGroup
                value={interfaceType}
                onChange={(e) => setInterfaceType(e.target.value as any)}
              >
                <FormControlLabel
                  value="analog"
                  control={<Radio />}
                  label={
                    <Box>
                      <Typography variant="body1">Analog Port</Typography>
                      <Typography variant="caption" color="text.secondary">
                        For analog sensors (voltage-based)
                      </Typography>
                    </Box>
                  }
                />
                <FormControlLabel
                  value="digital"
                  control={<Radio />}
                  label={
                    <Box>
                      <Typography variant="body1">Digital Port</Typography>
                      <Typography variant="caption" color="text.secondary">
                        For digital sensors (on/off, PWM)
                      </Typography>
                    </Box>
                  }
                />
                <FormControlLabel
                  value="i2c"
                  control={<Radio />}
                  label={
                    <Box>
                      <Typography variant="body1">I²C Bus</Typography>
                      <Typography variant="caption" color="text.secondary">
                        For I²C sensors (multiple devices on one bus)
                      </Typography>
                    </Box>
                  }
                />
                <FormControlLabel
                  value="wireless"
                  control={<Radio />}
                  label={
                    <Box>
                      <Typography variant="body1">Wireless (BLE/LoRa/Sigfox)</Typography>
                      <Typography variant="caption" color="text.secondary">
                        For wireless sensors (coming soon)
                      </Typography>
                    </Box>
                  }
                  disabled
                />
              </RadioGroup>
            </FormControl>
            {interfaceType === 'wireless' && (
              <Alert severity="info" sx={{ mt: 2 }}>
                Wireless sensor support is coming in a future update.
              </Alert>
            )}
          </Box>
        );

      case 1:
        const ports = getAvailablePorts();
        return (
          <Box sx={{ mt: 2 }}>
            <Typography variant="body1" gutterBottom>
              Select the port where your sensor is connected:
            </Typography>
            {ports.length === 0 ? (
              <Alert severity="warning" sx={{ mt: 2 }}>
                No {interfaceType} devices detected. Make sure your sensor is connected and click "Scan Now" on the Devices page.
              </Alert>
            ) : (
              <FormControl fullWidth sx={{ mt: 2 }}>
                <InputLabel>Port</InputLabel>
                <Select
                  value={selectedPort}
                  onChange={(e) => setSelectedPort(e.target.value)}
                  label="Port"
                >
                  {ports.map((port: any, idx: number) => (
                    <MenuItem key={idx} value={interfaceType === 'i2c' ? port.key : port.key}>
                      {interfaceType === 'i2c' ? (
                        <Box>
                          <Typography variant="body1">{port.key}</Typography>
                          <Typography variant="caption" color="text.secondary">
                            Address: {port.address}
                          </Typography>
                        </Box>
                      ) : (
                        <Box>
                          <Typography variant="body1">{port.key}</Typography>
                          <Typography variant="caption" color="text.secondary">
                            Pin: {port.pin}
                          </Typography>
                        </Box>
                      )}
                    </MenuItem>
                  ))}
                </Select>
              </FormControl>
            )}
          </Box>
        );

      case 2:
        return (
          <Box sx={{ mt: 2 }}>
            <Typography variant="body1" gutterBottom>
              What type of sensor is this?
            </Typography>
            <FormControl fullWidth sx={{ mt: 2 }}>
              <InputLabel>Sensor Type</InputLabel>
              <Select
                value={selectedSensor}
                onChange={(e) => setSelectedSensor(e.target.value)}
                label="Sensor Type"
              >
                {SENSOR_TYPES.map((sensor) => (
                  <MenuItem key={sensor.value} value={sensor.value}>
                    {sensor.label}
                  </MenuItem>
                ))}
              </Select>
            </FormControl>
            {selectedSensor === 'unknown' && (
              <Alert severity="info" sx={{ mt: 2 }}>
                <Typography variant="body2">
                  We'll help you submit this sensor for integration. You'll be able to provide details in the next step.
                </Typography>
              </Alert>
            )}
          </Box>
        );

      case 3:
        return (
          <Box sx={{ mt: 2 }}>
            <Typography variant="h6" gutterBottom>
              Review Your Configuration
            </Typography>
            <Box sx={{ mt: 2, p: 2, bgcolor: 'background.paper', borderRadius: 1 }}>
              <Typography variant="body2" color="text.secondary">Interface</Typography>
              <Typography variant="body1" sx={{ mb: 2, textTransform: 'capitalize' }}>
                {interfaceType}
              </Typography>

              <Typography variant="body2" color="text.secondary">Port</Typography>
              <Typography variant="body1" sx={{ mb: 2 }}>
                {selectedPort}
              </Typography>

              <Typography variant="body2" color="text.secondary">Sensor Type</Typography>
              <Typography variant="body1">
                {SENSOR_TYPES.find((s) => s.value === selectedSensor)?.label || selectedSensor}
              </Typography>
            </Box>
            {selectedSensor === 'unknown' && (
              <TextField
                fullWidth
                label="Sensor Name (Optional)"
                value={sensorName}
                onChange={(e) => setSensorName(e.target.value)}
                sx={{ mt: 2 }}
                placeholder="e.g., Custom Soil Sensor"
              />
            )}
          </Box>
        );

      default:
        return null;
    }
  };

  return (
    <Dialog open={open} onClose={handleClose} maxWidth="sm" fullWidth>
      <DialogTitle>Add New Sensor</DialogTitle>
      <DialogContent>
        <Stepper activeStep={activeStep} sx={{ mt: 2, mb: 4 }}>
          {steps.map((label) => (
            <Step key={label}>
              <StepLabel>{label}</StepLabel>
            </Step>
          ))}
        </Stepper>
        {renderStepContent()}
      </DialogContent>
      <DialogActions>
        <Button onClick={handleClose}>Cancel</Button>
        <Button onClick={handleBack} disabled={activeStep === 0}>
          Back
        </Button>
        <Button onClick={handleNext} variant="contained" disabled={!canProceed()}>
          {activeStep === steps.length - 1 ? 'Add Sensor' : 'Next'}
        </Button>
      </DialogActions>
    </Dialog>
  );
};

export default SensorWizard;

