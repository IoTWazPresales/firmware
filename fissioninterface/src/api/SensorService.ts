import axios from 'axios';
import { API_CONFIG } from './apiConfig';
import { SensorData } from '../types/sensors';

class SensorService {
    // Method to fetch sensor data
    public async getSensorData(): Promise<SensorData> {
        try {
            const response = await axios.get<SensorData>(API_CONFIG.SENSOR.DATA);
            return response.data;
        } catch (error) {
            console.error('Error fetching sensor data:', error);
            throw error;
        }
    }
}

export default new SensorService();