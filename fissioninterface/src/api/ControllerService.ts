import axios from 'axios';
import { API_CONFIG } from './apiConfig';

export interface ControllerData {
    pumpState: boolean;
    intakeFanState: boolean;
    extractorFanState: boolean;
    lightsState: boolean;
}

class ControllerService {
    public async setThresholds(
        minMoisture: number,
        maxMoisture: number,
        minCO2: number,
        maxCO2: number,
        minTemp: number,
        maxTemp: number,
        minHumi: number,
        maxHumi: number
    ): Promise<void> {
        try {
            console.log("Setting all thresholds");
            // Sending a single POST request with all thresholds data
            await axios.post(API_CONFIG.CONTROLLER.SET_THRESHOLDS, {
                minMoisture,
                maxMoisture,
                minCO2,
                maxCO2,
                minTemp,
                maxTemp,
                minHumi,
                maxHumi
            });
            console.log("All thresholds set successfully.");
        } catch (error) {
            console.error("Error setting thresholds:", error);
            throw error;
        }
    }

    // Method to get all the thresholds from the server
    public async getThresholds(): Promise<any> {
        try {
            const response = await axios.get(API_CONFIG.CONTROLLER.GET_THRESHOLDS);
            return response.data;
        } catch (error) {
            console.error("Error fetching thresholds:", error);
            throw error;
        }
    }

    public async getDeviceStates(): Promise<ControllerData> {
        try {
            console.log("Fetching controller data");
            const response = await axios.get<ControllerData>(API_CONFIG.CONTROLLER.DATA);
            return response.data;
        } catch (error) {
            console.error("Error fetching controller data:", error);
            throw error;
        }
    }
}

export default new ControllerService();
