

import axios from 'axios';

export interface SensorData {
    temperature: number;
    ph: number;
    humidity: number;
    airtemp: number;
    moisture: number;
    tdsSensor: number;
    realtime: string;
}

class SensorService {
    private apiUrl: string;

    constructor(apiUrl: string) {
        this.apiUrl = apiUrl;
    }


    
    // Method to fetch sensor data
    public async getSensorData(): Promise<SensorData> {
        try {
            const response = await axios.get<SensorData>(this.apiUrl);
            return response.data;
        } catch (error) {
            console.error('Error fetching sensor data:', error);
            throw error; // Handle error as needed
        }
    }
}

export default new SensorService('http://192.168.101.209/api/sensor');
