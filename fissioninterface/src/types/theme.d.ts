import "@mui/material/styles";

// Extend the Material-UI Palette interface
declare module "@mui/material/styles" {
  interface Palette {
    sparklines: {
      main: string;
      highlight: string;
    };
  }
  interface PaletteOptions {
    sparklines?: {
      main?: string;
      highlight?: string;
    };
  }
}
