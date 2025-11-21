import React, { createContext, useContext, useState, useEffect, FC, ReactNode } from 'react';
import axiosInstance from '../api/axiosInstance';
import { API_CONFIG } from '../api/apiConfig';

interface ConnectionContextType {
  isConnected: boolean;
  isChecking: boolean;
  lastChecked: Date | null;
  checkConnection: () => Promise<void>;
}

const ConnectionContext = createContext<ConnectionContextType | undefined>(undefined);

export const useConnectionStatus = () => {
  const context = useContext(ConnectionContext);
  if (!context) {
    throw new Error('useConnectionStatus must be used within ConnectionProvider');
  }
  return context;
};

interface ConnectionProviderProps {
  children: ReactNode;
}

export const ConnectionProvider: FC<ConnectionProviderProps> = ({ children }) => {
  const [isConnected, setIsConnected] = useState<boolean>(false);
  const [isChecking, setIsChecking] = useState<boolean>(false);
  const [lastChecked, setLastChecked] = useState<Date | null>(null);

  const checkConnection = async () => {
    setIsChecking(true);
    try {
      // Try to reach a lightweight endpoint (system status is usually fast)
      await axiosInstance.get(API_CONFIG.SYSTEM.STATUS, { timeout: 3000 });
      setIsConnected(true);
      setLastChecked(new Date());
    } catch (error: any) {
      setIsConnected(false);
      setLastChecked(new Date());
    } finally {
      setIsChecking(false);
    }
  };

  // Check connection on mount and periodically
  useEffect(() => {
    checkConnection();
    const interval = setInterval(checkConnection, 30000); // Check every 30 seconds
    return () => clearInterval(interval);
  }, []);

  return (
    <ConnectionContext.Provider value={{ isConnected, isChecking, lastChecked, checkConnection }}>
      {children}
    </ConnectionContext.Provider>
  );
};

