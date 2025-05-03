import { FC, useContext, useEffect } from 'react';
import { Navigate, Routes, Route, useLocation } from 'react-router-dom';
import { useSnackbar, VariantType } from 'notistack';
import  Dashboard  from '../src/project/Dashboard';
import ControlPanel from '../src/project/ControlPanel'; 
import Devices from '../src/project/Devices'; 
import WiFiConnection from './framework/wifi/WiFiConnection';
import AccessPoint from './framework/ap/AccessPoint';
import NetworkTime from './framework/ntp/NetworkTime';
import System from './framework/system/System';
import { Outlet } from 'react-router-dom';
import { Layout} from './components';

interface SecurityRedirectProps {
  message: string;
  variant?: VariantType;
  signOut?: boolean;
}

const RootRedirect: FC<SecurityRedirectProps> = ({ message, variant, signOut }) => {
  
  const { enqueueSnackbar } = useSnackbar();
  useEffect(() => {
  
    enqueueSnackbar(message, { variant });
  }, [message, variant,enqueueSnackbar]);
  return (<Navigate to="/" />);
};

export const RemoveTrailingSlashes = () => {
  const location = useLocation();
  return location.pathname.match('/.*/$') && (
    <Navigate
      to={{
        pathname: location.pathname.replace(/\/+$/, ""),
        search: location.search
      }}
    />
  );
};

const AppRouting: FC = () => {


  return (
      
      <Layout>
        <RemoveTrailingSlashes />
      <Routes>
      <Route path="/" element={<Navigate to="/Dashboard" />} />
      <Route path="/Dashboard/*" element={<Dashboard />} />
      <Route path="/Controlpanel/*" element={<ControlPanel />} />
        <Route path="/wifi/*" element={<WiFiConnection />} />
        <Route path="/ap/*" element={<AccessPoint />} />
        <Route path="/ntp/*" element={<NetworkTime />} />
        <Route path="/system/*" element={<System />} />
        <Route path="/Devices/*" element={<Devices />} />
        <Route path="/*" element={<Navigate to="/" />} />
        <Route path="*" element={<Navigate to="/Dashboard" />} />
      </Routes>
      <Outlet />
    </Layout>
  );
};

export default AppRouting;
