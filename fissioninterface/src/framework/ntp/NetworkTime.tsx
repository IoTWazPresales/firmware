import React, { FC, useContext } from 'react';
import { Navigate, Route, Routes } from 'react-router-dom';

import { Tab } from '@mui/material';

import { RouterTabs, useLayoutTitle, useRouterTab } from '../../components';


import NTPStatusForm from './NTPStatusForm';
import NTPSettingsForm from './NTPSettingsForm';

const NetworkTime: FC = () => {
  useLayoutTitle("Network Time");


  const { routerTab } = useRouterTab();

  return (
    <>
      <RouterTabs value={routerTab}>
        <Tab value="status" label="NTP Status" />
        <Tab value="settings" label="NTP Settings" />
      </RouterTabs>
      <Routes>
        <Route path="status" element={<NTPStatusForm />} />
        <Route
          path="settings"
          element={
          
              <NTPSettingsForm />
            
          }
        />
        <Route path="/*" element={<Navigate replace to="status" />} />
      </Routes>
    </>
  );

};

export default NetworkTime;
