import { FC, useContext, useState,useEffect } from "react";
import { useSnackbar } from "notistack";

import {
  Avatar, Box, Button, Dialog, DialogActions, DialogContent, DialogTitle,
  Divider, List, ListItem, ListItemAvatar, ListItemText
} from "@mui/material";
import DevicesIcon from '@mui/icons-material/Devices';
import ShowChartIcon from '@mui/icons-material/ShowChart';
import MemoryIcon from '@mui/icons-material/Memory';
import AppsIcon from '@mui/icons-material/Apps';
import DataUsageIcon from '@mui/icons-material/DataUsage';
import SdStorageIcon from '@mui/icons-material/SdStorage';
import FolderIcon from '@mui/icons-material/Folder';
import RefreshIcon from '@mui/icons-material/Refresh';
import PowerSettingsNewIcon from '@mui/icons-material/PowerSettingsNew';
import SettingsBackupRestoreIcon from '@mui/icons-material/SettingsBackupRestore';
import SystemService from "../../api/system";
import * as SystemApi from "../../api/system";
import { EspPlatform, SystemStatus } from "../../types";
import { ButtonRow, FormLoader, SectionContent } from "../../components";
import { extractErrorMessage, useRest } from "../../utils";


function formatNumber(num: number) {
  return new Intl.NumberFormat().format(num);
}

