import { AxiosResponse } from 'axios';
import { OTASettings, SystemStatus } from '../types/system';
import { AXIOS, FileUploadConfig, uploadFile } from './endpoints';

class SystemService {
  private apiUrl: string;

  constructor(apiUrl: string) {
    this.apiUrl = apiUrl;
  }

  // Method to read system status
  public async readSystemStatus(timeout?: number): Promise<SystemStatus> {
    try {
      const response: AxiosResponse<SystemStatus> = await AXIOS.get(`${this.apiUrl}/systemStatus`, { timeout });
      return response.data;
    } catch (error) {
      console.error('Error reading system status:', error);
      throw error;
    }
  }

  // Method to restart system
  public async restart(): Promise<void> {
    try {
      await AXIOS.post(`${this.apiUrl}/restart`);
    } catch (error) {
      console.error('Error restarting system:', error);
      throw error;
    }
  }

  // Method to perform factory reset
  public async factoryReset(): Promise<void> {
    try {
      await AXIOS.post(`${this.apiUrl}/factoryReset`);
    } catch (error) {
      console.error('Error performing factory reset:', error);
      throw error;
    }
  }

  // Method to read OTA settings
  public async readOTASettings(): Promise<OTASettings> {
    try {
      const response: AxiosResponse<OTASettings> = await AXIOS.get(`${this.apiUrl}/otaSettings`);
      return response.data;
    } catch (error) {
      console.error('Error reading OTA settings:', error);
      throw error;
    }
  }

  // Method to update OTA settings
  public async updateOTASettings(otaSettings: OTASettings): Promise<OTASettings> {
    try {
      const response: AxiosResponse<OTASettings> = await AXIOS.post(`${this.apiUrl}/otaSettings`, otaSettings);
      return response.data;
    } catch (error) {
      console.error('Error updating OTA settings:', error);
      throw error;
    }
  }

  // Method to upload firmware
  public async uploadFirmware(file: File, config?: FileUploadConfig): Promise<void> {
    try {
      await uploadFile(`${this.apiUrl}/uploadFirmware`, file, config);
    } catch (error) {
      console.error('Error uploading firmware:', error);
      throw error;
    }
  }
}

// Create a default instance with a predefined API URL
export default new SystemService('http://192.168.101.209/api');
