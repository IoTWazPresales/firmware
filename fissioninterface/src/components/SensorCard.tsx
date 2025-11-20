import React from 'react';
import { Card, Typography, Box, Switch, Skeleton, Badge } from '@mui/material';
import WarningIcon from '@mui/icons-material/Warning';
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
  onClick?: () => void;
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
  onClick,
}) => {
  const showThresholds = minThreshold !== undefined && maxThreshold !== undefined;
  const resolvedHealthColor =
    healthColor && progressValue !== undefined ? healthColor(progressValue) : 'primary';
  
  // Check if value is outside thresholds (alert condition)
  const isAlert = showThresholds && value !== null && typeof value === 'number' && (
    (minThreshold !== null && value < minThreshold) ||
    (maxThreshold !== null && value > maxThreshold)
  );

  const handleKeyDown = (event: React.KeyboardEvent<HTMLDivElement>) => {
    if (!onClick) {
      return;
    }
    if (event.key === 'Enter' || event.key === ' ') {
      event.preventDefault();
      onClick();
    }
  };

  return (
    <Badge
      badgeContent={isAlert ? <WarningIcon sx={{ fontSize: 16, color: 'error.main' }} /> : 0}
      color="error"
      invisible={!isAlert}
      sx={{ width: '100%', maxWidth: 350 }}
    >
      <Card
        sx={{
          width: '100%',
          minHeight: 280,
          maxWidth: 350,
          padding: 2,
          borderRadius: 2,
          boxShadow: isAlert 
            ? '0 0 15px rgba(239, 68, 68, 0.4), 0 4px 6px rgba(0, 0, 0, 0.3)' 
            : '0 0 10px rgba(0, 212, 255, 0.3), 0 2px 4px rgba(0, 0, 0, 0.2)',
          border: isAlert ? '2px solid' : '1px solid',
          borderColor: isAlert ? 'error.main' : 'rgba(0, 212, 255, 0.5)',
          display: 'flex',
          flexDirection: 'column',
          alignItems: 'center',
          transition: 'transform 0.2s, box-shadow 0.3s, border-color 0.3s',
          cursor: onClick ? 'pointer' : 'default',
          '&:hover': {
            transform: 'translateY(-4px)',
            boxShadow: isAlert 
              ? '0 0 25px rgba(239, 68, 68, 0.6), 0 6px 8px rgba(0, 0, 0, 0.4)' 
              : '0 0 20px rgba(0, 212, 255, 0.5), 0 4px 8px rgba(0, 0, 0, 0.3)',
            borderColor: isAlert ? 'error.main' : 'rgba(0, 255, 255, 0.8)',
          },
        }}
        onClick={onClick}
        role={onClick ? 'button' : undefined}
        tabIndex={onClick ? 0 : undefined}
        onKeyDown={handleKeyDown}
        aria-label={onClick ? `View details for ${title}` : undefined}
      >
      <Box sx={{ width: '100%', display: 'flex', justifyContent: 'space-between', mb: 1 }}>
        <Typography variant="subtitle2">{title}</Typography>
        {deviceStatus !== undefined && (
          <Switch checked={deviceStatus} color={deviceStatus ? 'success' : 'error'} />
        )}
      </Box>

      <Box sx={{ textAlign: 'center', minHeight: 50, width: '100%' }}>
        {loading ? (
          <Box sx={{ width: '100%' }}>
            <Skeleton variant="text" width="60%" sx={{ mx: 'auto', height: 40 }} />
            <Skeleton variant="text" width="40%" sx={{ mx: 'auto' }} />
          </Box>
        ) : value !== null ? (
          <Typography 
            variant="h4" 
            color="primary" 
            sx={{ 
              fontWeight: 600,
              textShadow: '0 0 10px rgba(0, 212, 255, 0.5)',
            }}
          >
            {`${Number(value).toFixed(2)} ${unit}`}
          </Typography>
        ) : (
          <Box sx={{ py: 2 }}>
            <Typography variant="body2" color="text.secondary" sx={{ fontStyle: 'italic' }}>
              No Data Available
            </Typography>
          </Box>
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

      <Box sx={{ flexGrow: 1, display: 'flex', alignItems: 'center', width: '100%', justifyContent: 'center', minHeight: 80 }}>
        {loading ? (
          <Skeleton variant="rectangular" width="100%" height={60} sx={{ borderRadius: 1 }} />
        ) : (
          <ChartContainer
            type={chartType}
            data={chartData}
            progressValue={progressValue}
            healthColor={resolvedHealthColor}
          />
        )}
      </Box>
      </Card>
    </Badge>
  );
};

export default SensorCard;
