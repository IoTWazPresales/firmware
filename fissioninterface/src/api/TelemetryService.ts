import axiosInstance from './axiosInstance';
import { API_CONFIG } from './apiConfig';

export interface TelemetryResponse {
  uptime_ms: number;
  free_heap: number;
  min_free_heap: number;
  max_alloc_heap: number;
  wifi_connected: boolean;
  wifi_rssi?: number;
  queues?: {
    hybrid_available?: number;
    cloud_available?: number;
  };
  [key: string]: any;
}

class TelemetryService {
  async fetchTelemetry(): Promise<TelemetryResponse> {
    const response = await axiosInstance.get(API_CONFIG.TELEMETRY.STATUS);
    return response.data as TelemetryResponse;
  }
}

const telemetryService = new TelemetryService();

export default telemetryService;
