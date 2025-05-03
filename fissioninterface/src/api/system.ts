import { AxiosResponse } from 'axios';
import { OTASettings, SystemStatus } from '../types/system';
import { AXIOS, FileUploadConfig, uploadFile } from './endpoints';
import { API_CONFIG } from './apiConfig';

class SystemService {
    // Method to read system status
    public async readSystemStatus(timeout?: number): Promise<SystemStatus> {
        try {
            const response: AxiosResponse<SystemStatus> = await AXIOS.get(API_CONFIG.SYSTEM.STATUS, { timeout });
            return response.data;
        } catch (error) {
            console.error('Error reading system status:', error);
            throw error;
        }
    }

    // Method to restart system
    public async restart(): Promise<void> {
        try {
            await AXIOS.post(API_CONFIG.SYSTEM.RESTART);
        } catch (error) {
            console.error('Error restarting system:', error);
            throw error;
        }
    }

    // Method to perform factory reset
    public async factoryReset(): Promise<void> {
        try {
            await AXIOS.post(API_CONFIG.SYSTEM.FACTORY_RESET);
        } catch (error) {
            console.error('Error performing factory reset:', error);
            throw error;
        }
    }

    // Method to read OTA settings
    public async readOTASettings(): Promise<OTASettings> {
        try {
            const response: AxiosResponse<OTASettings> = await AXIOS.get(API_CONFIG.SYSTEM.OTA_SETTINGS);
            return response.data;
        } catch (error) {
            console.error('Error reading OTA settings:', error);
            throw error;
        }
    }

    // Method to update OTA settings
    public async updateOTASettings(otaSettings: OTASettings): Promise<OTASettings> {
        try {
            const response: AxiosResponse<OTASettings> = await AXIOS.post(API_CONFIG.SYSTEM.OTA_SETTINGS, otaSettings);
            return response.data;
        } catch (error) {
            console.error('Error updating OTA settings:', error);
            throw error;
        }
    }

    // Method to upload firmware
    public async uploadFirmware(file: File, config?: FileUploadConfig): Promise<void> {
        try {
            await uploadFile(API_CONFIG.SYSTEM.UPLOAD_FIRMWARE, file, config);
        } catch (error) {
            console.error('Error uploading firmware:', error);
            throw error;
        }
    }
}

export default new SystemService();
