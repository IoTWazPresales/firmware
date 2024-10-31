import React, { FC, RefObject, useEffect ,useState } from 'react';
import logo from './logo.svg';
import './App.css';
import api from './api/sensors';
import { SnackbarProvider } from 'notistack';
import { BrowserRouter as Router } from 'react-router-dom'; // Import BrowserRouter
import { IconButton, Button, Typography,
  Paper,
  Stack,
  Alert,
  Card,
  CardContent,  
  CardActions,
  Switch  } from '@mui/material';
  import { useWs } from './utils/useWs';
  import { updateValue} from './utils/binding';
  
import { WEB_SOCKET_ROOT } from './api/endpoints';
import CloseIcon from '@mui/icons-material/Close';
import CustomTheme from './CustomTheme';
import SectionContent from './SectionContent';
import { Layout } from './components/layout'; 
import {TemperatureState, SensorPHState} from './utils/types'
import SensorService, {SensorData} from './api/SensorService';
import SensorCard from './components/SensorCard';


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

const App: FC = () => {
  const [sensorData, setSensorData] = useState<SensorData | null>(null);
    const [loading, setLoading] = useState<boolean>(true);
    const [error, setError] = useState<string | null>(null);

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

        
        const intervalId = setInterval(fetchSensorData, 5000); // Fetch data every 5 seconds

        // Clean up the interval on component unmount
        return () => clearInterval(intervalId);
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
    <Router>    
    <Layout> 
    <SectionContent title='GrowBox Dashboard' titleGutter>
      <Typography variant="body1" paragraph>
        System status and direct control panel.
      </Typography>
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
          

         {/* Water Temperature */}
         <Card sx={{ minWidth: 200, margin: 1 }}>
            <CardContent>
              <Typography variant="h6">Water Temperature</Typography>
              <Typography variant="body1">
               {sensorData?.temperature??"Loading"}°C

              </Typography>
            </CardContent>
          </Card>
 {/* Air Temperature */}
 <Card sx={{ minWidth: 200, margin: 1 }}>
            <CardContent>
              <Typography variant="h6">Air Temperature</Typography>
              <Typography variant="body1">
              loading
              </Typography>
            </CardContent>
          </Card>
          {/* Air Humidity */}
          <Card sx={{ minWidth: 200, margin: 1 }}>
            <CardContent>
              <Typography variant="h6">Air Humidity</Typography>
              <Typography variant="body1">
              Not Detected °C%RH
              </Typography>
            </CardContent>
          </Card>
           {/* Soil Moisture */}
           <Card sx={{ minWidth: 200, margin: 1 }}>
            <CardContent>
              <Typography variant="h6">Soil Moisture</Typography>
              <Typography variant="body1">
              Not Detected °C %
              </Typography>
          
            </CardContent>
          </Card>
           {/* Water Particulates */}
           <Card sx={{ minWidth: 200, margin: 1 }}>
            <CardContent>
              <Typography variant="h6">Water Particulates</Typography>
              <Typography variant="body1">
              Not Detected °C %
              </Typography>
              
            </CardContent>
          </Card>
         {/* Water PH */}
         <Card sx={{ minWidth: 200, margin: 1 }}>
            <CardContent>
              <Typography variant="h6">Water PH</Typography>
              <Typography variant="body1">
               {sensorData?.ph??"Loading"}pH
              </Typography>
            </CardContent>
          </Card>
           {/* RTC Timestamp */}
           <Card sx={{ minWidth: 200, margin: 1 }}>
            <CardContent>
              <Typography variant="h6">RTC Timestamp</Typography>
              <Typography variant="body1">
              Not Detected °C
              </Typography>
            </CardContent>
          </Card>
       
        {/* Power Consumption */}
        <Card sx={{ minWidth: 200, margin: 1 }}>
            <CardContent>
              <Typography variant="h6">Power Consumption</Typography>
              <Typography variant="body1">
              Not Detected °C
              </Typography>
              <Typography variant="body2">
                 {/* Additional logic for power consumption */}
              </Typography>
            </CardContent>
          </Card>
          <Card variant="outlined" sx={{ mb: 2 }}>
  <CardContent>
    <Typography variant="h6">Lights</Typography>
    <Stack direction="row" spacing={2} alignItems="center">
      <Typography>On/Off:</Typography>
      <Typography>{true ? "On" : "Off"}</Typography> {/* Replace true with actual boolean state if needed */}
    </Stack>
    <Typography>Schedule:</Typography>
    <Typography>On: 7:00 am</Typography>
    <Typography>Off: 21:00 pm</Typography>
  </CardContent>
</Card>
<Card variant="outlined" sx={{ mb: 2 }}>
  <CardContent>
    <Typography variant="h6">Air Pump</Typography>
    <Stack direction="row" spacing={2} alignItems="center">
      <Typography>On/Off:</Typography>
      <Typography>{false ? "On" : "Off"}</Typography> {/* Replace false with actual boolean state if needed */}
    </Stack>
  </CardContent>
</Card>
<Card variant="outlined" sx={{ mb: 2 }}>
  <CardContent>
    <Typography variant="h6">Extractor Fan</Typography>
    <Stack direction="row" spacing={2} alignItems="center">
      <Typography>On/Off:</Typography>
      <Typography>{true ? "On" : "Off"}</Typography> {/* Replace true with actual boolean state if needed */}
    </Stack>
  </CardContent>
</Card>
        </Stack>
      </Paper>

    </SectionContent>
    </Layout>
    </Router>
    </SnackbarProvider>
    </CustomTheme>
  );
};





  /*return (
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
       
      </SnackbarProvider>
    </CustomTheme>
  );
};
*/

export default App;
