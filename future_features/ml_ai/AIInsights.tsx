import React, { FC, useEffect, useState } from 'react';
import {
  Card,
  CardContent,
  Typography,
  Box,
  Chip,
  LinearProgress,
  Alert,
  Grid,
  List,
  ListItem,
  ListItemText,
  ListItemIcon,
} from '@mui/material';
import TrendingUpIcon from '@mui/icons-material/TrendingUp';
import TrendingDownIcon from '@mui/icons-material/TrendingDown';
import WarningIcon from '@mui/icons-material/Warning';
import CheckCircleIcon from '@mui/icons-material/CheckCircle';
import axiosInstance from '../api/axiosInstance';
import { API_CONFIG } from '../api/apiConfig';

interface Prediction {
  predictions: number[];
  insights: {
    current_value: number;
    predicted_value_6h: number;
    trend: string;
    recommendations: Array<{
      action: string;
      priority: string;
      reason: string;
    }>;
  };
}

interface Recommendation {
  parameter: string;
  current: number;
  optimal: number;
  action: string;
  priority: string;
  suggestion: string;
}

interface AIInsightsProps {
  deviceId: string;
  currentReadings: any;
}

const AIInsights: FC<AIInsightsProps> = ({ deviceId, currentReadings }) => {
  const [predictions, setPredictions] = useState<Prediction | null>(null);
  const [recommendations, setRecommendations] = useState<any>(null);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    const fetchInsights = async () => {
      setLoading(true);
      setError(null);

      try {
        // Fetch predictions
        const predResponse = await axiosInstance.post(
          `${API_CONFIG.BASE_URL}/ml/predict`,
          {
            device_id: deviceId,
            sensor_type: 'temperature',
            hours_ahead: 6,
          }
        );
        setPredictions(predResponse.data);

        // Fetch recommendations
        const recResponse = await axiosInstance.post(
          `${API_CONFIG.BASE_URL}/ml/recommendations`,
          currentReadings,
          {
            params: { plant_type: 'general' },
          }
        );
        setRecommendations(recResponse.data);
      } catch (err: any) {
        if (err?.isNetworkError) {
          setError('ML service not available. Please ensure the service is running.');
        } else {
          setError('Failed to fetch AI insights');
        }
      } finally {
        setLoading(false);
      }
    };

    if (deviceId && currentReadings) {
      fetchInsights();
    }
  }, [deviceId, currentReadings]);

  if (loading) {
    return (
      <Card>
        <CardContent>
          <Typography variant="h6" gutterBottom>
            🧠 AI Insights
          </Typography>
          <LinearProgress />
          <Typography variant="body2" color="text.secondary" sx={{ mt: 2 }}>
            Analyzing sensor data...
          </Typography>
        </CardContent>
      </Card>
    );
  }

  if (error) {
    return (
      <Card>
        <CardContent>
          <Alert severity="warning">{error}</Alert>
        </CardContent>
      </Card>
    );
  }

  return (
    <Grid container spacing={2}>
      {/* Predictions Card */}
      {predictions && (
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                📈 Predictions
              </Typography>
              <Box sx={{ mb: 2 }}>
                <Typography variant="body2" color="text.secondary">
                  Current: {predictions.insights.current_value.toFixed(2)}°C
                </Typography>
                <Typography variant="body2" color="text.secondary">
                  Predicted (6h): {predictions.insights.predicted_value_6h.toFixed(2)}°C
                </Typography>
                <Chip
                  icon={
                    predictions.insights.trend === 'increasing' ? (
                      <TrendingUpIcon />
                    ) : (
                      <TrendingDownIcon />
                    )
                  }
                  label={`Trend: ${predictions.insights.trend}`}
                  color={predictions.insights.trend === 'increasing' ? 'success' : 'info'}
                  sx={{ mt: 1 }}
                />
              </Box>

              {predictions.insights.recommendations.length > 0 && (
                <Box>
                  <Typography variant="subtitle2" gutterBottom>
                    Recommendations:
                  </Typography>
                  <List dense>
                    {predictions.insights.recommendations.map((rec: any, idx: number) => (
                      <ListItem key={idx}>
                        <ListItemIcon>
                          <WarningIcon color={rec.priority === 'high' ? 'error' : 'warning'} />
                        </ListItemIcon>
                        <ListItemText
                          primary={rec.action}
                          secondary={rec.reason}
                        />
                      </ListItem>
                    ))}
                  </List>
                </Box>
              )}
            </CardContent>
          </Card>
        </Grid>
      )}

      {/* Recommendations Card */}
      {recommendations && (
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                💡 Recommendations
              </Typography>
              <Box sx={{ mb: 2 }}>
                <Typography variant="body2" gutterBottom>
                  Health Score: {recommendations.overall_health_score}/100
                </Typography>
                <LinearProgress
                  variant="determinate"
                  value={recommendations.overall_health_score}
                  sx={{ height: 8, borderRadius: 4 }}
                />
              </Box>

              {recommendations.recommendations && recommendations.recommendations.length > 0 ? (
                <List dense>
                  {recommendations.recommendations.map((rec: Recommendation, idx: number) => (
                    <ListItem key={idx}>
                      <ListItemIcon>
                        {rec.priority === 'high' ? (
                          <WarningIcon color="error" />
                        ) : (
                          <CheckCircleIcon color="warning" />
                        )}
                      </ListItemIcon>
                      <ListItemText
                        primary={rec.suggestion}
                        secondary={`${rec.parameter}: ${rec.current.toFixed(2)} → ${rec.optimal.toFixed(2)}`}
                      />
                    </ListItem>
                  ))}
                </List>
              ) : (
                <Alert severity="success">
                  All parameters are within optimal range!
                </Alert>
              )}
            </CardContent>
          </Card>
        </Grid>
      )}
    </Grid>
  );
};

export default AIInsights;

