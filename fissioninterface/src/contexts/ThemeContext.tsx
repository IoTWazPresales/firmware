import React, { createContext, useContext, useState, useEffect, FC, ReactNode } from 'react';

export type ThemeMode = 'light' | 'dark' | 'dashboard';

interface ThemeContextType {
  themeMode: ThemeMode;
  setThemeMode: (mode: ThemeMode) => void;
  toggleTheme: () => void;
}

const ThemeContext = createContext<ThemeContextType | undefined>(undefined);

export const useThemeMode = () => {
  const context = useContext(ThemeContext);
  if (!context) {
    throw new Error('useThemeMode must be used within ThemeProvider');
  }
  return context;
};

export const useOptionalThemeMode = () => useContext(ThemeContext);

interface ThemeProviderProps {
  children: ReactNode;
}

const THEME_STORAGE_KEY = 'themeMode';

const readStoredTheme = (): ThemeMode => {
  if (typeof window === 'undefined') {
    return 'dashboard';
  }
  try {
    const saved = window.localStorage.getItem(THEME_STORAGE_KEY);
    if (saved === 'light' || saved === 'dark' || saved === 'dashboard') {
      return saved;
    }
  } catch {
    // ignore - fall back to default
  }
  return 'dashboard';
};

export const ThemeProvider: FC<ThemeProviderProps> = ({ children }) => {
  const [themeMode, setThemeModeState] = useState<ThemeMode>(() => readStoredTheme());

  useEffect(() => {
    try {
      window.localStorage.setItem(THEME_STORAGE_KEY, themeMode);
    } catch {
      // ignore write failures (e.g., Safari private mode)
    }
  }, [themeMode]);

  const setThemeMode = (mode: ThemeMode) => {
    setThemeModeState(mode);
  };

  const toggleTheme = () => {
    if (themeMode === 'dashboard') {
      setThemeMode('light');
    } else if (themeMode === 'light') {
      setThemeMode('dark');
    } else {
      setThemeMode('dashboard');
    }
  };

  return (
    <ThemeContext.Provider value={{ themeMode, setThemeMode, toggleTheme }}>
      {children}
    </ThemeContext.Provider>
  );
};

