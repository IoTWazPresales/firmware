import { FC, ReactNode, useMemo } from "react";
import { CssBaseline, ThemeOptions } from "@mui/material";
import { createTheme, responsiveFontSizes, ThemeProvider as MuiThemeProvider } from "@mui/material/styles";
import { indigo, orange, red, green, grey, cyan } from "@mui/material/colors";
import { useOptionalThemeMode, ThemeMode } from "./contexts/ThemeContext";

// Light Theme
const lightTheme: ThemeOptions = {
  palette: {
    mode: "light",
    text: {
      primary: "#333",
      secondary: grey[500],
    },
    background: {
      default: "#eeeeee",
    },
    primary: {
      light: "#5ecc97",
      main: "#2c9c69",
      dark: "#006f3e",
      contrastText: "#fff",
    },
    secondary: {
      light: "#60f293",
      main: "#25c065",
      dark: "#00903a",
      contrastText: "#000",
    },
    info: {
      main: indigo[500],
    },
    warning: {
      main: orange[500],
    },
    error: {
      main: red[500],
    },
    success: {
      main: green[500],
    },
  },
};

// Dark Theme
const darkTheme: ThemeOptions = {
  palette: {
    mode: "dark",
    text: {
      primary: "#fff",
      secondary: grey[500],
    },
    background: {
      default: "#252525",
    },
    primary: {
      light: "#5ecc97",
      main: "#2c9c69",
      dark: "#006f3e",
      contrastText: "#fff",
    },
    secondary: {
      light: "#60f293",
      main: "#25c065",
      dark: "#00903a",
      contrastText: "#000",
    },
    info: {
      main: indigo[500],
    },
    warning: {
      main: orange[500],
    },
    error: {
      main: red[500],
    },
    success: {
      main: green[500],
    },
  },
};

// 🚀 Dashboard Theme (Matching the Image)
const dashboardTheme: ThemeOptions = {
  palette: {
    mode: "dark",
    text: {
      primary: "#e2e8f0", // Soft white text
      secondary: "#94a3b8", // Subtle gray text
    },
    background: {
      default: "#0f172a", // Deep dark blue background
      paper: "#1e293b", // Slightly lighter cards
    },
    primary: {
      main: "#00D4FF", // Bright neon cyan (Star Citizen style)
      light: "#00FFFF", // Brighter for hovers
      dark: "#0099CC", // Darker for pressed states
      contrastText: "#000", // Black text for better readability on bright neon
    },
    secondary: {
      main: "#00BFFF", // Slightly deeper neon cyan
      light: "#00D4FF",
      dark: "#0080CC",
      contrastText: "#000",
    },
    info: {
      main: cyan[400],
    },
    warning: {
      main: orange[400],
    },
    error: {
      main: red[400],
    },
    success: {
      main: green[400],
    },
    sparklines: {
      main: "#00D4FF", // Neon cyan for sparklines (matches primary)
      highlight: "#00FFFF", // Brighter cyan for highlighted spots
    },
  },
  typography: {
    fontFamily: "'Inter', 'Roboto', 'Helvetica', 'Arial', sans-serif",
    h6: {
      color: "#e2e8f0",
      fontWeight: 400,
    },
    body1: {
      color: "#cbd5e1",
    },
    subtitle1: {
      color: "#94a3b8",
    },
  },
  components: {
    MuiCard: {
      styleOverrides: {
        root: {
          backgroundColor: "#1e293b", // Match card background to theme
          boxShadow: "0 4px 20px rgba(0, 0, 0, 0.3)",
          borderRadius: "12px",
          padding: "16px",
        },
      },
    },
    MuiButton: {
      styleOverrides: {
        root: {
          textTransform: "none",
          borderRadius: "8px",
          fontWeight: 600,
          transition: "all 0.3s ease",
        },
        contained: {
          backgroundColor: "#00D4FF", // Solid neon cyan
          color: "#000",
          boxShadow: "0 0 15px rgba(0, 212, 255, 0.5), 0 4px 6px rgba(0, 0, 0, 0.3)",
          "&:hover": {
            backgroundColor: "#00FFFF", // Brighter on hover
            boxShadow: "0 0 25px rgba(0, 255, 255, 0.7), 0 6px 8px rgba(0, 0, 0, 0.4)",
            transform: "translateY(-1px)",
          },
          "&:active": {
            backgroundColor: "#0099CC",
            boxShadow: "0 0 10px rgba(0, 212, 255, 0.4), 0 2px 4px rgba(0, 0, 0, 0.3)",
          },
          "&.MuiButton-colorError": {
            backgroundColor: "#ef4444", // Keep red for error buttons
            color: "#fff",
            boxShadow: "0 0 15px rgba(239, 68, 68, 0.5), 0 4px 6px rgba(0, 0, 0, 0.3)",
            "&:hover": {
              backgroundColor: "#f87171",
              boxShadow: "0 0 25px rgba(239, 68, 68, 0.7), 0 6px 8px rgba(0, 0, 0, 0.4)",
            },
          },
        },
        outlined: {
          borderColor: "#00D4FF",
          color: "#00D4FF",
          "&:hover": {
            borderColor: "#00FFFF",
            backgroundColor: "rgba(0, 212, 255, 0.1)",
            boxShadow: "0 0 15px rgba(0, 212, 255, 0.3)",
          },
          "&.MuiButton-colorError": {
            borderColor: "#ef4444", // Keep red for error outlined buttons
            color: "#ef4444",
            "&:hover": {
              borderColor: "#f87171",
              backgroundColor: "rgba(239, 68, 68, 0.1)",
              boxShadow: "0 0 15px rgba(239, 68, 68, 0.3)",
            },
          },
        },
      },
    },
    MuiSlider: {
      styleOverrides: {
        root: {
          color: "#00D4FF",
        },
      },
    },
    MuiTableHead: {
      styleOverrides: {
        root: {
          "& .MuiTableRow-root": {
            backgroundColor: "#0a1a2e", // Deep dark blue background
            borderBottom: "2px solid #00D4FF", // Neon cyan border
            boxShadow: "0 0 15px rgba(0, 212, 255, 0.3) inset",
            "& .MuiTableCell-root": {
              color: "#00D4FF", // Neon cyan text
              fontWeight: 700,
              textShadow: "0 0 8px rgba(0, 212, 255, 0.5)",
            },
          },
        },
      },
    },
  },
};

// Create Themes
const themes = {
  light: createTheme(lightTheme),
  dark: createTheme(darkTheme),
  dashboard: createTheme(dashboardTheme),
};

interface CustomThemeProps {
  children: ReactNode;
  themeMode?: ThemeMode;
}

// Theme Provider Component
const CustomTheme: FC<CustomThemeProps> = ({
  children,
  themeMode: propThemeMode,
}) => {
  const context = useOptionalThemeMode();
  const resolvedThemeMode: ThemeMode = propThemeMode ?? context?.themeMode ?? "dashboard";
  const theme = useMemo(() => responsiveFontSizes(themes[resolvedThemeMode]), [resolvedThemeMode]);
  return (
    <MuiThemeProvider theme={theme}>
      <CssBaseline />
      {children}
    </MuiThemeProvider>
  );
};

export default CustomTheme;