const SystemStatusForm: FC = () => {
  
  const [loading, setLoading] = useState<boolean>(true);
  const [error, setError] = useState<string>();
 const [systemStatus, setsystemStatus] = useState<SystemStatus>();


    const fetchSystemData = async () => {
         try {
            const data = await SystemService.readSystemStatus();
            setsystemStatus(data);
           
        } catch (err) {
            setError('Failed to system data.');
        } finally {
            setLoading(false);
        }
    };
    const handleRefresh = () => {
      setLoading(true);
      fetchSystemData();
    };
    useEffect(() => {
      const SystemInterval = setInterval(fetchSystemData, 5000);
  
      return () => {
        clearInterval(SystemInterval);
      };
    }, []);


 // Cleanup both intervals on component unmount
 




















 
  const [confirmRestart, setConfirmRestart] = useState<boolean>(false);
  const [confirmFactoryReset, setConfirmFactoryReset] = useState<boolean>(false);
  const [processing, setProcessing] = useState<boolean>(false);
  const { enqueueSnackbar } = useSnackbar();

  const restart = async () => {
    setProcessing(true);
    try {
      await SystemService.restart();
      enqueueSnackbar("Device is restarting", { variant: 'info' });
    } catch (error: any) {
      enqueueSnackbar(extractErrorMessage(error, 'Problem restarting device'), { variant: 'error' });
    } finally {
      setConfirmRestart(false);
      setProcessing(false);
    }
  };

  const renderRestartDialog = () => (
    <Dialog
      open={confirmRestart}
      onClose={() => setConfirmRestart(false)}
    >
      <DialogTitle>Confirm Restart</DialogTitle>
      <DialogContent dividers>
        Are you sure you want to restart the device?
      </DialogContent>
      <DialogActions>
        <Button variant="contained" onClick={() => setConfirmRestart(false)} color="secondary">
          Cancel
        </Button>
        <Button
          startIcon={<PowerSettingsNewIcon />}
          variant="contained"
          onClick={restart}
          disabled={processing}
          color="primary"
          autoFocus
        >
          Restart
        </Button>
      </DialogActions>
    </Dialog>
  );

  const factoryReset = async () => {
    setProcessing(true);
    try {
      await SystemService.factoryReset();
      enqueueSnackbar("Device has been factory reset and will now restart", { variant: 'info' });
    } catch (error: any) {
      enqueueSnackbar(extractErrorMessage(error, 'Problem factory resetting the device'), { variant: 'error' });
    } finally {
      setConfirmFactoryReset(false);
      setProcessing(false);
    }
  };

  const renderFactoryResetDialog = () => (
    <Dialog
      open={confirmFactoryReset}
      onClose={() => setConfirmFactoryReset(false)}
    >
      <DialogTitle>Confirm Factory Reset</DialogTitle>
      <DialogContent dividers>
        Are you sure you want to reset the device to its factory defaults?
      </DialogContent>
      <DialogActions>
        <Button variant="contained" onClick={() => setConfirmFactoryReset(false)} color="secondary">
          Cancel
        </Button>
        <Button
          startIcon={<SettingsBackupRestoreIcon />}
          variant="contained"
          onClick={factoryReset}
          disabled={processing}
          autoFocus
          color="error"
        >
          Factory Reset
        </Button>
      </DialogActions>
    </Dialog>
  );

  const content = () => {
    if (!systemStatus) {
      return (<FormLoader onRetry={systemStatus} errorMessage={error} />);
    }

    return (
      <>
        <List>
          <ListItem>
            <ListItemAvatar>
              <Avatar>
                <DevicesIcon />
              </Avatar>
            </ListItemAvatar>
            <ListItemText primary="Device (Platform / SDK)" secondary={systemStatus.esp_platform + ' / ' + systemStatus.sdk_version} />
          </ListItem>
          <Divider variant="inset" component="li" />
          <ListItem >
            <ListItemAvatar>
              <Avatar>
                <ShowChartIcon />
              </Avatar>
            </ListItemAvatar>
            <ListItemText primary="CPU Frequency" secondary={systemStatus.cpu_freq_mhz + ' MHz'} />
          </ListItem>
          <Divider variant="inset" component="li" />
          <ListItem >
            <ListItemAvatar>
              <Avatar>
                <MemoryIcon />
              </Avatar>
            </ListItemAvatar>
            <ListItemText
              primary="Heap (Free / Max Alloc)"
              secondary={
                formatNumber(systemStatus.free_heap) +
                ' / ' +
                formatNumber(systemStatus.max_alloc_heap) +
                ' bytes ' +
                (systemStatus.esp_platform === EspPlatform.ESP8266 ? '(' + systemStatus.heap_fragmentation + '% fragmentation)' : '')
              }
            />
          </ListItem>
          {
            (
              systemStatus.esp_platform === EspPlatform.ESP32 && systemStatus.psram_size > 0) && (
              <>
                <Divider variant="inset" component="li" />
                <ListItem >
                  <ListItemAvatar>
                    <Avatar>
                      <AppsIcon />
                    </Avatar>
                  </ListItemAvatar>
                  <ListItemText
                    primary="PSRAM (Size / Free)"
                    secondary={formatNumber(systemStatus.psram_size) + ' / ' + formatNumber(systemStatus.free_psram) + ' bytes'}
                  />
                </ListItem>
              </>
            )
          }
          <Divider variant="inset" component="li" />
          <ListItem >
            <ListItemAvatar>
              <Avatar>
                <DataUsageIcon />
              </Avatar>
            </ListItemAvatar>
            <ListItemText
              primary="Sketch (Size / Free)"
              secondary={formatNumber(systemStatus.sketch_size) + ' / ' + formatNumber(systemStatus.free_sketch_space) + ' bytes'}
            />
          </ListItem>
          <Divider variant="inset" component="li" />
          <ListItem >
            <ListItemAvatar>
              <Avatar>
                <SdStorageIcon />
              </Avatar>
            </ListItemAvatar>
            <ListItemText
              primary="Flash Chip (Size / Speed)"
              secondary={formatNumber(systemStatus.flash_chip_size) + ' bytes / ' + (systemStatus.flash_chip_speed / 1000000).toFixed(0) + ' MHz'}
            />
          </ListItem>
          <Divider variant="inset" component="li" />
          <ListItem >
            <ListItemAvatar>
              <Avatar>
                <FolderIcon />
              </Avatar>
            </ListItemAvatar>
            <ListItemText
              primary="File System (Used / Total)"
              secondary={
                formatNumber(systemStatus.fs_used) +
                ' / ' +
                formatNumber(systemStatus.fs_total) +
                ' bytes (' + formatNumber(systemStatus.fs_total - systemStatus.fs_used) + '\xa0bytes free)'
              }
            />
          </ListItem>
          <Divider variant="inset" component="li" />
        </List>
        <Box display="flex" flexWrap="wrap">
          <Box flexGrow={1}>
            <ButtonRow mt={1}>
              <Button startIcon={<RefreshIcon />} variant="contained" color="secondary" onClick={handleRefresh}>
                Refresh
              </Button>
            </ButtonRow>
          </Box>
          {
           
            <Box flexWrap="nowrap" whiteSpace="nowrap">
              <ButtonRow mt={1}>
                <Button startIcon={<PowerSettingsNewIcon />} variant="contained" color="primary" onClick={() => setConfirmRestart(true)}>
                  Restart
                </Button>
                <Button
                  startIcon={<SettingsBackupRestoreIcon />}
                  variant="contained"
                  onClick={() => setConfirmFactoryReset(true)}
                  color="error"
                >
                  Factory reset
                </Button>
              </ButtonRow>
            </Box>
          }
        </Box>
        {renderRestartDialog()}
        {renderFactoryResetDialog()}
      </>
    );
  };

  return (
    <SectionContent title='System Status' titleGutter>
      {content()}
    </SectionContent>
  );

};

export default SystemStatusForm;
