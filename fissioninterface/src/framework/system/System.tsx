import React, { FC, useContext } from 'react';
import { Navigate, Routes, Route } from 'react-router-dom';

import { Tab } from '@mui/material';
import { FeaturesContext } from '../../contexts/features';
import { useRouterTab, RouterTabs, useLayoutTitle } from '../../components';
import SystemStatusForm from './SystemStatusForm';
import OTASettingsForm from './OTASettingsForm';

const System: FC = () => {
  useLayoutTitle("System");


  const { features } = useContext(FeaturesContext);
  const { routerTab } = useRouterTab();

  return (
    <>
      <RouterTabs value={routerTab}>
        <Tab value="status" label="System Status" />
        {features.ota && (
          <Tab value="ota" label="OTA Settings"  />
        )}
        {features.upload_firmware && (
          <Tab value="upload" label="Upload Firmware"  />
        )}
      </RouterTabs>
      <Routes>
        <Route path="status" element={<SystemStatusForm />} />
        {features.ota && (
          <Route
            path="ota"
            element={
            
                <OTASettingsForm />
             
            }
          />
        )}
        <Route path="/*" element={<Navigate replace to="status" />} />
      </Routes>
    </>
  );

};

export default System;
