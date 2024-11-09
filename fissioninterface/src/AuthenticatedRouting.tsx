import { FC, useCallback, useContext, useEffect } from 'react';
import { Navigate, Routes, Route, useNavigate, useLocation } from 'react-router-dom';
import { AxiosError } from 'axios';
import { Outlet } from 'react-router-dom';

import { FeaturesContext } from './contexts/features';
import { PROJECT_PATH } from './api/env';
import { AXIOS } from './api/endpoints';
import { Layout} from './components';

import ProjectRouting from './project/ProjectRouting';

import WiFiConnection from './framework/wifi/WiFiConnection';
import AccessPoint from './framework/ap/AccessPoint';
import NetworkTime from './framework/ntp/NetworkTime';
import System from './framework/system/System';
import { Dashboard } from '@mui/icons-material';
//import Security from './framework/security/Security';

const AuthenticatedRouting: FC = () => {
  const { features } = useContext(FeaturesContext);
  const location = useLocation();
  const navigate = useNavigate();

 

  return (
    <Layout>
      <Routes>
      <Route path="/Dashboard/*" element={<Dashboard />} />
        <Route path="/wifi/*" element={<WiFiConnection />} />
        <Route path="/ap/*" element={<AccessPoint />} />
         {features.ntp && (
          <Route path="/ntp/*" element={<NetworkTime />} />
        )}
        
        <Route path="/system/*" element={<System />} />
        <Route path="/*" element={<Navigate to="/" />} />
      </Routes>
      <Outlet />
    </Layout>
  );
};

export default AuthenticatedRouting;
