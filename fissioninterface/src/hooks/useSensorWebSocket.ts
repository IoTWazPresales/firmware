import { useEffect, useState, useRef } from 'react';
import { useWs } from '../utils/useWs';
import { SensorValues } from '../types/sensors';

interface SensorWebSocketData {
  values: SensorValues;
  timestamp: number;
}

export const useSensorWebSocket = (enabled: boolean = true) => {
  // NeuroGrow WebSocket endpoint is /ws (not /ws/ws)
  const location = window.location;
  const wsProtocol = location.protocol === "https:" ? "wss:" : "ws:";
  const wsUrl = `${wsProtocol}//${location.host}/ws`;
  const { connected, data } = useWs<SensorWebSocketData>(wsUrl, 100);
  const [sensorData, setSensorData] = useState<SensorValues>({});
  const lastUpdateRef = useRef<number>(0);

  useEffect(() => {
    if (enabled && data && data.values) {
      // Only update if data is newer (prevent stale updates)
      if (data.timestamp > lastUpdateRef.current) {
        setSensorData(data.values);
        lastUpdateRef.current = data.timestamp;
      }
    }
  }, [data, enabled]);

  return { connected, sensorData, setSensorData };
};

