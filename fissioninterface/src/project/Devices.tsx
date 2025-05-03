import React, { FC, useEffect, useState } from 'react';
import '../App.css';
import { SnackbarProvider, useSnackbar } from 'notistack';
import {
    Table,
    TableBody,
    TableCell,
    TableContainer,
    TableHead,
    TableRow,
    Paper,
    Select,
    MenuItem,
    Button,
    Box,
    Typography,
    Chip,
} from '@mui/material';
import { fetchScannedDevices } from '../api/DeviceScannerService';
import { fetchSensorAssignments, saveSensorAssignment } from '../api/DeviceConfigService';

// Predefined ports with their corresponding pins
const PORTS = [
    { port: 'Port 1', pin: 'D2' },  // GPIO 25
    { port: 'Port 2', pin: 'D3' },  // GPIO 26
    { port: 'Port 3', pin: 'D5' },  // GPIO 0
    { port: 'Port 4', pin: 'D6' },  // GPIO 14
    { port: 'Port 5', pin: 'D7' },  // GPIO 13
    { port: 'Port 6', pin: 'D9' },  // GPIO 2
];

// Available sensor types
const SENSOR_TYPES = [
    { value: '', label: 'None' },
    { value: 'airTempHumidity', label: 'Air Temperature and Humidity (DHT11)' },
    { value: 'soilMoisture', label: 'Soil Moisture' },
    { value: 'ph', label: 'pH Sensor' },
    { value: 'tds', label: 'TDS Sensor' },
    { value: 'temperature', label: 'Temperature (DS18B20)' },
    { value: 'npk', label: 'NPK Sensor' },
    { value: 'airQuality', label: 'Air Quality (ENS160)' },
    { value: 'spectral', label: 'Spectral Sensor (AS7341)' },
    { value: 'rtc', label: 'Real-Time Clock (DS3231)' },
];

