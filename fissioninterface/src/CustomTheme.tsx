import { FC, ReactNode } from 'react';

import { CssBaseline, ThemeOptions } from '@mui/material';
import { createTheme, responsiveFontSizes, ThemeProvider } from '@mui/material/styles';
import { indigo, orange, red, green, grey } from '@mui/material/colors';

const lightTheme: ThemeOptions = {
  palette: {
    // mode: "dark",
    text: {
      primary: '#333',
      secondary: grey[500],
    },
    background: {
      default: "#eeeeee"
    },
    primary: {
      light: '#5ecc97',
      main: '#2c9c69',
      dark: '#006f3e',
      contrastText: '#fff',
    },
    secondary: {
      light: '#60f293',
      main: '#25c065',
      dark: '#00903a',
      contrastText: '#000',
    },
    info: {
      main: indigo[500]
    },
    warning: {
      main: orange[500]
    },
    error: {
      main: red[500]
    },
    success: {
      main: green[500]
    }
  }
};

const darkTheme: ThemeOptions = {
  palette: {
    mode: "dark",
    text: {
      primary: '#fff',
      secondary: grey[500],
    },
    background: {
      default: "#353535"
    },
    primary: {
      light: '#5ecc97',
      main: '#2c9c69',
      dark: '#006f3e',
      contrastText: '#fff',
    },
    secondary: {
      light: '#60f293',
      main: '#25c065',
      dark: '#00903a',
      contrastText: '#000',
    },
    info: {
      main: indigo[500]
    },
    warning: {
      main: orange[500]
    },
    error: {
      main: red[500]
    },
    success: {
      main: green[500]
    }
  }
};

const theme = responsiveFontSizes(
  createTheme(darkTheme as any)
);

const CustomTheme: FC<{ children: ReactNode }> = ({ children }) => (
  <ThemeProvider theme={theme}>
    <CssBaseline />
    {children}
  </ThemeProvider>
);

export default CustomTheme;
