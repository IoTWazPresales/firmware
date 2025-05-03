import { FC, ReactNode } from "react";
import { CssBaseline, ThemeOptions } from "@mui/material";
import { createTheme, responsiveFontSizes, ThemeProvider } from "@mui/material/styles";
import { indigo, orange, red, green, grey, blue, cyan, purple } from "@mui/material/colors";

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
      main: "#6366f1", // Neon blue
      light: "#818cf8",
      dark: "#4f46e5",
      contrastText: "#fff",
    },
    secondary: {
      main: "#06b6d4", // Neon cyan
      light: "#67e8f9",
      dark: "#0891b2",
      contrastText: "#fff",
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
      main: "#00FFFF", // Green color for sparklines
      highlight: "#FFFF00", // Yellow for highlighted spots
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
          background: "linear-gradient(45deg, #6366f1, #06b6d4)",
          "&:hover": {
            background: "linear-gradient(45deg, #4f46e5, #0891b2)",
          },
        },
      },
    },
    MuiSlider: {
      styleOverrides: {
        root: {
          color: "#6366f1",
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

// Theme Provider Component
const CustomTheme: FC<{ children: ReactNode; themeMode?: "light" | "dark" | "dashboard" }> = ({
  children,
  themeMode = "dashboard",
}) => {
  const theme = responsiveFontSizes(themes[themeMode]);
  return (
    <ThemeProvider theme={theme}>
      <CssBaseline />
      {children}
    </ThemeProvider>
  );
};

export default CustomTheme;
