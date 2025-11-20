import { FC } from 'react';
import {
  Dialog,
  DialogTitle,
  DialogContent,
  DialogActions,
  Button,
  Typography,
  Stack,
  Chip,
  Box,
} from '@mui/material';
import WarningAmberIcon from '@mui/icons-material/WarningAmber';
import { Sparklines, SparklinesLine, SparklinesSpots } from 'react-sparklines-typescript';

interface SensorDetailDialogProps {
  open: boolean;
  onClose: () => void;
  sensorName: string;
  unit: string;
  value: number | string | null;
  history: number[];
  thresholds?: { min?: number | null; max?: number | null };
  deviceStatus?: boolean;
}

const SensorDetailDialog: FC<SensorDetailDialogProps> = ({
  open,
  onClose,
  sensorName,
  unit,
  value,
  history,
  thresholds,
  deviceStatus,
}) => {
  const formattedValue =
    value === null || value === undefined
      ? 'Unknown'
      : typeof value === 'number'
        ? `${value.toFixed(2)} ${unit}`
        : `${value} ${unit}`;

  const isAlert =
    typeof value === 'number' &&
    thresholds &&
    ((thresholds.min !== null && thresholds.min !== undefined && value < thresholds.min) ||
      (thresholds.max !== null && thresholds.max !== undefined && value > thresholds.max));

  return (
    <Dialog open={open} onClose={onClose} maxWidth="sm" fullWidth>
      <DialogTitle>{sensorName}</DialogTitle>
      <DialogContent dividers>
        <Stack spacing={2}>
          <Box>
            <Typography variant="subtitle2" color="text.secondary">
              Current Value
            </Typography>
            <Typography variant="h4" sx={{ fontWeight: 600 }}>
              {formattedValue}
            </Typography>
            {deviceStatus !== undefined && (
              <Chip
                size="small"
                label={deviceStatus ? 'Device Active' : 'Device Idle'}
                color={deviceStatus ? 'success' : 'default'}
                sx={{ mt: 1 }}
              />
            )}
          </Box>

          {thresholds && (
            <Stack direction="row" spacing={2}>
              <Chip label={`Min: ${thresholds.min ?? '—'} ${unit}`} variant="outlined" />
              <Chip label={`Max: ${thresholds.max ?? '—'} ${unit}`} variant="outlined" />
              {isAlert && (
                <Chip
                  color="error"
                  icon={<WarningAmberIcon />}
                  label="Outside safe range"
                />
              )}
            </Stack>
          )}

          <Box>
            <Typography variant="subtitle2" gutterBottom>
              Last 20 samples
            </Typography>
            {history.length > 0 ? (
              <Box sx={{ height: 120 }}>
                <Sparklines data={history} height={120} margin={6}>
                  <SparklinesLine color="#06b6d4" style={{ fill: 'none' }} />
                  <SparklinesSpots size={4} spotColors={['#6366f1']} />
                </Sparklines>
              </Box>
            ) : (
              <Typography variant="body2" color="text.secondary">
                No history available for this sensor yet.
              </Typography>
            )}
          </Box>
        </Stack>
      </DialogContent>
      <DialogActions>
        <Button onClick={onClose}>Close</Button>
      </DialogActions>
    </Dialog>
  );
};

export default SensorDetailDialog;

