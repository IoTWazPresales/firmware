import { useEffect, FC, useState, useCallback, useRef } from 'react';
import { useSnackbar } from 'notistack';
import WiFiService from "../../api/wifi";
import { Button, } from '@mui/material';
import PermScanWifiIcon from '@mui/icons-material/PermScanWifi';

import * as WiFiApi from "../../api/wifi";

import { WiFiNetwork, WiFiNetworkList } from '../../types';
import { ButtonRow, FormLoader, SectionContent } from '../../components';
import { extractErrorMessage } from '../../utils';

import WiFiNetworkSelector from './WiFiNetworkSelector';

const NUM_POLLS = 10;
const POLLING_FREQUENCY = 500;

const compareNetworks = (network1: WiFiNetwork, network2: WiFiNetwork) => {
  if (network1.rssi < network2.rssi)
    return 1;
  if (network1.rssi > network2.rssi)
    return -1;
  return 0;
};

const WiFiNetworkScanner: FC = () => {

  const { enqueueSnackbar } = useSnackbar();
  
  const pollCount = useRef(0);
  const [networkList, setNetworkList] = useState<WiFiNetworkList | undefined>(undefined);
  const [errorMessage, setErrorMessage] = useState<string>();


  const finishedWithError = useCallback((message: string) => {
    enqueueSnackbar(message, { variant: 'error' });
    setNetworkList(undefined);
    setErrorMessage(message);
  }, [enqueueSnackbar]);

  const pollNetworkList = useCallback(async () => {
    try {
      const response = await WiFiService.listNetworks();
      if (response && response.networks) {
        // Check if scan is still in progress (networks array exists but might be empty)
        if ((response as any).scanning === true) {
          // Scan still in progress, poll again
          pollCount.current++;
          if (pollCount.current < NUM_POLLS) {
            setTimeout(pollNetworkList, POLLING_FREQUENCY);
          } else {
            finishedWithError('Scan timeout - please try again');
          }
          return;
        }
        
        // Sorting network list by signal strength (RSSI)
        response.networks.sort((network1, network2) => network2.rssi - network1.rssi);
        setNetworkList(response);
        pollCount.current = 0;  // Reset poll count on success
      } else {
        // No networks found - might be scan in progress or truly no networks
        pollCount.current++;
        if (pollCount.current < NUM_POLLS) {
          setTimeout(pollNetworkList, POLLING_FREQUENCY);
        } else {
          finishedWithError("No networks found.");
        }
      }
    } catch (error: any) {
      pollCount.current++;
      if (pollCount.current < NUM_POLLS) {
        // Retry on error (might be network issue)
        setTimeout(pollNetworkList, POLLING_FREQUENCY);
      } else {
        finishedWithError('Error fetching network list.');
      }
    }
  }, [finishedWithError]);


 const startNetworkScan = useCallback(async () => {
    setNetworkList(undefined);  // Clear previous results
    setErrorMessage(undefined);
    pollCount.current = 0;
    try {
      await WiFiService.scanNetworks();  // Trigger scan
      setTimeout(pollNetworkList, POLLING_FREQUENCY);  // Start polling for network list after scan
    } catch (error: any) {
      finishedWithError('Problem scanning for WiFi networks');
    }
  }, [pollNetworkList, finishedWithError]);

  useEffect(() => {
    // On mount, try to get existing scan results first (from pre-scan)
    // Don't trigger a new scan immediately - check if results already exist
    // This prevents disconnection when opening the tab
    pollNetworkList();
  }, []); // Empty deps - only run once on mount
 

  const renderNetworkScanner = () => {
    if (!networkList) {
      return (<FormLoader message="Scanning&hellip;" errorMessage={errorMessage} />);
    }
    return (
      <WiFiNetworkSelector networkList={networkList} />
    );
  };

  return (
    <SectionContent title="Network Scanner">
      {renderNetworkScanner()}
      <ButtonRow pt={1}>
        <Button
          startIcon={<PermScanWifiIcon />}
          variant="contained"
          color="secondary"
          onClick={startNetworkScan}
          disabled={!errorMessage && !networkList}
        >
          Scan again&hellip;
        </Button>
      </ButtonRow>
    </SectionContent>
  );

};

export default WiFiNetworkScanner;
