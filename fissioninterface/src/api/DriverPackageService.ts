import axios from 'axios';
import { API_CONFIG } from './apiConfig';

export interface DriverPackageSummary {
  name: string;
  size: number;
}

class DriverPackageService {
  async listPackages(): Promise<DriverPackageSummary[]> {
    const response = await axios.get(API_CONFIG.DRIVER_PACKAGES.LIST);
    const items = response.data?.items ?? [];
    return Array.isArray(items) ? items : [];
  }

  async uploadPackage(file: File): Promise<void> {
    const formData = new FormData();
    formData.append('file', file);
    await axios.post(API_CONFIG.DRIVER_PACKAGES.UPLOAD, formData, {
      headers: {
        'Content-Type': 'multipart/form-data',
      },
    });
  }
}

const driverPackageService = new DriverPackageService();

export default driverPackageService;
