import React, { FC } from 'react';
import { Card, CardContent, Typography, Button } from '@mui/material';

interface SensorCardProps {
  title: string;
  value: number | null;
  unit: string;
  showButton?: boolean;
  buttonText?: string;
  onButtonClick?: () => void;
}

const SensorCard: FC<SensorCardProps> = ({
  title,
  value,
  unit,
  showButton = false,
  buttonText,
  onButtonClick,
}) => {
  const isUnavailable = value === null || value === -1;

  return (
    <Card sx={{ minWidth: 200, margin: 1, opacity: isUnavailable ? 0.5 : 1 }}>
      <CardContent>
        <Typography variant="h6">{title}</Typography>
        <Typography variant="body1">
          {isUnavailable ? "Data Unavailable" : `${value} ${unit}`}
        </Typography>
        {showButton && (
          <Button onClick={onButtonClick} variant="contained" size="small">
            {buttonText}
          </Button>
        )}
      </CardContent>
    </Card>
  );
};

export default SensorCard;
