import axios from 'axios';
import { API_CONFIG } from './apiConfig';

// Fetch existing assignments: GET /api/config
export const fetchSensorAssignments = async (): Promise<{[pin:string]:string}> => {
  const resp = await axios.get<{[pin:string]:string}>(API_CONFIG.SCANNER.CONFIG);
  return resp.data;
};

// **New**: send entire map to overwrite config.json
export const saveAllSensorAssignments = async (
  assignments: { [pin: string]: string }
): Promise<void> => {
  await axios.post(API_CONFIG.SCANNER.CONFIG, assignments);
};
export const resetSensorAssignments = async (): Promise<void> => {
  await axios.post(API_CONFIG.SCANNER.RESET);
};