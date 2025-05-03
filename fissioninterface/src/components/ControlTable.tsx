import React from "react";
import {
  Table,
  TableBody,
  TableCell,
  TableContainer,
  TableHead,
  TableRow,
  Paper,
  Slider,
  Button,
  Typography,
} from "@mui/material";

interface Device {
  id: string;
  name: string;
  monitors: string; // New Column for What It Monitors
  lastSeen: string;
  lastOn: string;
  status: boolean;
  minThreshold: number;
  maxThreshold: number;
  unit: string;
}

interface ControlTableProps {
  devices: Device[];
  onThresholdChange: (id: string, newMin: number, newMax: number) => void;
  onSaveThresholds: (id: string) => void;
}

const ControlTable: React.FC<ControlTableProps> = ({
  devices,
  onThresholdChange,
  onSaveThresholds,
}) => {
  return (
    <TableContainer component={Paper}>
      <Table>
        <TableHead>
          <TableRow>
            <TableCell>Device Name</TableCell>
            <TableCell>Monitors</TableCell> {/* New Column */}
            <TableCell>Last Seen</TableCell>
            <TableCell>Last On</TableCell>
            <TableCell>Status</TableCell>
            <TableCell>Threshold Settings</TableCell>
            <TableCell>Action</TableCell>
          </TableRow>
        </TableHead>
        <TableBody>
          {devices.map((device) => (
            <TableRow key={device.id}>
              <TableCell>{device.name}</TableCell>
              <TableCell>{device.monitors}</TableCell> {/* New Column Data */}
              <TableCell>{device.lastSeen}</TableCell>
              <TableCell>{device.lastOn}</TableCell>
              <TableCell>{device.status ? "On" : "Off"}</TableCell>
              <TableCell>
                <Typography variant="caption">Min: {device.minThreshold}{device.unit}</Typography>
                <Slider
                  value={[device.minThreshold, device.maxThreshold]}
                  onChange={(_, newValue) => {
                    if (Array.isArray(newValue)) {
                      onThresholdChange(device.id, newValue[0], newValue[1]);
                    }
                  }}
                  valueLabelDisplay="auto"
                  min={0}
                  max={100}
                />
                <Typography variant="caption">Max: {device.maxThreshold}{device.unit}</Typography>
              </TableCell>
              <TableCell>
                <Button variant="contained" color="primary" onClick={() => onSaveThresholds(device.id)}>
                  Save
                </Button>
              </TableCell>
            </TableRow>
          ))}
        </TableBody>
      </Table>
    </TableContainer>
  );
};

export default ControlTable;
