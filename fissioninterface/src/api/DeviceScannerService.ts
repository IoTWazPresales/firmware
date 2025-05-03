import axios from 'axios';
import { API_CONFIG } from './apiConfig';

export const fetchScannedDevices = async () => {
    try {
        const response = await axios.get(API_CONFIG.SCANNER.DEVICES);
        return response.data;
    } catch (error) {
        console.error('Error fetching scanned devices:', error);
        throw error;
    }
};
