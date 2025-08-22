// src/api/DeviceScannerService.ts

import axios from 'axios';
import { API_CONFIG } from './apiConfig';

/**
 * Represents one digital GPIO pin scan result.
 */
export interface DigitalPin {
  pin:       number;      // GPIO number, e.g. 2, 4, 5…
  key:       string;      // human-readable, e.g. "D2"
  detected:  boolean;     // true if something drives the line
  value?:    0 | 1;       // when detected, the driven level
}

/**
 * Represents one analog GPIO pin scan result.
 */
export interface AnalogPin {
  pin:       number;      // GPIO number, e.g. 32, 33, 34…
  key:       string;      // e.g. "A32"
  detected:  boolean;     // true if a mid-range voltage was seen
  rawValue?: number;      // average reading, when detected
}

/**
 * Full scan payload returned by the backend.
 */
export interface ScanResults {
  i2cDevices:  string[];      // e.g. ["0x3C", "0x76"]
  digitalPins: DigitalPin[];  
  analogPins:  AnalogPin[];
}

/**
 * Fetches the latest scan results.
 */
export const fetchScannedDevices = async (): Promise<ScanResults> => {
  const resp = await axios.get<ScanResults>(API_CONFIG.SCANNER.DEVICES);
  return resp.data;
};

/**
 * Triggers a new scan, then reloads the results.
 */
export const triggerScan = async (): Promise<ScanResults> => {
  // Fire the scan
  await axios.post(API_CONFIG.SCANNER.TRIGGER);
  // Re-fetch updated results
  return fetchScannedDevices();
};
