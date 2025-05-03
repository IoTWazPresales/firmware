import React, { useEffect, useState } from 'react';
import { Card, Typography, Box, Switch } from '@mui/material';
import ChartContainer from './ChartContainer';

interface SensorCardProps {
  title: string;
  value: number | string | null;
  unit: string;
  loading: boolean;
  chartType: 'pie' | 'circular' | 'sparkline';
  chartData?: any;
  minThreshold?: number;
  maxThreshold?: number;
  deviceStatus?: boolean;
  progressValue?: number;
  healthColor?: (value: number) => string;
}

const SensorCard: React.FC<SensorCardProps> = ({
  title,
  value,
  unit,
  loading,
  chartType,
  chartData,
  minThreshold,
  maxThreshold,
  deviceStatus,
  progressValue,
  healthColor,
}) => {
  const [loadingText, setLoadingText] = useState('Loading');

  useEffect(() => {
    if (loading) {
      const interval = setInterval(() => {
        setLoadingText((prev) => (prev === 'Loading...' ? 'Loading' : `${prev}.`));
      }, 500);
      return () => clearInterval(interval);
    }
  }, [loading]);
  
  const showThresholds = minThreshold !== undefined && maxThreshold !== undefined;
  const resolvedHealthColor =
    healthColor && progressValue !== undefined ? healthColor(progressValue) : 'primary';

  return (
    <Card
      sx={{
        width: 250, // Fixed width
    height: 250, // Fixed height to match width, making it square
    padding: 2,
    borderRadius: 2,
    boxShadow: 2,
    display: 'flex',
    flexDirection: 'column',
    
    alignItems: 'center',
      }}
    >
      <Box sx={{ width: '100%', display: 'flex', justifyContent: 'space-between', mb: 1 }}>
        <Typography variant="subtitle2">{title}</Typography>
        {deviceStatus !== undefined && (
          <Switch checked={deviceStatus} color={deviceStatus ? 'success' : 'error'} />
        )}
      </Box>

      <Box sx={{ textAlign: 'center', minHeight: 50 }}>
        {loading ? (
          <Typography variant="body1" color="text.secondary">
            {loadingText}
          </Typography>
        ) : value !== null ? (
          <Typography variant="h4" color="primary">
            {`${Number(value).toFixed(2)} ${unit}`}
          </Typography>
        ) : (
          <Typography variant="body1" color="text.secondary">
            No Data
          </Typography>
        )}
      </Box>

      {showThresholds && (
        <Box sx={{ display: 'flex', justifyContent: 'space-between', width: '100%', mb: 2 }}>
          <Typography variant="caption" color="text.secondary">
            Min: {minThreshold} {unit}
          </Typography>
          <Typography variant="caption" color="text.secondary">
            Max: {maxThreshold} {unit}
          </Typography>
        </Box>
      )}

      <Box sx={{ flexGrow: 1, display: 'flex', alignItems: 'center',width: '100%', justifyContent: 'center'}}>
        <ChartContainer
          type={chartType}
          data={chartData}
          progressValue={progressValue}
          healthColor={resolvedHealthColor}
        />
      </Box>
    </Card>
  );
};

export default SensorCard;
