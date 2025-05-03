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
  size?: number; // Add size prop for consistent scaling

}

const ChartContainer: React.FC<ChartContainerProps> = ({ type, data, progressValue, healthColor }) => {
  const theme = useTheme(); // ✅ Get theme inside the component

    if (type === 'sparkline') {
      
        if (!Array.isArray(data)) {
          console.warn('Expected an array for sparkline, but received:', data);
        }
      
        if (data.length === 0) {
          console.warn('Sparkline data array is empty.');
        }
      
        return (
           
            <>
            <Sparklines data={data} height={30} margin={10} >
            <SparklinesLine color={theme.palette.sparklines.main} style={{ fill: "none" }} />
            <SparklinesSpots spotColors={[theme.palette.sparklines.highlight]} size={4} />
            </Sparklines>
          </>
        );
      }

  if (type === 'pie' && typeof data === 'object' && Object.keys(data).length > 0) {
    return <PieChartComponent pieData={data}  />;
  }

  if (type === 'circular' && progressValue !== undefined) {
    return <CircularProgressBarComponent progressValue={progressValue} healthColor={healthColor} />;
  }

  return (
    <div style={{ textAlign: 'center', color: '#999' }}>
      <p>No Chart Data</p>
    </div>
  );
};

export default ChartContainer;
