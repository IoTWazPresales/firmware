
import axios from 'axios';

export interface ControllerData {
    
    pumpState: boolean;
}

class ControllerService {
    private apiUrl: string;

    constructor(apiUrl: string) {
        this.apiUrl = apiUrl;
    }
    public async setMoistureThresholds(minMoisture: number, maxMoisture: number): Promise<void> {
        try {
            console.log("Setting moisture thresholds");
            await axios.post(`${this.apiUrl}/setThresholds`, {
                minMoisture,
                maxMoisture,
            });
            console.log("Thresholds set successfully");
        } catch (error) {
            console.error("Error setting moisture thresholds:", error);
            throw error; // Handle error as needed
        }
    }
    
    
    // Method to fetch sensor data
    public async getControllerData(): Promise<ControllerData> {
        try {
            console.log("fetching controller data");
            const response = await axios.get<ControllerData>(this.apiUrl);
            return response.data;
        } catch (error) {
            console.error('Error fetching controller data:', error);
            throw error; // Handle error as needed
        }
    }
}

export default new ControllerService('http://192.168.101.209/api/relay');
