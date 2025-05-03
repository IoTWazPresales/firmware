import React from 'react';
import { PieChart } from '@mui/x-charts';

interface PieChartComponentProps {
  pieData: Record<string, number>;
  height?: number;
  width?: number;
  innerRadius?: number;
  outerRadius?: number;
  paddingAngle?: number;
  cornerRadius?: number;
  startAngle?: number;
  endAngle?: number;
  cx?: number;
  cy?: number;

}

const PieChartComponent: React.FC<PieChartComponentProps> = ({ pieData, height = 100, width = 200,  innerRadius = 10,
  outerRadius = 50,
  paddingAngle = 2,
  cornerRadius = 5,
  startAngle = 0,
  endAngle = 360,
 }) => {
  if (!pieData || Object.keys(pieData).length === 0) {
    return null; // Return nothing if no data is present
  }
  const colorMapping: Record<string, string> = {
    Blue: 'skyblue',
    Green: 'limegreen',
    Red: '#e15759',
    FarRed: 'darkred',
  };

  return (
    
    <PieChart
      series={[
        {
          data: Object.entries(pieData).map(([label, value]) => ({ id: label, value, color: colorMapping[label] || 'gray', // Use 'gray' as a fallback for unknown labels
          })),
          innerRadius,
          outerRadius,
          paddingAngle,
          cornerRadius,
          startAngle,
          endAngle,
         
        },
        
      ]}
      height={height}
      width={width}
      slotProps={{
        legend: { hidden: true },
      }}
      margin={{ right: 0, left:0 }}

      
    />
  );
};

export default PieChartComponent;
