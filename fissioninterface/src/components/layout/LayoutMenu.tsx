import { FC, useContext } from 'react';

import { Divider, List } from '@mui/material';
import SettingsRemoteIcon from '@mui/icons-material/SettingsRemote';

import SettingsInputAntennaIcon from '@mui/icons-material/SettingsInputAntenna';
import AccessTimeIcon from '@mui/icons-material/AccessTime';
import DeviceHubIcon from '@mui/icons-material/DeviceHub';
import SettingsIcon from '@mui/icons-material/Settings';
import LockIcon from '@mui/icons-material/Lock';
import WifiIcon from '@mui/icons-material/Wifi';

import ProjectMenu from '../../menu/ProjectMenu';
import LayoutMenuItem from './LayoutMenuItem';

const LayoutMenu: FC = () => {
  
  return (
    <>
      <List disablePadding component="nav">
        <LayoutMenuItem icon={SettingsRemoteIcon} label="Dashboard" to={`/dashboard`} />
        <LayoutMenuItem icon={WifiIcon} label="WiFi Connection" to="/wifi" />
        <LayoutMenuItem icon={SettingsInputAntennaIcon} label="Access Point" to="/ap" />
        <LayoutMenuItem icon={SettingsIcon} label="System" to="/system" />
      </List>
    </>
  );
};

export default LayoutMenu;
