// src/components/LayoutAppBar.tsx

import React, { FC, useState, MouseEvent } from 'react';
import {
  AppBar,
  Box,
  IconButton,
  Toolbar,
  Typography,
  Button,
  Menu,
  MenuItem,
  Tooltip,
} from '@mui/material';
import MenuIcon from '@mui/icons-material/Menu';
import Brightness4Icon from '@mui/icons-material/Brightness4';
import Brightness7Icon from '@mui/icons-material/Brightness7';
import DashboardIcon from '@mui/icons-material/Dashboard';
import { useThemeMode } from '../../contexts/ThemeContext';

export const DRAWER_WIDTH = 280;

interface LayoutAppBarProps {
  title: string;
  onToggleDrawer: () => void;
  onConnect?: () => void;
  onDisconnect?: () => void;
  isConnected?: boolean;
}

const LayoutAppBar: FC<LayoutAppBarProps> = ({
  title,
  onToggleDrawer,
  onConnect,
  onDisconnect,
  isConnected = false,
}) => {
  const [anchorEl, setAnchorEl] = useState<null | HTMLElement>(null);
  const { themeMode, toggleTheme } = useThemeMode();

  const handleMenuOpen = (e: MouseEvent<HTMLElement>) => {
    setAnchorEl(e.currentTarget);
  };
  const handleMenuClose = () => {
    setAnchorEl(null);
  };

  const handleClickConnect = () => {
    handleMenuClose();
    onConnect && onConnect();
  };
  const handleClickDisconnect = () => {
    handleMenuClose();
    onDisconnect && onDisconnect();
  };

  return (
    <AppBar
      position="fixed"
      sx={{
        width: { md: `calc(100% - ${DRAWER_WIDTH}px)` },
        ml:    { md: `${DRAWER_WIDTH}px` },
        boxShadow: '0 2px 10px rgba(0, 212, 255, 0.2)',
        borderBottom: '1px solid rgba(0, 212, 255, 0.3)',
        backgroundColor: 'rgba(15, 23, 42, 0.95)',
        backdropFilter: 'blur(10px)',
      }}
    >
      <Toolbar>
        <IconButton
          color="inherit"
          aria-label="open drawer"
          edge="start"
          onClick={onToggleDrawer}
          sx={{ 
            mr: 2, 
            display: { md: 'none' },
            '&:hover': {
              color: '#00D4FF',
              boxShadow: '0 0 10px rgba(0, 212, 255, 0.4)',
            },
            transition: 'all 0.3s ease',
          }}
        >
          <MenuIcon />
        </IconButton>

        <Typography 
          variant="h6" 
          noWrap 
          component="div"
          sx={{
            color: '#00D4FF',
            fontWeight: 700,
            textShadow: '0 0 10px rgba(0, 212, 255, 0.5)',
          }}
        >
          {title}
        </Typography>

        <Box flexGrow={1} />

        {/* Theme Toggle */}
        <Tooltip title={`Switch theme (Current: ${themeMode})`}>
          <IconButton 
            color="inherit" 
            onClick={toggleTheme} 
            sx={{ 
              mr: 1,
              '&:hover': {
                color: '#00D4FF',
                boxShadow: '0 0 15px rgba(0, 212, 255, 0.5)',
                backgroundColor: 'rgba(0, 212, 255, 0.1)',
              },
              transition: 'all 0.3s ease',
            }}
          >
            {themeMode === 'dashboard' ? <DashboardIcon /> : themeMode === 'dark' ? <Brightness7Icon /> : <Brightness4Icon />}
          </IconButton>
        </Tooltip>

        {/* ← now a menu */}
        <Button
          color="inherit"
          onClick={handleMenuOpen}
          sx={{ 
            textTransform: 'none',
            border: '1px solid rgba(0, 212, 255, 0.3)',
            borderRadius: '8px',
            px: 2,
            '&:hover': {
              borderColor: '#00D4FF',
              backgroundColor: 'rgba(0, 212, 255, 0.1)',
              boxShadow: '0 0 15px rgba(0, 212, 255, 0.3)',
              color: '#00D4FF',
            },
            transition: 'all 0.3s ease',
          }}
        >
          {isConnected ? 'Connected' : 'Connect to App'}
        </Button>

        <Menu
          anchorEl={anchorEl}
          open={Boolean(anchorEl)}
          onClose={handleMenuClose}
        >
          {!isConnected && onConnect && (
            <MenuItem onClick={handleClickConnect}>
              Connect to App
            </MenuItem>
          )}
          {isConnected && (
            <MenuItem disabled>
              Connected
            </MenuItem>
          )}
          {isConnected && onDisconnect && (
            <MenuItem onClick={handleClickDisconnect}>
              Disconnect
            </MenuItem>
          )}
        </Menu>
      </Toolbar>
    </AppBar>
  );
};

export default LayoutAppBar;
