import React, { FC } from 'react';
import { Chip, Tooltip, Box } from '@mui/material';
import { useConnectionStatus } from '../contexts/ConnectionContext';
import CheckCircleIcon from '@mui/icons-material/CheckCircle';
import ErrorIcon from '@mui/icons-material/Error';
import RefreshIcon from '@mui/icons-material/Refresh';
import { IconButton } from '@mui/material';

const ConnectionStatus: FC = () => {
  const { isConnected, isChecking, checkConnection } = useConnectionStatus();

  return (
    <Tooltip 
      title={
        isConnected 
          ? 'Connected to ESP32 backend' 
          : 'Backend not available. Please ensure the ESP32 is connected and running.'
      }
    >
      <Box sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
        <Chip
          icon={isConnected ? <CheckCircleIcon /> : <ErrorIcon />}
          label={isConnected ? 'Connected' : 'Disconnected'}
          color={isConnected ? 'success' : 'error'}
          size="small"
          sx={{
            ...(!isConnected && {
              animation: 'pulse 2s infinite',
              '@keyframes pulse': {
                '0%, 100%': { opacity: 1 },
                '50%': { opacity: 0.5 },
              },
            }),
          }}
        />
        {!isConnected && (
          <IconButton 
            size="small" 
            onClick={checkConnection}
            disabled={isChecking}
            sx={{ 
              color: 'error.main',
              '&:hover': { backgroundColor: 'error.light', opacity: 0.8 },
            }}
          >
            <RefreshIcon fontSize="small" />
          </IconButton>
        )}
      </Box>
    </Tooltip>
  );
};

export default ConnectionStatus;

