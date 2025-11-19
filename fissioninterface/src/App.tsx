import React, { FC, useState, useEffect, RefObject } from 'react';
import { SnackbarProvider } from 'notistack';
import { IconButton } from '@mui/material';
import CloseIcon from '@mui/icons-material/Close';
import { FeaturesLoader } from './contexts/features';
import { ThemeProvider } from './contexts/ThemeContext';
import CustomTheme from './CustomTheme';
import AppRouting from './AppRouting';
import { LoadingSpinner } from './components';

const App: FC = () => {
  const [isAppReady, setIsAppReady] = useState(false); // App initialization state
  const notistackRef: RefObject<any> = React.createRef();

  const onClickDismiss = (key: string | number | undefined) => () => {
    notistackRef.current?.closeSnackbar(key);
  };

  // Simulate app initialization (e.g., fetching config, user auth, etc.)
  useEffect(() => {
    const initializeApp = async () => {
      try {
        // Simulated initialization delay
        await new Promise((resolve) => setTimeout(resolve, 1000));
        setIsAppReady(true);
      } catch (error) {
        console.error('Error initializing app:', error);
      }
    };

    initializeApp();
  }, []);

  return (
    <ThemeProvider>
      <CustomTheme>
        <SnackbarProvider
          maxSnack={3}
          anchorOrigin={{ vertical: 'bottom', horizontal: 'left' }}
          ref={notistackRef}
          action={(key) => (
            <IconButton onClick={onClickDismiss(key)} size="small">
              <CloseIcon />
            </IconButton>
          )}
        >
          {/* Show Loading Spinner until app is ready */}
          {!isAppReady ? <LoadingSpinner /> : <AppRouting />}
        </SnackbarProvider>
      </CustomTheme>
    </ThemeProvider>
  );
};

export default App;
