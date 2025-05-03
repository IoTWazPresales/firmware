// apiConfig.ts

const BASE_URL = 'http://192.168.101.252/api';

export const API_CONFIG = {
    BASE_URL,
    WIFI: {
        STATUS: `${BASE_URL}/wifiStatus`,
        SCAN: `${BASE_URL}/scanNetworks`,
        LIST: `${BASE_URL}/listNetworks`,
        SETTINGS: `${BASE_URL}/wifiSettings`,
    },
    SYSTEM: {
        STATUS: `${BASE_URL}/systemStatus`,
        RESTART: `${BASE_URL}/restart`,
        FACTORY_RESET: `${BASE_URL}/factoryReset`,
        OTA_SETTINGS: `${BASE_URL}/otaSettings`,
        UPLOAD_FIRMWARE: `${BASE_URL}/uploadFirmware`,
    },
    SENSOR: {
        DATA: `${BASE_URL}/sensor`,
        LOGS: `${BASE_URL}/logs`, // Added new endpoint for sensor logs
    },
    CONTROLLER: {
        DATA: `${BASE_URL}/relay/GetDeviceStates`,
        SET_THRESHOLDS: `${BASE_URL}/relay/setThresholds`, 
        GET_THRESHOLDS: `${BASE_URL}/relay/getThresholds`, 
    },
    SCANNER: {
        DEVICES: `${BASE_URL}/scan`, // Add the device scanner endpoint
        CONFIG: `${BASE_URL}/config`, // New endpoint to load assignments
        CONFIGURE: `${BASE_URL}/configure`, // New endpoint to save assignments
    },
};
