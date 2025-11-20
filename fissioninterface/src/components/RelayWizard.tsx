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
  Alert,
  Slider,
} from '@mui/material';
import { SensorParameterOption } from '../api/ControllerService';

interface RelayWizardProps {
  open: boolean;
  onClose: () => void;
  onComplete: (config: {
    pin: string;
    deviceType: string;
    parameter: string;
    min: number;
    max: number;
  }) => void;
  availablePins: string[];
  parameters: SensorParameterOption[];
}

const steps = ['Select Pin', 'Choose Device', 'Set Parameter', 'Configure Thresholds', 'Confirm'];

const DEVICE_TYPES = [
  { value: 'waterPump', label: 'Water Pump', icon: '💧' },
  { value: 'extractorFan', label: 'Extractor Fan', icon: '🌀' },
  { value: 'intakeFan', label: 'Intake Fan', icon: '💨' },
  { value: 'lights', label: 'Grow Lights', icon: '💡' },
];

const RelayWizard: FC<RelayWizardProps> = ({
  open,
  onClose,
  onComplete,
  availablePins,
  parameters,
}) => {
  const [activeStep, setActiveStep] = useState(0);
  const [selectedPin, setSelectedPin] = useState<string>('');
  const [deviceType, setDeviceType] = useState<string>('');
  const [parameter, setParameter] = useState<string>('');
  const [minThreshold, setMinThreshold] = useState<number>(0);
  const [maxThreshold, setMaxThreshold] = useState<number>(100);

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
      pin: selectedPin,
      deviceType,
      parameter,
      min: minThreshold,
      max: maxThreshold,
    });
    handleReset();
    onClose();
  };

  const handleReset = () => {
    setActiveStep(0);
    setSelectedPin('');
    setDeviceType('');
    setParameter('');
    setMinThreshold(0);
    setMaxThreshold(100);
  };

  const handleClose = () => {
    handleReset();
    onClose();
  };

  const canProceed = () => {
    switch (activeStep) {
      case 0:
        return selectedPin !== '';
      case 1:
        return deviceType !== '';
      case 2:
        return parameter !== '';
      case 3:
        return minThreshold < maxThreshold;
      case 4:
        return true;
      default:
        return false;
    }
  };

  const getParameterUnit = () => {
    const param = parameters.find((p) => p.id === parameter);
    return param?.unit || '';
  };

  const getParameterMax = () => {
    if (parameter.toUpperCase() === 'CO2') return 2000;
    if (parameter.toUpperCase() === 'PH') return 14;
    return 200;
  };

  const getParameterStep = () => {
    if (parameter.toUpperCase() === 'PH') return 0.1;
    if (parameter.toUpperCase() === 'CO2') return 10;
    return 1;
  };

  const renderStepContent = () => {
    switch (activeStep) {
      case 0:
        return (
          <Box sx={{ mt: 2 }}>
            <Typography variant="body1" gutterBottom>
              Which GPIO pin is your relay connected to?
            </Typography>
            <FormControl fullWidth sx={{ mt: 2 }}>
              <InputLabel>GPIO Pin</InputLabel>
              <Select
                value={selectedPin}
                onChange={(e) => setSelectedPin(e.target.value)}
                label="GPIO Pin"
              >
                {availablePins.map((pin) => (
                  <MenuItem key={pin} value={pin}>
                    Pin {pin}
                  </MenuItem>
                ))}
              </Select>
            </FormControl>
            <Alert severity="info" sx={{ mt: 2 }}>
              <Typography variant="body2">
                Available pins: {availablePins.join(', ')}
              </Typography>
            </Alert>
          </Box>
        );

      case 1:
        return (
          <Box sx={{ mt: 2 }}>
            <Typography variant="body1" gutterBottom>
              What device is controlled by this relay?
            </Typography>
            <FormControl component="fieldset" sx={{ mt: 2, width: '100%' }}>
              <RadioGroup
                value={deviceType}
                onChange={(e) => setDeviceType(e.target.value)}
              >
                {DEVICE_TYPES.map((device) => (
                  <FormControlLabel
                    key={device.value}
                    value={device.value}
                    control={<Radio />}
                    label={
                      <Box sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
                        <Typography variant="h6">{device.icon}</Typography>
                        <Typography variant="body1">{device.label}</Typography>
                      </Box>
                    }
                  />
                ))}
              </RadioGroup>
            </FormControl>
          </Box>
        );

      case 2:
        return (
          <Box sx={{ mt: 2 }}>
            <Typography variant="body1" gutterBottom>
              Which sensor parameter should control this device?
            </Typography>
            <FormControl fullWidth sx={{ mt: 2 }}>
              <InputLabel>Sensor Parameter</InputLabel>
              <Select
                value={parameter}
                onChange={(e) => {
                  setParameter(e.target.value);
                  // Reset thresholds when parameter changes
                  setMinThreshold(0);
                  setMaxThreshold(getParameterMax());
                }}
                label="Sensor Parameter"
              >
                {parameters.map((param) => (
                  <MenuItem key={param.id} value={param.id}>
                    {param.label} {param.unit ? `(${param.unit})` : ''}
                  </MenuItem>
                ))}
              </Select>
            </FormControl>
            {parameter && (
              <Alert severity="info" sx={{ mt: 2 }}>
                <Typography variant="body2">
                  The device will activate when the sensor value goes outside the threshold range you set next.
                </Typography>
              </Alert>
            )}
          </Box>
        );

      case 3:
        return (
          <Box sx={{ mt: 2 }}>
            <Typography variant="body1" gutterBottom>
              Set the activation thresholds for {parameters.find((p) => p.id === parameter)?.label}:
            </Typography>
            <Box sx={{ mt: 3, px: 2 }}>
              <Slider
                value={[minThreshold, maxThreshold]}
                onChange={(_, newValue) => {
                  if (Array.isArray(newValue)) {
                    setMinThreshold(newValue[0]);
                    setMaxThreshold(newValue[1]);
                  }
                }}
                valueLabelDisplay="auto"
                min={0}
                max={getParameterMax()}
                step={getParameterStep()}
                marks={[
                  { value: 0, label: '0' },
                  { value: getParameterMax(), label: String(getParameterMax()) },
                ]}
              />
              <Box sx={{ display: 'flex', justifyContent: 'space-between', mt: 2 }}>
                <TextField
                  label="Minimum"
                  type="number"
                  value={minThreshold}
                  onChange={(e) => setMinThreshold(Number(e.target.value))}
                  inputProps={{ min: 0, max: getParameterMax(), step: getParameterStep() }}
                  size="small"
                  sx={{ width: 120 }}
                />
                <TextField
                  label="Maximum"
                  type="number"
                  value={maxThreshold}
                  onChange={(e) => setMaxThreshold(Number(e.target.value))}
                  inputProps={{ min: 0, max: getParameterMax(), step: getParameterStep() }}
                  size="small"
                  sx={{ width: 120 }}
                />
              </Box>
              {minThreshold >= maxThreshold && (
                <Alert severity="error" sx={{ mt: 2 }}>
                  Minimum must be less than maximum
                </Alert>
              )}
              <Alert severity="info" sx={{ mt: 2 }}>
                <Typography variant="body2">
                  Device will activate when value is &lt; {minThreshold} or &gt; {maxThreshold} {getParameterUnit()}
                </Typography>
              </Alert>
            </Box>
          </Box>
        );

      case 4:
        const selectedDevice = DEVICE_TYPES.find((d) => d.value === deviceType);
        const selectedParam = parameters.find((p) => p.id === parameter);
        return (
          <Box sx={{ mt: 2 }}>
            <Typography variant="h6" gutterBottom>
              Review Your Configuration
            </Typography>
            <Box sx={{ mt: 2, p: 2, bgcolor: 'background.paper', borderRadius: 1 }}>
              <Typography variant="body2" color="text.secondary">GPIO Pin</Typography>
              <Typography variant="body1" sx={{ mb: 2 }}>
                Pin {selectedPin}
              </Typography>

              <Typography variant="body2" color="text.secondary">Device</Typography>
              <Typography variant="body1" sx={{ mb: 2 }}>
                {selectedDevice?.icon} {selectedDevice?.label}
              </Typography>

              <Typography variant="body2" color="text.secondary">Control Parameter</Typography>
              <Typography variant="body1" sx={{ mb: 2 }}>
                {selectedParam?.label} ({selectedParam?.unit})
              </Typography>

              <Typography variant="body2" color="text.secondary">Activation Thresholds</Typography>
              <Typography variant="body1">
                Activate when &lt; {minThreshold} or &gt; {maxThreshold} {getParameterUnit()}
              </Typography>
            </Box>
          </Box>
        );

      default:
        return null;
    }
  };

  return (
    <Dialog open={open} onClose={handleClose} maxWidth="sm" fullWidth>
      <DialogTitle>Add Relay Control</DialogTitle>
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
          {activeStep === steps.length - 1 ? 'Add Relay' : 'Next'}
        </Button>
      </DialogActions>
    </Dialog>
  );
};

export default RelayWizard;

