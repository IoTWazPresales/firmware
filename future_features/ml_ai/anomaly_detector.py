"""
Anomaly Detection Service for NeuroGrow
Uses Isolation Forest for detecting sensor anomalies
"""
import numpy as np
from sklearn.ensemble import IsolationForest
from sklearn.preprocessing import StandardScaler
import joblib
from datetime import datetime, timedelta
import json

class NeuroGrowAnomalyDetector:
    def __init__(self):
        self.model = IsolationForest(
            contamination=0.1,  # Expect 10% anomalies
            random_state=42,
            n_estimators=100
        )
        self.scaler = StandardScaler()
        self.is_trained = False
    
    def prepare_features(self, sensor_data):
        """
        Convert sensor readings into feature vectors
        Input: List of sensor readings with timestamps
        Output: Feature matrix
        """
        features = []
        
        for reading in sensor_data:
            # Basic features
            feature_vector = [
                reading.get('temperature', 0),
                reading.get('humidity', 0),
                reading.get('ph', 0),
                reading.get('tds', 0),
                reading.get('moisture', 0),
            ]
            
            # Derived features (rate of change)
            if len(features) > 0:
                prev = features[-1]
                feature_vector.extend([
                    feature_vector[0] - prev[0],  # temp change
                    feature_vector[1] - prev[1],  # humidity change
                    feature_vector[2] - prev[2],  # pH change
                ])
            else:
                feature_vector.extend([0, 0, 0])
            
            features.append(feature_vector)
        
        return np.array(features)
    
    def train(self, historical_data):
        """Train the anomaly detection model"""
        features = self.prepare_features(historical_data)
        
        # Scale features
        features_scaled = self.scaler.fit_transform(features)
        
        # Train model
        self.model.fit(features_scaled)
        self.is_trained = True
        
        return self
    
    def detect_anomalies(self, current_readings):
        """Detect anomalies in current sensor readings"""
        if not self.is_trained:
            raise ValueError("Model not trained. Call train() first.")
        
        features = self.prepare_features(current_readings)
        features_scaled = self.scaler.transform(features)
        
        # Predict anomalies (-1 = anomaly, 1 = normal)
        predictions = self.model.predict(features_scaled)
        anomaly_scores = self.model.score_samples(features_scaled)
        
        results = []
        for i, (pred, score) in enumerate(zip(predictions, anomaly_scores)):
            results.append({
                'is_anomaly': pred == -1,
                'anomaly_score': float(score),
                'severity': 'high' if score < -0.5 else 'medium' if score < 0 else 'low',
                'sensor_reading': current_readings[i]
            })
        
        return results
    
    def save_model(self, filepath):
        """Save trained model"""
        joblib.dump({
            'model': self.model,
            'scaler': self.scaler
        }, filepath)
    
    def load_model(self, filepath):
        """Load trained model"""
        data = joblib.load(filepath)
        self.model = data['model']
        self.scaler = data['scaler']
        self.is_trained = True

