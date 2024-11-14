import { FC, useState, useEffect } from "react";

import { Avatar, Button, Divider, List, ListItem, ListItemAvatar, ListItemText, Theme, useTheme } from "@mui/material";
import SettingsInputComponentIcon from '@mui/icons-material/SettingsInputComponent';
import SettingsInputAntennaIcon from '@mui/icons-material/SettingsInputAntenna';
import DeviceHubIcon from '@mui/icons-material/DeviceHub';
import WifiIcon from '@mui/icons-material/Wifi';
import DnsIcon from '@mui/icons-material/Dns';
import RefreshIcon from '@mui/icons-material/Refresh';
import WiFiService from "../../api/wifi";


import * as WiFiApi from "../../api/wifi";
import { WiFiConnectionStatus, WiFiStatus } from "../../types";
import { ButtonRow, FormLoader, SectionContent } from "../../components";
import { useRest } from "../../utils";

const isConnected = ({ status }: WiFiStatus) => status === WiFiConnectionStatus.WIFI_STATUS_CONNECTED;

const wifiStatusHighlight = ({ status }: WiFiStatus, theme: Theme) => {
  switch (status) {
    case WiFiConnectionStatus.WIFI_STATUS_IDLE:
    case WiFiConnectionStatus.WIFI_STATUS_DISCONNECTED:
    case WiFiConnectionStatus.WIFI_STATUS_NO_SHIELD:
      return theme.palette.info.main;
    case WiFiConnectionStatus.WIFI_STATUS_CONNECTED:
      return theme.palette.success.main;
    case WiFiConnectionStatus.WIFI_STATUS_CONNECT_FAILED:
    case WiFiConnectionStatus.WIFI_STATUS_CONNECTION_LOST:
      return theme.palette.error.main;
    default:
      return theme.palette.warning.main;
  }
};

const wifiStatus = ({ status }: WiFiStatus) => {
  switch (status) {
    case WiFiConnectionStatus.WIFI_STATUS_NO_SHIELD:
      return "Inactive";
    case WiFiConnectionStatus.WIFI_STATUS_IDLE:
      return "Idle";
    case WiFiConnectionStatus.WIFI_STATUS_NO_SSID_AVAIL:
      return "No SSID Available";
    case WiFiConnectionStatus.WIFI_STATUS_CONNECTED:
      return "Connected";
    case WiFiConnectionStatus.WIFI_STATUS_CONNECT_FAILED:
      return "Connection Failed";
    case WiFiConnectionStatus.WIFI_STATUS_CONNECTION_LOST:
      return "Connection Lost";
    case WiFiConnectionStatus.WIFI_STATUS_DISCONNECTED:
      return "Disconnected";
    default:
      return "Unknown";
  }
};

const dnsServers = ({ dns_ip_1, dns_ip_2 }: WiFiStatus) => {
  if (!dns_ip_1) {
    return "none";
  }
  return dns_ip_1 + (dns_ip_2 ? ',' + dns_ip_2 : '');
};

const WiFiStatusForm: FC = () => {
  const [wifiData, setwifiData] = useState<WiFiStatus | null>(null);
  const [loading, setLoading] = useState<boolean>(true);
  const [error, setError] = useState<string | null>(null);
  
  const fetchWifiData = async () => {
    try {
      const data = await WiFiService.readWiFiStatus();
      setwifiData(data);
    } catch (err) {
      setError('Failed to fetch WiFi data.');
    } finally {
      setLoading(false);
    }
  };
  const handleRefresh = () => {
    setLoading(true);
    fetchWifiData();
  };
  useEffect(() => {
    const wifiInterval = setInterval(fetchWifiData, 5000);

    return () => {
      clearInterval(wifiInterval);
    };
  }, []);

  const theme = useTheme();

  const content = () => {
    if (loading) return <FormLoader />;
    if (error) return <p>{error}</p>;
    if (!wifiData) return null;

    return (
      <>
        <List>
          <ListItem>
            <ListItemAvatar>
              <Avatar sx={{ bgcolor: wifiStatusHighlight(wifiData, theme) }}>
                <WifiIcon />
              </Avatar>
            </ListItemAvatar>
            <ListItemText primary="Status" secondary={wifiStatus(wifiData)} />
          </ListItem>
          <Divider variant="inset" component="li" />
          {
            isConnected(wifiData) &&
            <>
              <ListItem>
                <ListItemAvatar>
                  <Avatar>
                    <SettingsInputAntennaIcon />
                  </Avatar>
                </ListItemAvatar>
                <ListItemText primary="SSID" secondary={wifiData.ssid} />
              </ListItem>
              <Divider variant="inset" component="li" />
              <ListItem>
                <ListItemAvatar>
                  <Avatar>IP</Avatar>
                </ListItemAvatar>
                <ListItemText primary="IP Address" secondary={wifiData.local_ip} />
              </ListItem>
              <Divider variant="inset" component="li" />
              <ListItem>
                <ListItemAvatar>
                  <Avatar>
                    <DeviceHubIcon />
                  </Avatar>
                </ListItemAvatar>
                <ListItemText primary="MAC Address" secondary={wifiData.mac_address} />
              </ListItem>
              <Divider variant="inset" component="li" />
              <ListItem>
                <ListItemAvatar>
                  <Avatar>#</Avatar>
                </ListItemAvatar>
                <ListItemText primary="Subnet Mask" secondary={wifiData.subnet_mask} />
              </ListItem>
              <Divider variant="inset" component="li" />
              <ListItem>
                <ListItemAvatar>
                  <Avatar>
                    <SettingsInputComponentIcon />
                  </Avatar>
                </ListItemAvatar>
                <ListItemText primary="Gateway IP" secondary={wifiData.gateway_ip || "none"} />
              </ListItem>
              <Divider variant="inset" component="li" />
              <ListItem>
                <ListItemAvatar>
                  <Avatar>
                    <DnsIcon />
                  </Avatar>
                </ListItemAvatar>
                <ListItemText primary="DNS Server IP" secondary={dnsServers(wifiData)} />
              </ListItem>
              <Divider variant="inset" component="li" />
            </>
          }
        </List>
        <ButtonRow pt={1}>
          <Button startIcon={<RefreshIcon />} variant="contained" color="secondary" onClick={handleRefresh}>
            Refresh
          </Button>
        </ButtonRow>
      </>
    );
  };

  return (
    <SectionContent title='WiFi Status' titleGutter>
      {content()}
    </SectionContent>
  );

};

export default WiFiStatusForm;
