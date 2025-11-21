import axios, { AxiosError, AxiosInstance } from 'axios';
import { API_CONFIG } from './apiConfig';

// Create axios instance with default config
const axiosInstance: AxiosInstance = axios.create({
  baseURL: API_CONFIG.BASE_URL,
  timeout: 10000, // 10 second timeout
  headers: {
    'Content-Type': 'application/json',
  },
});

// Request interceptor
axiosInstance.interceptors.request.use(
  (config) => {
    return config;
  },
  (error) => {
    return Promise.reject(error);
  }
);

// Response interceptor to handle errors gracefully
axiosInstance.interceptors.response.use(
  (response) => {
    return response;
  },
  (error: AxiosError) => {
    // Handle network errors (backend not available)
    if (!error.response) {
      // Network error - backend is not reachable
      const networkError = new Error('Backend not available. Please ensure the NeuroGrow device is connected and running.');
      (networkError as any).isNetworkError = true;
      (networkError as any).originalError = error;
      return Promise.reject(networkError);
    }
    
    // Handle HTTP errors (backend responded with error status)
    return Promise.reject(error);
  }
);

export default axiosInstance;

