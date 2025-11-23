import { FC, useContext, useEffect, useState } from 'react';
import { ValidateFieldsError } from 'async-validator';

import { Avatar, Button, Checkbox, IconButton, List, ListItem, ListItemAvatar, ListItemSecondaryAction, ListItemText, Dialog, DialogTitle, DialogContent, DialogContentText, DialogActions, Alert } from '@mui/material';
import LockOpenIcon from '@mui/icons-material/LockOpen';
import DeleteIcon from '@mui/icons-material/Delete';
import SaveIcon from '@mui/icons-material/Save';
import LockIcon from '@mui/icons-material/Lock';
import InfoIcon from '@mui/icons-material/Info';
import WiFiService from "../../api/wifi";
import * as WiFiApi from "../../api/wifi";
import { WiFiSettings } from '../../types';
import { BlockFormControlLabel, ButtonRow, FormLoader, SectionContent, ValidatedPasswordField, ValidatedTextField } from '../../components';
import { validate, createWiFiSettingsValidator } from '../../validators';
import { updateValue, useRest } from '../../utils';
import { WiFiConnectionStatus, WiFiStatus } from "../../types";
import { isNetworkOpen, networkSecurityMode } from './WiFiNetworkSelector';
import { WiFiConnectionContext } from './WiFiConnectionContext';

const WiFiSettingsForm: FC = () => {
  const [data, setData] = useState<WiFiSettings | null>(null);  // Holds the Wi-Fi settings
  const [errorMessage, setErrorMessage] = useState<string>('');
  const [saving, setSaving] = useState(false); // Define the saving state

  const [wifiSettings, setwifiSettings] = useState<WiFiSettings | null>(null);
  const { selectedNetwork, deselectNetwork } = useContext(WiFiConnectionContext);
  const [loading, setLoading] = useState<boolean>(true);
  const [error, setError] = useState<string | null>(null);
  const [fieldErrors, setFieldErrors] = useState<ValidateFieldsError>();
  const [initialized, setInitialized] = useState(false);
  const [showSuccessDialog, setShowSuccessDialog] = useState(false);
  const [savedSSID, setSavedSSID] = useState<string>('');
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
      
      // Show success dialog with instructions
      setSavedSSID(data.ssid);
      setShowSuccessDialog(true);
      setErrorMessage('');
      
    } catch (error: any) {
      console.error('Error saving WiFi settings:', error);
      setErrorMessage(error?.message || 'Error saving WiFi settings');
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

    fetchWiFiSettings();
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


    return (
      <SectionContent title="Wifi Configuration">
        <Alert severity="info" icon={<InfoIcon />} sx={{ mb: 2 }}>
          <strong>After saving:</strong> The device will connect to your WiFi network. You'll need to disconnect from "NeuroGrow-Setup" and connect to the same network, then access the device at <strong>http://neurogrow.local</strong>
        </Alert>
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
        
        <Dialog open={showSuccessDialog} onClose={() => setShowSuccessDialog(false)} maxWidth="sm" fullWidth>
          <DialogTitle>WiFi Settings Saved</DialogTitle>
          <DialogContent>
            <DialogContentText component="div">
              <p><strong>The device is now connecting to "{savedSSID}"...</strong></p>
              <p>Follow these steps to reconnect:</p>
              <ol style={{ marginLeft: '20px', paddingLeft: '20px' }}>
                <li>Disconnect from <strong>"NeuroGrow-Setup"</strong> WiFi network on your device</li>
                <li>Connect to <strong>"{savedSSID}"</strong> on your device</li>
                <li>Access the device at: <strong>http://neurogrow.local</strong></li>
                <li>Or check your router's connected devices list for the IP address</li>
              </ol>
            </DialogContentText>
          </DialogContent>
          <DialogActions>
            <Button onClick={() => setShowSuccessDialog(false)} color="primary" variant="contained">
              Got it
            </Button>
          </DialogActions>
        </Dialog>
        </SectionContent>
    );



    
  };

  

export default WiFiSettingsForm;
