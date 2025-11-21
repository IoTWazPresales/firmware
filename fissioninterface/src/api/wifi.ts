import axiosInstance from './axiosInstance';
import { API_CONFIG } from './apiConfig';
import { WiFiNetworkList, WiFiSettings, WiFiStatus } from '../types';

class WiFiService {
    // Method to fetch WiFi status
    public async readWiFiStatus(): Promise<WiFiStatus> {
        try {
            const response = await axiosInstance.get<WiFiStatus>(API_CONFIG.WIFI.STATUS);
            return response.data;
        } catch (error) {
            console.error('Error fetching WiFi status:', error);
            throw error;
        }
    }

    // Method to scan networks
    public async scanNetworks(): Promise<void> {
        try {
            await axiosInstance.get(API_CONFIG.WIFI.SCAN);
        } catch (error) {
            console.error('Error scanning networks:', error);
            throw error;
        }
    }

    // Method to list available networks
    public async listNetworks(): Promise<WiFiNetworkList> {
        try {
            const response = await axiosInstance.get<WiFiNetworkList>(API_CONFIG.WIFI.LIST);
            return response.data;
        } catch (error) {
            console.error('Error listing networks:', error);
            throw error;
        }
    }

    // Method to fetch WiFi settings
    public async readWiFiSettings(): Promise<WiFiSettings> {
        try {
            const response = await axiosInstance.get<WiFiSettings>(API_CONFIG.WIFI.SETTINGS);
            return response.data;
        } catch (error) {
            console.error('Error fetching WiFi settings:', error);
            throw error;
        }
    }

    // Method to update WiFi settings
    public async updateWiFiSettings(wifiSettings: WiFiSettings): Promise<WiFiSettings> {
        try {
            const response = await axiosInstance.post<WiFiSettings>(API_CONFIG.WIFI.SETTINGS, wifiSettings);
            return response.data;
        } catch (error) {
            console.error('Error updating WiFi settings:', error);
            throw error;
        }
    }
}

export default new WiFiService();
