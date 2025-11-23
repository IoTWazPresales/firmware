"""
Time-Series Prediction Service for NeuroGrow
Uses LSTM for predicting future sensor values
"""
import numpy as np
from tensorflow import keras
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import LSTM, Dense, Dropout
from sklearn.preprocessing import MinMaxScaler
from datetime import datetime, timedelta
import json

class NeuroGrowPredictor:
    def __init__(self, lookback=24, forecast_hours=6):
        """
        lookback: Number of past hours to use for prediction
        forecast_hours: Hours into future to predict
        """
        self.lookback = lookback
        self.forecast_hours = forecast_hours
        self.model = None
        self.scaler = MinMaxScaler(feature_range=(0, 1))
        self.is_trained = False
    
    def create_sequences(self, data, lookback):
        """Create sequences for LSTM training"""
        X, y = [], []
        for i in range(lookback, len(data)):
            X.append(data[i-lookback:i])
            y.append(data[i])
        return np.array(X), np.array(y)
    
    def prepare_data(self, historical_data, target_sensor='temperature'):
        """Prepare time-series data from historical readings"""
        # Extract target sensor values
        values = [reading.get(target_sensor, 0) for reading in historical_data]
        
        # Create sequences
        scaled = self.scaler.fit_transform(np.array(values).reshape(-1, 1))
        
        return scaled
    
    def build_model(self, input_shape):
        """Build LSTM model architecture"""
        model = Sequential([
            LSTM(50, return_sequences=True, input_shape=input_shape),
            Dropout(0.2),
            LSTM(50, return_sequences=True),
            Dropout(0.2),
            LSTM(50),
            Dropout(0.2),
            Dense(1)
        ])
        
        model.compile(
            optimizer='adam',
            loss='mean_squared_error',
            metrics=['mae']
        )
        
        return model
    
    def train(self, historical_data, target_sensor='temperature', epochs=50):
        """Train prediction model"""
        # Prepare data
        scaled_data = self.prepare_data(historical_data, target_sensor)
        
        # Create sequences
        X, y = self.create_sequences(scaled_data, self.lookback)
        
        # Reshape for LSTM (samples, timesteps, features)
        X = X.reshape((X.shape[0], X.shape[1], 1))
        
        # Build and train model
        self.model = self.build_model((X.shape[1], 1))
        
        self.model.fit(
            X, y,
            epochs=epochs,
            batch_size=32,
            validation_split=0.2,
            verbose=1
        )
        
        self.is_trained = True
        return self
    
    def predict(self, recent_readings, target_sensor='temperature'):
        """Predict future values"""
        if not self.is_trained:
            raise ValueError("Model not trained. Call train() first.")
        
        # Get last lookback hours
        values = [r.get(target_sensor, 0) for r in recent_readings[-self.lookback:]]
        scaled = self.scaler.transform(np.array(values).reshape(-1, 1))
        
        # Reshape for prediction
        X = scaled.reshape((1, self.lookback, 1))
        
        # Predict
        predictions = []
        current_input = X
        
        for _ in range(self.forecast_hours):
            pred = self.model.predict(current_input, verbose=0)
            predictions.append(float(self.scaler.inverse_transform(pred)[0][0]))
            
            # Update input for next prediction
            current_input = np.append(
                current_input[:, 1:, :],
                pred.reshape(1, 1, 1),
                axis=1
            )
        
        return predictions
    
    def get_prediction_insights(self, recent_readings, target_sensor='temperature'):
        """Get actionable insights from predictions"""
        predictions = self.predict(recent_readings, target_sensor)
        current_value = recent_readings[-1].get(target_sensor, 0)
        
        # Calculate trends
        trend = 'increasing' if predictions[-1] > current_value else 'decreasing'
        change_magnitude = abs(predictions[-1] - current_value)
        
        # Generate recommendations
        insights = {
            'current_value': current_value,
            'predicted_value_6h': predictions[-1],
            'trend': trend,
            'change_magnitude': change_magnitude,
            'recommendations': []
        }
        
        if target_sensor == 'ph':
            if predictions[-1] < 5.5:
                insights['recommendations'].append({
                    'action': 'Add pH up solution',
                    'priority': 'high',
                    'reason': f'pH predicted to drop to {predictions[-1]:.2f} in 6 hours'
                })
            elif predictions[-1] > 7.5:
                insights['recommendations'].append({
                    'action': 'Add pH down solution',
                    'priority': 'high',
                    'reason': f'pH predicted to rise to {predictions[-1]:.2f} in 6 hours'
                })
        
        elif target_sensor == 'temperature':
            if predictions[-1] > 30:
                insights['recommendations'].append({
                    'action': 'Increase ventilation or reduce lighting',
                    'priority': 'medium',
                    'reason': f'Temperature predicted to reach {predictions[-1]:.1f}°C'
                })
        
        return insights

