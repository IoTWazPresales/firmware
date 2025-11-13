import axios from 'axios';
import { API_CONFIG } from './apiConfig';
import { SensorResponse, SensorValues } from '../types/sensors';

class SensorService {
    // Method to fetch sensor data
    public async getSensorData(): Promise<SensorResponse> {
        try {
            const response = await axios.get(API_CONFIG.SENSOR.DATA);
            const rawData = response.data;

            if (rawData && typeof rawData === 'object' && 'values' in rawData && 'meta' in rawData) {
                return rawData as SensorResponse;
            }

            const values: SensorValues = rawData ?? {};
            return {
                values,
                meta: [],
                ...(typeof rawData === 'object' && rawData !== null ? rawData : {})
            };
        } catch (error) {
            console.error('Error fetching sensor data:', error);
            if (error instanceof Error) {
                throw new Error(`Network error: ${error.message}`);
            }
            throw error;
        }
    }
}

export default new SensorService();