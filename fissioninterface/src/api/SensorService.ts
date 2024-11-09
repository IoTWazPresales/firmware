// SensorService.ts
/*import axios from 'axios';


class SensorService {
  private baseUrl: string;
  private axiosInstance: any;

  constructor(baseUrl: string) {
    this.baseUrl = baseUrl;
    this.axiosInstance = axios.create({
      baseURL: this.baseUrl,
      timeout: 5000, // Set a timeout of 5 seconds
    });
  }

  async getTemperature(): Promise<string> {
    try {
      const response = await this.axiosInstance.get('/api/temperature');
      return response.data.watertemperature;
    } catch (error) {
      this.handleError(error, 'Temperature');
      return 'Failed to connect to ESP32';
    }
  }

  async getPH(): Promise<string> {
    try {
      const response = await this.axiosInstance.get('/api/ph');
      return response.data.ph;
    } catch (error) {
      this.handleError(error, 'pH');
      return 'Failed to connect to ESP32';
    }
  }

  private handleError(error: any, sensorType: string) {
    console.error(`Error fetching ${sensorType} data:`, error);
    if (error.response) {
      console.error('Server responded with:', error.response.data);
    } else if (error.request) {
      console.error('Request was made but no response received:', error.request);
    } else {
      console.error('Error setting up the request:', error.message);
    }
  }

  // Add more methods for additional sensors as needed
}

export default SensorService;
*/


// SensorService.ts


import axios from 'axios';

export interface SensorData {
    temperature: number;
    ph: number;
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

export default new SensorService('http://192.168.101.206/api/sensor');