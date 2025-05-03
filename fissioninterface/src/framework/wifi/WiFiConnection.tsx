import React, { FC, useCallback, useContext, useState } from 'react';
import { Navigate, Routes, Route, useNavigate } from 'react-router-dom';

import { Tab } from '@mui/material';

import { RouterTabs, useLayoutTitle, useRouterTab } from '../../components';
import { WiFiNetwork } from '../../types';
import { WiFiConnectionContext } from './WiFiConnectionContext';
import WiFiStatusForm from './WiFiStatusForm';
import WiFiNetworkScanner from './WiFiNetworkScanner';
import WiFiSettingsForm from './WiFiSettingsForm';

const WiFiConnection: FC = () => {
  useLayoutTitle("WiFi Connection");


  const navigate = useNavigate();
  const { routerTab } = useRouterTab();

  const [selectedNetwork, setSelectedNetwork] = useState<WiFiNetwork>();

  const selectNetwork = useCallback((network: WiFiNetwork) => {
    setSelectedNetwork(network);
    navigate('settings');
  }, [navigate]);

  const deselectNetwork = useCallback(() => {
    setSelectedNetwork(undefined);
  }, []);

  return (
    <WiFiConnectionContext.Provider
      value={{
        selectedNetwork,
        selectNetwork,
        deselectNetwork
      }}
    >
      <RouterTabs value={routerTab}>
        <Tab value="status" label="WiFi Status" />
        <Tab value="scan" label="Scan Networks"  />
        <Tab value="settings" label="WiFi Settings"  />
      </RouterTabs>
      <Routes>
        <Route path="status" element={<WiFiStatusForm />} />
        <Route
          path="scan"
          element={
       
              <WiFiNetworkScanner />
           
          }
        />
        <Route
          path="settings"
          element={
          
              <WiFiSettingsForm />
       
          }
        />
        <Route path="/*" element={<Navigate replace to="status" />} />
      </Routes>
    </WiFiConnectionContext.Provider>
  );

};

export default WiFiConnection;
