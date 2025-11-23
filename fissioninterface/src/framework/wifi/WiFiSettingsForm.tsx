import { FC, useContext, useEffect, useState } from 'react';
import { ValidateFieldsError } from 'async-validator';

import { Avatar, Button, Checkbox, IconButton, List, ListItem, ListItemAvatar, ListItemSecondaryAction, ListItemText, Alert, Box, Typography } from '@mui/material';
import LockOpenIcon from '@mui/icons-material/LockOpen';
import DeleteIcon from '@mui/icons-material/Delete';
import SaveIcon from '@mui/icons-material/Save';
import LockIcon from '@mui/icons-material/Lock';
import CheckCircleIcon from '@mui/icons-material/CheckCircle';
import WiFiService from "../../api/wifi";
import * as WiFiApi from "../../api/wifi";
import { WiFiSettings, WiFiStatus, WiFiConnectionStatus } from '../../types';
import { BlockFormControlLabel, ButtonRow, FormLoader, SectionContent, ValidatedPasswordField, ValidatedTextField } from '../../components';
import { validate, createWiFiSettingsValidator } from '../../validators';
import { updateValue, useRest } from '../../utils';
import { isNetworkOpen, networkSecurityMode } from './WiFiNetworkSelector';
import { WiFiConnectionContext } from './WiFiConnectionContext';
import { useSnackbar } from 'notistack';

