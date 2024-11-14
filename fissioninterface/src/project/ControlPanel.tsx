import React, { FC, RefObject, useEffect ,useState } from 'react';
import logo from './logo.svg';
import '../App.css';
import { SnackbarProvider, useSnackbar } from 'notistack';
import { BrowserRouter as Router } from 'react-router-dom'; // Import BrowserRouter
import { RouterTabs, useLayoutTitle, useRouterTab } from '../components';
import { Tune, Settings } from '@mui/icons-material';
import Error from '@mui/icons-material/Error';
import { ControlPoint } from '@mui/icons-material';
import { IconButton, Button, Typography,
  Paper,
  Stack,
  Alert,
  Card,
  CardContent,
  Box,
  Slider,  
  CircularProgress,
  CardActions,
  Switch  } from '@mui/material';
  import { useWs } from '../utils/useWs';
  import { updateValue} from '../utils/binding';
  
import { WEB_SOCKET_ROOT } from '../api/endpoints';
import CloseIcon from '@mui/icons-material/Close';
import CustomTheme from '../CustomTheme';
import SectionContent from '../SectionContent';
import { Layout } from '../components/layout'; 
import SensorService, {SensorData} from '../api/SensorService';
import ControllerService, {ControllerData} from '../api/ControllerService';
import SensorCard from '../components/CustomCard';


interface SensorDataState {
  [key: string]: number | null;
}


const dataMoisture = [
  65, 65, 64.5, 64, 63, 62, 60, 57, 54, 50, 45, 41, 38, 36, 34, 31, 28, 24, 19
];

const dataLights = [
  1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1
];

const dataAirPump = [
  1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
];

const dataFanRelay = [
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
];

const dataPowerConsumation = [
  534, 521, 540, 240, 220, 235, 238, 228
];

