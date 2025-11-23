import axiosInstance from './axiosInstance';
import { API_CONFIG } from './apiConfig';

export interface SensorLogEntry {
    timestamp: string;
    temperature: number;
    ph: number;
    humidity: number;
    airtemp: number;
    moisture: number;
    tdsSens: number;
}

class SensorHistory {
    public async getSensorLogs(): Promise<SensorLogEntry[]> {
        try {
            const response = await axiosInstance.get<SensorLogEntry[]>(API_CONFIG.SENSOR.LOGS);
            console.log('Sensor Logs:', response.data);
            return response.data;
        } catch (error) {
            console.error('Error fetching sensor logs:', error);
            throw error;
        }
    }
}

const sensorHistory = new SensorHistory();

export default sensorHistory;
