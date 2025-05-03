import React from 'react';
import { CircularProgress } from '@mui/material';

interface CircularProgressBarComponentProps {
  progressValue: number;
  healthColor?: string;
  size?: number;
  thickness?: number;
}

const CircularProgressBarComponent: React.FC<CircularProgressBarComponentProps> = ({
  progressValue,
  healthColor = 'primary',
  size = 100,
  thickness = 5,
}) => {
  return (
    <CircularProgress
      variant="determinate"
      value={progressValue}
      size={size}
      thickness={thickness}
      sx={{ color: healthColor }}
    />
  );
};

export default CircularProgressBarComponent;
