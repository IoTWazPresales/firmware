import React, { useState } from 'react';
import { Card, CardContent, Typography, Box, Button, Switch, FormControlLabel, Slider } from '@mui/material';
import { Settings as SettingsIcon } from '@mui/icons-material';

interface SensorCardProps {
  title: string;
  value: string | boolean;
  isLoading?: boolean;
  settings?: boolean;
  sliderData?: {
    minMoisture: number;
    maxMoisture: number;
    handleMinMoistureChange: (event: Event, newValue: number | number[]) => void;
    handleMaxMoistureChange: (event: Event, newValue: number | number[]) => void;
  };
  onButtonClick?: () => void;
  buttonLabel?: string;
  switch?: boolean;  // New prop for rendering the switch
  onSwitchChange?: (checked: boolean) => void; // New prop to handle switch changes
}

const SensorCard: React.FC<SensorCardProps> = ({
  title,
  value,
  isLoading = false,
  settings,
  sliderData,
  onButtonClick,
  buttonLabel,
  switch: renderSwitch, // Destructure the `switch` prop
  onSwitchChange,
}) => {
  const [switchState, setSwitchState] = useState(false);

  const handleSwitchChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    const checked = event.target.checked;
    setSwitchState(checked);
    if (onSwitchChange) {
      onSwitchChange(checked); // Notify parent about the state change
    }
  };

  return (
    <Card
      sx={{
        minWidth: 200,
        margin: 2,
        borderRadius: 3,
        boxShadow: 3,
        background: 'linear-gradient(45deg, #6b8e23, #9acd32)',
        overflow: 'hidden',
        transition: 'transform 0.3s ease, box-shadow 0.3s ease',
        '&:hover': { transform: 'scale(1.05)', boxShadow: 6 }, // Hover effect
      }}
    >
      <CardContent sx={{ color: '#fff' }}>
        <Typography variant="h5" sx={{ fontWeight: 'bold' }}>
          {title}
        </Typography>

        <Typography variant="body1">{isLoading ? 'Loading...' : value}</Typography>

        {/* Render Switch if the prop is passed as true */}
        {renderSwitch && (
          <Box mt={2}>
            <FormControlLabel
              control={<Switch checked={switchState} onChange={handleSwitchChange} />}
              label={switchState ? 'On' : 'Off'}
            />
          </Box>
        )}

        {/* Render settings for Soil Moisture if settings prop is true */}
        {settings && sliderData && (
          <Box mt={2}>
            <Typography variant="subtitle1" sx={{ fontWeight: 'bold' }}>
              Soil Moisture Settings
            </Typography>

            {/* Min Moisture Slider */}
            <Box display="flex" alignItems="center" mt={1}>
              <Typography variant="body2" sx={{ minWidth: 90 }}>Min Moisture:</Typography>
              <Slider
                value={sliderData.minMoisture}
                onChange={sliderData.handleMinMoistureChange}
                valueLabelDisplay="auto"
                min={0}
                max={100}
                sx={{ flexGrow: 1, ml: 2 }}
              />
            </Box>

            {/* Max Moisture Slider */}
            <Box display="flex" alignItems="center" mt={1}>
              <Typography variant="body2" sx={{ minWidth: 90 }}>Max Moisture:</Typography>
              <Slider
                value={sliderData.maxMoisture}
                onChange={sliderData.handleMaxMoistureChange}
                valueLabelDisplay="auto"
                min={0}
                max={100}
                sx={{ flexGrow: 1, ml: 2 }}
              />
            </Box>
          </Box>
        )}

        {onButtonClick && buttonLabel && (
          <Button
            variant="contained"
            color="secondary"
            onClick={onButtonClick}
            sx={{
              mt: 2,
              width: '100%',
              fontSize: '1rem',
              padding: '10px',
              backgroundColor: '#28a745',
              '&:hover': { backgroundColor: '#218838' }, // Hover effect for button
            }}
            endIcon={<SettingsIcon />}
          >
            {buttonLabel}
          </Button>
        )}
      </CardContent>
    </Card>
  );
};

export default SensorCard;
