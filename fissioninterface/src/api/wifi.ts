import { WiFiNetworkList, WiFiSettings, WiFiStatus } from '../types';
import axios from 'axios';

class WiFiService {
  private apiUrl: string;

  constructor(apiUrl: string) {
    this.apiUrl = apiUrl;
  }

  // Method to fetch WiFi status
  public async readWiFiStatus(): Promise<WiFiStatus> {
    try {
      const response = await axios.get<WiFiStatus>(`${this.apiUrl}/wifiStatus`);
      return response.data;
    } catch (error) {
      console.error('Error fetching WiFi status:', error);
      throw error; // Handle error as needed
    }
  }

  // Method to scan networks
  public async scanNetworks(): Promise<void> {
    try {
      await axios.get(`${this.apiUrl}/scanNetworks`);
    } catch (error) {
      console.error('Error scanning networks:', error);
      throw error; // Handle error as needed
    }
  }

  // Method to list available networks
  public async listNetworks(): Promise<WiFiNetworkList> {
    try {
      const response = await axios.get<WiFiNetworkList>(`${this.apiUrl}/listNetworks`);
      return response.data;
    } catch (error) {
      console.error('Error listing networks:', error);
      throw error; // Handle error as needed
    }
  }

  // Method to fetch WiFi settings
  public async readWiFiSettings(): Promise<WiFiSettings> {
    try {
      const response = await axios.get<WiFiSettings>(`${this.apiUrl}/wifiSettings`);
      return response.data;
    } catch (error) {
      console.error('Error fetching WiFi settings:', error);
      throw error; // Handle error as needed
    }
  }

  // Method to update WiFi settings
  public async updateWiFiSettings(wifiSettings: WiFiSettings): Promise<WiFiSettings> {
    try {
      const response = await axios.post<WiFiSettings>(`${this.apiUrl}/wifiSettings`, wifiSettings);
      return response.data;
    } catch (error) {
      console.error('Error updating WiFi settings:', error);
      throw error; // Handle error as needed
    }
  }
}

export default new WiFiService('http://192.168.101.209/api');
