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
        LOGS: `${BASE_URL}/logs`,
    },
    CONTROLLER: {
        DATA: `${BASE_URL}/relay/getDeviceStates`,
        GET_THRESHOLDS: `${BASE_URL}/relay/getThresholds`,
        SET_THRESHOLDS: `${BASE_URL}/relay/setThresholds`,
        GET_RELAY_CONFIG: `${BASE_URL}/relay/config`,
        SET_RELAY_CONFIG: `${BASE_URL}/relay/config`,
        SAVE_CONFIG: `${BASE_URL}/relay/saveConfig`,
        SENSORS: `${BASE_URL}/sensors`,
    },
    SCANNER: {
        DEVICES: `${BASE_URL}/scan`,
        CONFIG: `${BASE_URL}/config`,
        TRIGGER: `${BASE_URL}/scan/trigger`,
        RESET: `${BASE_URL}/config/reset`,
    },
    TELEMETRY: {
        STATUS: `${BASE_URL}/telemetry`,
    },
    DRIVER_PACKAGES: {
        LIST: `${BASE_URL}/driver-packages`,
        UPLOAD: `${BASE_URL}/driver-packages`,
    },
    SETUP: {
        GET_IP: `${BASE_URL ?? ''}/setup`,
        REGISTER: `${BASE_URL ?? ''}/register`,
    },
};