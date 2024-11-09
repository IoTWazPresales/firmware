import React, { FC, RefObject, useEffect ,useState } from 'react';
import logo from './logo.svg';
import '../App.css';
import { SnackbarProvider } from 'notistack';
import { BrowserRouter as Router } from 'react-router-dom'; // Import BrowserRouter
import { RouterTabs, useLayoutTitle, useRouterTab } from './../components';
import { IconButton, Button, Typography,
  Paper,
  Stack,
  Alert,
  Card,
  CardContent,  
  CardActions,
  Switch  } from '@mui/material';
  import { useWs } from '../utils/useWs';
  import { updateValue} from '../utils/binding';
  
import CloseIcon from '@mui/icons-material/Close';
import CustomTheme from '../CustomTheme';
import SectionContent from '../SectionContent';
import { Layout } from '../components/layout'; 
import SensorService, {SensorData} from '../api/SensorService';
import ControllerService, {ControllerData} from '../api/ControllerService';
import SensorCard from '../components/SensorCard';



const Dashboard: FC = () => {
  useLayoutTitle("Dashboard");
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
        /*const fetchControllerData = async () => {
          try {
            console.log("try fetch data 2")
             const data = await ControllerService.getControllerData();
             setControllerData(data);
         } catch (err) {
             setError('Failed to fetch sensor data.');
         } finally {
             setLoading(false);
         }
     };*/
        
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
          

         {/* Water Temperature */}
         <Card sx={{ minWidth: 200, margin: 1 }}>
            <CardContent>
              <Typography variant="h6">Water Temperature</Typography>
              <Typography variant="body1">
              {sensorData?.temperature !== -1 && sensorData?.temperature != null ? `${sensorData.temperature} °C` : "Loading"}
               
              </Typography>
            </CardContent>
          </Card>
 {/* Air Temperature */}
 <Card sx={{ minWidth: 200, margin: 1 }}>
            <CardContent>
              <Typography variant="h6">Air Temperature</Typography>
              <Typography variant="body1">
              </Typography>
            </CardContent>
          </Card>
          {/* Air Humidity */}
          <Card sx={{ minWidth: 200, margin: 1 }}>
            <CardContent>
              <Typography variant="h6">Air Humidity</Typography>
              <Typography variant="body1">
              </Typography>
            </CardContent>
          </Card>
           {/* Soil Moisture */}
           <Card sx={{ minWidth: 200, margin: 1 }}>
            <CardContent>
              <Typography variant="h6">Soil Moisture</Typography>
              <Typography variant="body1">
              
              </Typography>
          
            </CardContent>
          </Card>
           {/* Water Particulates */}
           <Card sx={{ minWidth: 200, margin: 1 }}>
            <CardContent>
              <Typography variant="h6">Water Particulates</Typography>
              <Typography variant="body1">
               
              </Typography>
              
            </CardContent>
          </Card>
         {/* Water PH */}
         <Card sx={{ minWidth: 200, margin: 1 }}>
            <CardContent>
              <Typography variant="h6">Water PH</Typography>
              <Typography variant="body1">
              {sensorData?.ph !== -1 && sensorData?.ph != null ? `${sensorData.ph} pH` : "Loading"}
              </Typography>
            </CardContent>
          </Card>
           {/* RTC Timestamp */}
           <Card sx={{ minWidth: 200, margin: 1 }}>
            <CardContent>
              <Typography variant="h6">RTC Timestamp</Typography>
              <Typography variant="body1">
               
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
      <Typography></Typography>
    </Stack>
    <Typography>Schedule:</Typography>
    <Typography>On: 7:00 am</Typography>
    <Typography>Off: 21:00 pm</Typography>
  </CardContent>
</Card>
<Card variant="outlined" sx={{ mb: 2 }}>
  <CardContent>
    <Typography variant="h6">Water Pump</Typography>
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

export default Dashboard;
