export interface Preset {
  id: string;
  name: string;
  description?: string;
  thresholds: Record<string, { min: number; max: number }>;
  createdAt: number;
}

const PRESET_STORAGE_KEY = 'sensor_presets';

export const PresetManager = {
  savePreset: (preset: Omit<Preset, 'id' | 'createdAt'>): Preset => {
    const presets = PresetManager.loadAllPresets();
    const newPreset: Preset = {
      ...preset,
      id: `preset_${Date.now()}`,
      createdAt: Date.now(),
    };
    presets.push(newPreset);
    localStorage.setItem(PRESET_STORAGE_KEY, JSON.stringify(presets));
    return newPreset;
  },

  loadAllPresets: (): Preset[] => {
    try {
      const stored = localStorage.getItem(PRESET_STORAGE_KEY);
      return stored ? JSON.parse(stored) : [];
    } catch {
      return [];
    }
  },

  loadPreset: (id: string): Preset | null => {
    const presets = PresetManager.loadAllPresets();
    return presets.find(p => p.id === id) || null;
  },

  deletePreset: (id: string): boolean => {
    const presets = PresetManager.loadAllPresets();
    const filtered = presets.filter(p => p.id !== id);
    localStorage.setItem(PRESET_STORAGE_KEY, JSON.stringify(filtered));
    return filtered.length < presets.length;
  },

  exportPresets: (): string => {
    return JSON.stringify(PresetManager.loadAllPresets(), null, 2);
  },

  importPresets: (json: string): { success: boolean; count: number; error?: string } => {
    try {
      const presets = JSON.parse(json) as Preset[];
      if (!Array.isArray(presets)) {
        return { success: false, count: 0, error: 'Invalid preset format' };
      }
      localStorage.setItem(PRESET_STORAGE_KEY, JSON.stringify(presets));
      return { success: true, count: presets.length };
    } catch (error) {
      return { success: false, count: 0, error: error instanceof Error ? error.message : 'Parse error' };
    }
  },
};