const ControlPanel: FC = () => {
  const { enqueueSnackbar } = useSnackbar(); // This gives access to enqueueSnackbar function

  useLayoutTitle("Control Panel");
  const [sensorData, setSensorData] = useState<SensorData | null>(null);
  const [controllerData, setControllerData] = useState<ControllerData | null>(null);
    const [loading, setLoading] = useState<boolean>(true);
    const [error, setError] = useState<string | null>(null);
    const [pumpState, setPumpState] = useState<boolean>(true);  // Declare pumpState here

    const [minMoisture, setMinMoisture] = useState(30); // default values
const [maxMoisture, setMaxMoisture] = useState(70);



const handleMinMoistureChange = (event: Event, newValue: number | number[]) => {
  const minValue = Array.isArray(newValue) ? newValue[0] : newValue;
  setMinMoisture(minValue);
};

const handleMaxMoistureChange = (event: Event, newValue: number | number[]) => {
  const maxValue = Array.isArray(newValue) ? newValue[0] : newValue;
  setMaxMoisture(maxValue);
};
const handleSetMoistureThresholds = async () => {
  try {
      // Call the service to set the moisture thresholds
      await ControllerService.setMoistureThresholds(minMoisture, maxMoisture);
      console.log('Moisture thresholds set successfully');
      enqueueSnackbar('Moisture thresholds set successfully!', { variant: 'success' });

  } catch (error) {
    console.error('Failed to set moisture threshold:', error);
    enqueueSnackbar('Failed to set moisture change', { variant: 'error' });
  }
};

    useEffect(() => {
     
        const fetchSensorData = async () => {
             try {
                const data = await SensorService.getSensorData();
                setSensorData(data);
               
            } catch (err) {
                setError('Failed to fetch sensor data.');
            } finally {
                setLoading(false);
            }
        };
        const fetchControllerData = async () => {
          try {
             const data = await ControllerService.getControllerData();
             setControllerData(data);
         } catch (err) {
             setError('Failed to fetch sensor data.');
         } finally {
             setLoading(false);
         }
     };
        
     const sensorInterval = setInterval(fetchSensorData, 5000);
     const controllerInterval = setInterval(fetchControllerData, 5000);
 
     // Cleanup both intervals on component unmount
     return () => {
         clearInterval(sensorInterval);
         clearInterval(controllerInterval);
     };
    }, []);



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
       <Alert severity='error' >
        Soil moisture too low, please switch on water pump.
      </Alert>
      <SectionContent title='System status and direct control panel' titleGutter>
        
      <Paper
 
        elevation={20}
        sx={{
          p: 0.5,
          display: 'flex',
          flexDirection: 'row',
          mt: 1
        }}
      >
        <Stack spacing={0} direction="row" flexWrap='wrap'>
               
          {/* Power Consumption */}
          <Card sx={{ minWidth: 200, margin: 1 }}>
  <CardContent>
    <Typography variant="h6">Water Pump</Typography>
    <Typography variant="body1">
      {controllerData?.pumpState !== undefined 
        ? (controllerData.pumpState ? "On" : "Off") 
        : "Loading"}
    </Typography>
    <Typography variant="body2">
      {/* Optional: Add additional information, such as last run time */}
    </Typography>
    
    {/* Moisture Threshold Settings */}
    <Box mt={2}>
      <Typography variant="subtitle1">Soil Moisture Settings</Typography>
      
      <Box display="flex" alignItems="center" mt={1}>
        <Typography variant="body2" sx={{ minWidth: 90 }}>Min Moisture:</Typography>
        <Slider
          value={minMoisture}
          onChange={handleMinMoistureChange}
          aria-labelledby="min-moisture-slider"
          valueLabelDisplay="auto"
          min={0}
          max={100}
        />
        <Typography variant="body2">{minMoisture}%</Typography>
      </Box>

      <Box display="flex" alignItems="center" mt={1}>
        <Typography variant="body2" sx={{ minWidth: 90 }}>Max Moisture:</Typography>
        <Slider
          value={maxMoisture}
          onChange={handleMaxMoistureChange}
          aria-labelledby="max-moisture-slider"
          valueLabelDisplay="auto"
          min={0}
          max={100}
        />
        <Typography variant="body2">{maxMoisture}%</Typography>
      </Box>
    </Box>
    
    <Button 
      variant="contained" 
      color="primary" 
      onClick={handleSetMoistureThresholds}
      sx={{ mt: 2 }}
    >
      Set Thresholds
    </Button>
  </CardContent>
</Card>

<SensorCard
  title="Water Pump"
  value={pumpState ? 'On' : 'Off'}
  isLoading={loading}
  switch={true}  // Render the Switch component
  onSwitchChange={(checked) => {
    // Handle switch toggle (e.g., update pumpState or API call)
    console.log('Switch toggled:', checked);
  }}
  onButtonClick={handleSetMoistureThresholds}
  buttonLabel="Toggle Pump"
/>

      
      <SensorCard
        title="Soil Moisture Settings"
        value="Adjust moisture levels"
        settings={true}
        sliderData={{
          minMoisture,
          maxMoisture,
          handleMinMoistureChange,
          handleMaxMoistureChange,
        }}
        onButtonClick={handleSetMoistureThresholds}
        buttonLabel="Set Thresholds"
      />



<SensorCard
        title="Extractor Fan Settings"
        value="Adjust CO2 levels"
        settings={true}
        sliderData={{
          minMoisture,
          maxMoisture,
          handleMinMoistureChange,
          handleMaxMoistureChange,
        }}
        onButtonClick={handleSetMoistureThresholds}
        buttonLabel="Set Thresholds"
      />

<SensorCard
        title="Light Settings"
        value="Adjust Light levels"
        settings={true}
        sliderData={{
          minMoisture,
          maxMoisture,
          handleMinMoistureChange,
          handleMaxMoistureChange,
        }}
        onButtonClick={handleSetMoistureThresholds}
        buttonLabel="Set Thresholds"
      />







            {/* Power Consumption */}
        <Card sx={{ minWidth: 200, margin: 1 }}>
            <CardContent>
              <Typography variant="h6">Extractor Fan</Typography>
              <Typography variant="body1">
              {controllerData?.pumpState !== undefined 
              ? (controllerData.pumpState ? "On" : "Off") 
              : "Loading"}
              </Typography>
              <Typography variant="body2">
                 {/* Additional logic for power consumption */}
              </Typography>
            </CardContent>
          </Card>
          
                       {/* Power Consumption */}
        <Card sx={{ minWidth: 200, margin: 1 }}>
            <CardContent>
              <Typography variant="h6">Extractor Fan</Typography>
              <Typography variant="body1">
              <Typography>Schedule:</Typography>
    <Typography>On: 7:00 am</Typography>
    <Typography>Off: 21:00 pm</Typography>
              </Typography>
              <Typography variant="body2">
                 {/* Additional logic for power consumption */}
              </Typography>
            </CardContent>
          </Card>

        </Stack>
      </Paper>

    </SectionContent>
  
   
    </SnackbarProvider>
    </CustomTheme>
  );
};



export default ControlPanel;
