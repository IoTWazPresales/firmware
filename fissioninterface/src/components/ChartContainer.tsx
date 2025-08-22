import React from 'react';
import { Sparklines, SparklinesLine, SparklinesSpots } from 'react-sparklines-typescript';
import PieChartComponent from './PieChartComponent';
import CircularProgressBarComponent from './CircularProgressBarComponent';
import { Typography, Box } from '@mui/material';
import { useTheme } from "@mui/material/styles";

interface ChartContainerProps {
  type: 'sparkline' | 'pie' | 'circular';
  data?: any;
  progressValue?: number;
  healthColor?: string;
  size?: number;
}

const ChartContainer: React.FC<ChartContainerProps> = ({
  type,
  data,
  progressValue,
  healthColor,
  size
}) => {
  const theme = useTheme();

  // — SPARKLINE —————————————————————————————————————————————
  if (type === 'sparkline') {
    // only render when data is an array with at least one point
    if (!Array.isArray(data) || data.length === 0) {
      return (
        <Box
          sx={{
            height: 30,
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'center',
            color: theme.palette.text.disabled,
            fontSize: '0.75rem',
          }}
        >
          
        </Box>
      );
    }

    return (
      <Sparklines data={data} height={30} margin={8}>
        <SparklinesLine
          color={theme.palette.sparklines.main}
          style={{ fill: 'none' }}
        />
        <SparklinesSpots
          spotColors={[theme.palette.sparklines.highlight]}
          size={4}
        />
      </Sparklines>
    );
  }

  // — PIE CHART —————————————————————————————————————————————
  if (type === 'pie' && data && Object.keys(data).length > 0) {
    return <PieChartComponent pieData={data} />;
  }

  // — CIRCULAR PROGRESS —————————————————————————————————————
  if (type === 'circular' && progressValue !== undefined) {
    return (
      <CircularProgressBarComponent
        progressValue={progressValue}
        healthColor={healthColor}
      />
    );
  }

  // — FALLBACK ——————————————————————————————————————————————
  return (
    <Box
      sx={{
        height: size || 40,
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'center',
        color: theme.palette.text.disabled,
      }}
    >
      <Typography variant="caption">No Chart Data</Typography>
    </Box>
  );
};

export default ChartContainer;
