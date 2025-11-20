
import { FC } from 'react';

import { Box, Divider, Drawer, Toolbar, Typography } from '@mui/material';

import LayoutMenu from './LayoutMenu';
import { DRAWER_WIDTH } from './Layout';

interface LayoutDrawerProps {
  mobileOpen: boolean;
  onClose: () => void;
}

const LayoutDrawer: FC<LayoutDrawerProps> = ({ mobileOpen, onClose }) => {

  const drawer = (
    <>
      <Toolbar disableGutters>
        <Box display="flex" alignItems="center" px={2}>
          {/* <LayoutDrawerLogo src="/app/icon.png" alt={PROJECT_NAME} /> */}
          <Typography 
            variant="h6" 
            sx={{
              color: '#00D4FF',
              fontWeight: 700,
              textShadow: '0 0 10px rgba(0, 212, 255, 0.5)',
            }}
          >
            Command Centre
          </Typography>
        </Box>
        <Divider absolute />
      </Toolbar>
      <Divider />
      <LayoutMenu />
      <Box
        sx={{
          display: 'flex',
          flexDirection: 'column',
          alignItems: 'center',
          justifyContent: 'flex-end',
          flexGrow: 1,
          pb: 2,
          pt: 2,
        }}
      >
        <img 
          style={{ height: 120, maxWidth: '80%', objectFit: 'contain' }} 
          src="/media/FissionLogo_SkyBlue.svg" 
          alt="Fission Logo" 
        />
      </Box>

    </>
  );

  return (
    <Box
      component="nav"
      sx={{ width: { md: DRAWER_WIDTH }, flexShrink: { md: 0 } }}
    >
      <Drawer
        variant="temporary"
        open={mobileOpen}
        onClose={onClose}
        ModalProps={{
          keepMounted: true, // Better open performance on mobile.
        }}
        sx={{
          display: { xs: 'block', md: 'none' },
          '& .MuiDrawer-paper': { boxSizing: 'border-box', width: DRAWER_WIDTH },
        }}
      >
        {drawer}
      </Drawer>
      <Drawer
        variant="permanent"
        sx={{
          display: { xs: 'none', md: 'block' },
          '& .MuiDrawer-paper': { boxSizing: 'border-box', width: DRAWER_WIDTH },
        }}
        open
      >
        {drawer}
      </Drawer>
    </Box>
  );

};

export default LayoutDrawer;