const Devices: FC = () => {
    const { enqueueSnackbar } = useSnackbar();
    const [devices, setDevices] = useState<any | null>(null);
    const [loadingScan, setLoadingScan] = useState<boolean>(false);
    const [error, setError] = useState<string | null>(null);
    const [assignments, setAssignments] = useState<{ [key: string]: string }>(
        PORTS.reduce((acc, port) => ({ ...acc, [port.pin]: '' }), {})
    );

    // Load saved assignments and scan for devices
    useEffect(() => {
        const loadAssignments = async () => {
            try {
                const savedAssignments = await fetchSensorAssignments();
                setAssignments((prev) => ({
                    ...prev,
                    ...savedAssignments,
                }));
            } catch (err: any) {
                console.error('Error loading assignments:', err.message, err);
                enqueueSnackbar('Could not load saved assignments. Using defaults.', { variant: 'warning' });
            }
        };

        const scanDevices = async () => {
            setLoadingScan(true);
            try {
                const data = await fetchScannedDevices();
                console.log('Scanned devices:', data);
                setDevices(data);
                setAssignments((prev) => {
                    const updated = { ...prev };
                    PORTS.forEach((port) => {
                        const isDetected = data?.digitalPins?.some((device: any) => device.pin === port.pin);
                        if (!isDetected && updated[port.pin]) {
                            updated[port.pin] = '';
                            enqueueSnackbar(`Sensor on ${port.port} (${port.pin}) was removed.`, { variant: 'info' });
                        }
                    });
                    return updated;
                });
            } catch (err: any) {
                console.error('Error scanning devices:', err.message, err);
                setError('Could not scan devices. Ports shown as not detected.');
                enqueueSnackbar('Could not scan devices. Check server connection.', { variant: 'warning' });
            } finally {
                setLoadingScan(false);
            }
        };

        loadAssignments();
        scanDevices();
        const interval = setInterval(scanDevices, 30000);
        return () => clearInterval(interval);
    }, [enqueueSnackbar]);

    // Handle sensor assignment changes
    const handleAssignmentChange = (pin: string, value: string) => {
        setAssignments((prev) => ({ ...prev, [pin]: value }));
    };

    // Save the sensor assignment
    const handleSaveAssignments = async (pin: string) => {
        const sensorType = assignments[pin];
        if (sensorType) {
            try {
                await saveSensorAssignment({ pin, sensorType });
                enqueueSnackbar(
                    `Assigned ${SENSOR_TYPES.find((t) => t.value === sensorType)?.label} to ${pin} successfully!`,
                    { variant: 'success' }
                );
            } catch (err: any) {
                console.error('Error saving assignment:', err.message, err);
                enqueueSnackbar('Failed to save assignment.', { variant: 'error' });
            }
        } else {
            enqueueSnackbar('Please select a sensor type before saving.', { variant: 'warning' });
        }
    };

    // Render the table
    return (
        <SnackbarProvider maxSnack={3} anchorOrigin={{ vertical: 'bottom', horizontal: 'left' }}>
            <Box sx={{ margin: 2 }}>
                {error && (
                    <Typography variant="body1" color="error" align="center" sx={{ mb: 2 }}>
                        {error}
                    </Typography>
                )}
                <TableContainer component={Paper} sx={{ boxShadow: 3 }}>
                    <Table>
                        <TableHead>
                            <TableRow sx={{ backgroundColor: '#1976d2' }}>
                                <TableCell sx={{ color: 'white', fontWeight: 'bold' }}>Port</TableCell>
                                <TableCell sx={{ color: 'white', fontWeight: 'bold' }}>Pin</TableCell>
                                <TableCell sx={{ color: 'white', fontWeight: 'bold' }}>Current Value</TableCell>
                                <TableCell sx={{ color: 'white', fontWeight: 'bold' }}>Status</TableCell>
                                <TableCell sx={{ color: 'white', fontWeight: 'bold' }}>Sensor Assignment</TableCell>
                                <TableCell sx={{ color: 'white', fontWeight: 'bold' }}>Actions</TableCell>
                            </TableRow>
                        </TableHead>
                        <TableBody>
                            {PORTS.map((port) => {
                                const detectedDevice = devices?.digitalPins?.find((device: any) => device.pin === port.pin);
                                const isDetected = !!detectedDevice;
                                return (
                                    <TableRow key={port.pin}>
                                        <TableCell>{port.port}</TableCell>
                                        <TableCell>{port.pin}</TableCell>
                                        <TableCell>{isDetected ? detectedDevice.value : '--'}</TableCell>
                                        <TableCell>
                                            <Chip
                                                label={isDetected ? 'Detected' : 'Not Detected'}
                                                color={isDetected ? 'success' : 'default'}
                                                sx={{ fontWeight: 'bold', fontSize: '0.9rem', padding: '5px' }}
                                            />
                                        </TableCell>
                                        <TableCell>
                                            <Select
                                                value={assignments[port.pin] || ''}
                                                onChange={(e) => handleAssignmentChange(port.pin, e.target.value as string)}
                                                displayEmpty
                                                fullWidth
                                                disabled={!isDetected}
                                            >
                                                {SENSOR_TYPES.map((type) => (
                                                    <MenuItem key={type.value} value={type.value}>
                                                        {type.label}
                                                    </MenuItem>
                                                ))}
                                            </Select>
                                        </TableCell>
                                        <TableCell>
                                            <Button
                                                variant="contained"
                                                color="primary"
                                                onClick={() => handleSaveAssignments(port.pin)}
                                                sx={{ fontWeight: 'bold' }}
                                                disabled={!isDetected || !assignments[port.pin]}
                                            >
                                                Save
                                            </Button>
                                        </TableCell>
                                    </TableRow>
                                );
                            })}
                        </TableBody>
                    </Table>
                </TableContainer>
                {loadingScan && (
                    <Typography variant="body2" align="center" sx={{ mt: 2 }}>
                        Scanning for devices...
                    </Typography>
                )}
            </Box>
        </SnackbarProvider>
    );
};

export default Devices;