const WiFiSettingsForm: FC = () => {
  const { enqueueSnackbar } = useSnackbar();
  const [data, setData] = useState<WiFiSettings | null>(null);  // Holds the Wi-Fi settings
  const [errorMessage, setErrorMessage] = useState<string>('');
  const [saving, setSaving] = useState(false); // Define the saving state
  const [wifiStatus, setWifiStatus] = useState<WiFiStatus | null>(null);

  const [wifiSettings, setwifiSettings] = useState<WiFiSettings | null>(null);
  const { selectedNetwork, deselectNetwork } = useContext(WiFiConnectionContext);
  const [loading, setLoading] = useState<boolean>(true);
  const [error, setError] = useState<string | null>(null);
  const [fieldErrors, setFieldErrors] = useState<ValidateFieldsError>();
  const [initialized, setInitialized] = useState(false);
  const validateAndSubmit = async () => {
    if (!data) {
      setErrorMessage('No data to save');
      return;
    }
    
    setSaving(true);
    setErrorMessage('');
    
    try {
      // Validate required fields
      if (!data.ssid || data.ssid.trim() === '') {
        setErrorMessage('SSID is required');
        setSaving(false);
        return;
      }
      
      // Call the save function
      await saveWiFiSettings(data);
      
      // Show success message
      setErrorMessage('');
      enqueueSnackbar('WiFi settings saved! Device is connecting...', { variant: 'success' });
      
      // Poll for connection status
      let attempts = 0;
      const checkConnection = async () => {
        try {
          const status = await WiFiService.readWiFiStatus();
          setWifiStatus(status);
          
          if (status.status === WiFiConnectionStatus.WIFI_STATUS_CONNECTED) {
            enqueueSnackbar(
              `Connected! Access device at: ${status.local_ip} or http://${data.hostname || 'neurogrow'}.local`,
              { variant: 'success', autoHideDuration: 10000 }
            );
          } else if (attempts < 15) {
            attempts++;
            setTimeout(checkConnection, 2000);
          }
        } catch (error) {
          console.error('Error checking connection:', error);
        }
      };
      setTimeout(checkConnection, 2000);
      
    } catch (error: any) {
      console.error('Error saving WiFi settings:', error);
      setErrorMessage(error?.message || 'Error saving WiFi settings');
      enqueueSnackbar('Failed to save WiFi settings', { variant: 'error' });
    } finally {
      setSaving(false);
    }
  };




  useEffect(() => {
    const fetchWiFiSettings = async () => {
      try {
        setLoading(true);
        const settings = await WiFiService.readWiFiSettings(); // Fetch from the API
        setData(settings);
      } catch (error) {
        setErrorMessage('Error fetching Wi-Fi settings');
        console.error(error);
      } finally {
        setLoading(false);
      }
    };

    const fetchWiFiStatus = async () => {
      try {
        const status = await WiFiService.readWiFiStatus();
        setWifiStatus(status);
      } catch (error) {
        console.error('Error fetching WiFi status:', error);
      }
    };

    fetchWiFiSettings();
    fetchWiFiStatus();
    
    // Poll WiFi status every 2 seconds to show connection status
    const statusInterval = setInterval(fetchWiFiStatus, 2000);
    return () => clearInterval(statusInterval);
  }, []); // Empty dependency array ensures this runs once after mount



  useEffect(() => {
    if (!initialized && data) {
      if (selectedNetwork) {
        setData({
          ssid: selectedNetwork.ssid,
          password: "",
          hostname: data?.hostname,
          static_ip_config: false,
        });
      }
      setInitialized(true);
    }
  }, [initialized, setInitialized, data, setData, selectedNetwork]);

  const updateFormValue = updateValue(setData);

  useEffect(() => deselectNetwork, [deselectNetwork]);

  const saveWiFiSettings = async (newSettings: WiFiSettings) => {
    try {
      setLoading(true);
      const updatedSettings = await WiFiService.updateWiFiSettings(newSettings); // Update via the API
      setData(updatedSettings);
    } catch (error) {
      setErrorMessage('Error saving Wi-Fi settings');
      console.error(error);
    } finally {
      setLoading(false);
    }
  };


    const isConnected = wifiStatus?.status === WiFiConnectionStatus.WIFI_STATUS_CONNECTED;
    
    return (
      <SectionContent title="Wifi Configuration">
        {isConnected && wifiStatus?.local_ip && (
          <Alert 
            icon={<CheckCircleIcon />} 
            severity="success" 
            sx={{ mb: 2 }}
          >
            <Typography variant="body2" component="div">
              <strong>Connected to WiFi!</strong>
              <br />
              Access device at: <strong>{wifiStatus.local_ip}</strong>
              <br />
              Or use mDNS: <strong>http://{data?.hostname || 'neurogrow'}.local</strong>
            </Typography>
          </Alert>
        )}
        {errorMessage && (
          <Alert severity="error" sx={{ mb: 2 }}>
            {errorMessage}
          </Alert>
        )}
        {
          selectedNetwork ?
            <List>
              <ListItem>
                <ListItemAvatar>
                  <Avatar>
                    {isNetworkOpen(selectedNetwork) ? <LockOpenIcon /> : <LockIcon />}
                  </Avatar>
                </ListItemAvatar>
                <ListItemText
                  primary={selectedNetwork.ssid}
                  secondary={"Security: " + networkSecurityMode(selectedNetwork) + ", Ch: " + selectedNetwork.channel}
                />
                <ListItemSecondaryAction>
                  <IconButton aria-label="Manual Config" onClick={deselectNetwork}>
                    <DeleteIcon />
                  </IconButton>
                </ListItemSecondaryAction>
              </ListItem>
            </List>
            :
            <ValidatedTextField
              fieldErrors={fieldErrors}
              name="ssid"
              label="SSID"
              fullWidth
              variant="outlined"
              value={data?.ssid}
              onChange={updateFormValue}
              margin="normal"
            />
        }
        {
          (!selectedNetwork || !isNetworkOpen(selectedNetwork)) &&
          <ValidatedPasswordField
            fieldErrors={fieldErrors}
            name="password"
            label="Password"
            fullWidth
            variant="outlined"
            value={data?.password}
            onChange={updateFormValue}
            margin="normal"
          />
        }
        <ValidatedTextField
          fieldErrors={fieldErrors}
          name="hostname"
          label="Hostname"
          fullWidth
          variant="outlined"
          value={data?.hostname}
          onChange={updateFormValue}
          margin="normal"
        />
        <BlockFormControlLabel
          control={
            <Checkbox
              name="static_ip_config"
              checked={data?.static_ip_config}
              onChange={updateFormValue}
            />
          }
          label="Static IP Config?"
        />
        {
          data?.static_ip_config &&
          <>
            <ValidatedTextField
              fieldErrors={fieldErrors}
              name="local_ip"
              label="Local IP"
              fullWidth
              variant="outlined"
              value={data?.local_ip}
              onChange={updateFormValue}
              margin="normal"
            />
            <ValidatedTextField
              fieldErrors={fieldErrors}
              name="gateway_ip"
              label="Gateway"
              fullWidth
              variant="outlined"
              value={data?.gateway_ip}
              onChange={updateFormValue}
              margin="normal"
            />
            <ValidatedTextField
              fieldErrors={fieldErrors}
              name="subnet_mask"
              label="Subnet"
              fullWidth
              variant="outlined"
              value={data?.subnet_mask}
              onChange={updateFormValue}
              margin="normal"
            />
            <ValidatedTextField
              fieldErrors={fieldErrors}
              name="dns_ip_1"
              label="DNS IP #1"
              fullWidth
              variant="outlined"
              value={data?.dns_ip_1}
              onChange={updateFormValue}
              margin="normal"
            />
            <ValidatedTextField
              fieldErrors={fieldErrors}
              name="dns_ip_2"
              label="DNS IP #2"
              fullWidth
              variant="outlined"
              value={data?.dns_ip_2}
              onChange={updateFormValue}
              margin="normal"
            />
          </>
        }
        <ButtonRow mt={1}>
          <Button startIcon={<SaveIcon />} disabled={saving} variant="contained" color="primary" type="submit" onClick={validateAndSubmit}>
            Save
          </Button>
        </ButtonRow>
        </SectionContent>
    );



    
  };

  

export default WiFiSettingsForm;
