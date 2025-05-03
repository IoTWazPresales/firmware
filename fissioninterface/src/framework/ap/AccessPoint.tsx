import React, { FC, useContext } from 'react';
import { Navigate, Routes, Route } from 'react-router-dom';

import { Tab } from '@mui/material';

import { RouterTabs, useLayoutTitle, useRouterTab } from '../../components';


import APStatusForm from './APStatusForm';
import APSettingsForm from './APSettingsForm';

const AccessPoint: FC = () => {
  useLayoutTitle("Access Point");

  const { routerTab } = useRouterTab();

  return (
    <>
      <RouterTabs value={routerTab}>
        <Tab value="status" label="Access Point Status" />
        <Tab value="settings" label="Access Point Settings" />
      </RouterTabs>
      <Routes>
        <Route path="status" element={<APStatusForm />} />
        <Route
          path="settings"
          element={
            
              <APSettingsForm />
          
          }
        />
        <Route path="/*" element={<Navigate replace to="status" />} />
      </Routes>
    </>
  );

};

export default AccessPoint;
