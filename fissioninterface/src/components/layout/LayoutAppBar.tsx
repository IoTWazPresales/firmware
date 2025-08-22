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
} from '@mui/material';
import MenuIcon from '@mui/icons-material/Menu';

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
        boxShadow: 'none',
      }}
    >
      <Toolbar>
        <IconButton
          color="inherit"
          aria-label="open drawer"
          edge="start"
          onClick={onToggleDrawer}
          sx={{ mr: 2, display: { md: 'none' } }}
        >
          <MenuIcon />
        </IconButton>

        <Typography variant="h6" noWrap component="div">
          {title}
        </Typography>

        <Box flexGrow={1} />

        {/* ← now a menu */}
        <Button
          color="inherit"
          onClick={handleMenuOpen}
          sx={{ textTransform: 'none' }}
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
