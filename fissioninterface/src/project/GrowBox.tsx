
import React, { FC } from 'react';
import { Navigate, Route, Routes } from 'react-router-dom';

import { Tab } from '@mui/material';

import { RouterTabs, useRouterTab, useLayoutTitle } from '../components';


import Dashboard from './Dashboard';

const GrowBox: FC = () => {
  useLayoutTitle("GrowBox");
  const { routerTab } = useRouterTab();

  return (
    <>
      <RouterTabs value={routerTab}>
        <Tab value="dashboard" label="Dashboard" />
        <Tab value="information" label="Schedules" />
        <Tab value="rest" label="Presets" />
        <Tab value="socket" label="History" />
        <Tab value="mqtt" label="Settings" />
      </RouterTabs>
      <Routes>
        <Route path="dashboard" element={<Dashboard />} />
        <Route path="/*" element={<Navigate replace to="dashboard" />} />
      </Routes>
    </>
  );
};

export default GrowBox;
