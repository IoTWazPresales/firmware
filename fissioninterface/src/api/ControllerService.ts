// src/api/ControllerService.ts

import axios from 'axios';
import { API_CONFIG } from './apiConfig';

export interface RelayConfig {
  id: string;
  pin: number;
  parameter: string;
}

export interface Thresholds {
  [key: string]: { min: number; max: number };
}

export interface DeviceStates {
  pumpState: boolean;
  intakeFanState: boolean;
  extractorFanState: boolean;
  lightsState: boolean;
}
export interface SaveRelay {
  id: string;
  pin: string;       // ← number now
  parameter: string;
  min: number;
  max: number;
}

// Fetch merged relays+thresholds
export const fetchRelayConfig = async (): Promise<{
  relays: Array<RelayConfig & { min: number; max: number }>
}> => {
  const resp = await axios.get(API_CONFIG.CONTROLLER.GET_RELAY_CONFIG);
  return resp.data as { relays: Array<RelayConfig & { min: number; max: number }> };
};

// (Legacy) fetch just from relays.json
export const updateRelayConfig = async (config: { relays: RelayConfig[] }) => {
  const resp = await axios.post(API_CONFIG.CONTROLLER.SET_RELAY_CONFIG, config);
  return resp.data as { status: string; message?: string };
};

export const fetchThresholds = async (): Promise<Thresholds> => {
  const resp = await axios.get(API_CONFIG.CONTROLLER.GET_THRESHOLDS);
  return resp.data as Thresholds;
};

export const setThresholds = async (thresholds: Thresholds) => {
  const resp = await axios.post(API_CONFIG.CONTROLLER.SET_THRESHOLDS, thresholds);
  return resp.data as { status: string; message?: string };
};

export const getDeviceStates = async (): Promise<DeviceStates> => {
  const resp = await axios.get(API_CONFIG.CONTROLLER.DATA);
  return resp.data as DeviceStates;
};


export const saveConfig = async (config: {
  relays: SaveRelay[];
}): Promise<{ status: string; message?: string }> => {
  const resp = await axios.post(API_CONFIG.CONTROLLER.SAVE_CONFIG, config);
  return resp.data as { status: string; message?: string };
};

export const fetchSensorParameters = async (): Promise<string[]> => {
  const resp = await axios.get(API_CONFIG.CONTROLLER.SENSORS);
  return resp.data.parameters as string[];
};
