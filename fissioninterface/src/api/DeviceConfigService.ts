import axios from 'axios';
import { API_CONFIG } from './apiConfig';

// Interface for assignment data (optional, for type safety)
interface SensorAssignment {
  pin: string;
  sensorType: string;
}

// Fetch saved sensor assignments
export const fetchSensorAssignments = async (): Promise<{ [key: string]: string }> => {
  try {
    const response = await axios.get(API_CONFIG.SCANNER.CONFIG);
    console.log('Fetched assignments:', response.data);
    return response.data;
  } catch (error) {
    console.error('Error fetching sensor assignments:', error);
    throw error;
  }
};

// Save a sensor assignment
export const saveSensorAssignment = async (assignment: SensorAssignment): Promise<void> => {
  try {
    const response = await axios.post(API_CONFIG.SCANNER.CONFIGURE, assignment);
    console.log('Saved assignment:', response.data);
  } catch (error) {
    console.error('Error saving sensor assignment:', error);
    throw error;
  }
};