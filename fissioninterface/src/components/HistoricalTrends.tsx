import { FC, useMemo } from 'react';
import { Card, CardContent, Typography, Grid, Box, Divider } from '@mui/material';
import { Sparklines, SparklinesLine, SparklinesSpots } from 'react-sparklines-typescript';
import { SensorLogEntry } from '../api/SensorHistory';

interface HistoricalTrendsProps {
  logs: SensorLogEntry[];
}

const METRICS: Array<{ key: keyof SensorLogEntry; label: string; unit: string }> = [
  { key: 'temperature', label: 'Water Temp', unit: '°C' },
  { key: 'airtemp', label: 'Air Temp', unit: '°C' },
  { key: 'humidity', label: 'Humidity', unit: '%' },
  { key: 'ph', label: 'pH', unit: '' },
  { key: 'moisture', label: 'Soil Moisture', unit: '%' },
  { key: 'tdsSens', label: 'TDS', unit: 'ppm' },
];

const HistoricalTrends: FC<HistoricalTrendsProps> = ({ logs }) => {
  const recentLogs = useMemo(() => logs.slice(-48), [logs]);

  if (!recentLogs.length) {
    return (
      <Card elevation={3}>
        <CardContent>
          <Typography variant="h6" gutterBottom>
            Historical Trends
          </Typography>
          <Typography variant="body2" color="text.secondary">
            No historical records available yet. Once the controller syncs with Supabase we’ll display rolling trends here.
          </Typography>
        </CardContent>
      </Card>
    );
  }

  return (
    <Card elevation={3}>
      <CardContent>
        <Typography variant="h6" gutterBottom>
          Historical Trends
        </Typography>
        <Grid container spacing={2}>
          {METRICS.map(({ key, label, unit }) => {
            const series = recentLogs.map((entry) => entry[key]).filter((v): v is number => typeof v === 'number');
            if (!series.length) {
              return (
                <Grid item xs={12} sm={6} md={4} key={key as string}>
                  <Box sx={{ p: 2, borderRadius: 2, bgcolor: 'background.default', border: '1px solid rgba(255,255,255,0.05)' }}>
                    <Typography variant="subtitle2" gutterBottom>
                      {label}
                    </Typography>
                    <Typography variant="caption" color="text.secondary">
                      Awaiting samples…
                    </Typography>
                  </Box>
                </Grid>
              );
            }

            const latestValue = series[series.length - 1];
            const delta = latestValue - series[0];
            const trend = delta > 0 ? '↑' : delta < 0 ? '↓' : '→';

            return (
              <Grid item xs={12} sm={6} md={4} key={key as string}>
                <Box sx={{ p: 2, borderRadius: 2, bgcolor: 'background.paper', border: '1px solid rgba(255,255,255,0.08)' }}>
                  <Typography variant="subtitle2">
                    {label}
                  </Typography>
                  <Typography variant="caption" color="text.secondary">
                    Last value: {latestValue.toFixed(2)} {unit} ({trend} {Math.abs(delta).toFixed(2)}{unit})
                  </Typography>
                  <Divider sx={{ my: 1, opacity: 0.2 }} />
                  <Box sx={{ height: 50 }}>
                    <Sparklines data={series} height={50} margin={6}>
                      <SparklinesLine color="#06b6d4" style={{ fill: 'none' }} />
                      <SparklinesSpots size={3} spotColors={['#6366f1']} />
                    </Sparklines>
                  </Box>
                </Box>
              </Grid>
            );
          })}
        </Grid>
      </CardContent>
    </Card>
  );
};

export default HistoricalTrends;

