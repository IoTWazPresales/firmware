
import axios from 'axios';

export interface ControllerData {
    
    pumpState: boolean;
}

class ControllerService {
    private apiUrl: string;

    constructor(apiUrl: string) {
        this.apiUrl = apiUrl;
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

export default new ControllerService('http://192.168.101.206/api/relay');
