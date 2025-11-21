"""
Recommendation Engine for NeuroGrow
Provides optimal parameter suggestions based on plant type and conditions
"""
import numpy as np
from sklearn.cluster import KMeans
from sklearn.preprocessing import StandardScaler
import json
from datetime import datetime

class NeuroGrowRecommendationEngine:
    def __init__(self):
        self.scaler = StandardScaler()
        self.optimal_configs = {}  # Plant type -> optimal parameters
    
    def learn_from_successful_grows(self, historical_data):
        """
        Learn optimal configurations from successful grows
        Uses clustering to find patterns in successful configurations
        """
        # Filter successful grows (e.g., high yield, healthy plants)
        successful_grows = [
            grow for grow in historical_data
            if grow.get('success_score', 0) > 0.8
        ]
        
        if len(successful_grows) < 5:
            return  # Not enough data
        
        # Extract features
        features = []
        for grow in successful_grows:
            features.append([
                grow.get('avg_temperature', 0),
                grow.get('avg_humidity', 0),
                grow.get('avg_ph', 0),
                grow.get('avg_ec', 0),
                grow.get('light_hours', 0),
            ])
        
        features = np.array(features)
        features_scaled = self.scaler.fit_transform(features)
        
        # Cluster to find optimal configurations
        kmeans = KMeans(n_clusters=3, random_state=42)
        clusters = kmeans.fit_predict(features_scaled)
        
        # Find best cluster (highest average success)
        cluster_scores = {}
        for i, cluster in enumerate(clusters):
            if cluster not in cluster_scores:
                cluster_scores[cluster] = []
            cluster_scores[cluster].append(successful_grows[i]['success_score'])
        
        best_cluster = max(cluster_scores, key=lambda k: np.mean(cluster_scores[k]))
        
        # Extract optimal parameters from best cluster
        best_grows = [successful_grows[i] for i, c in enumerate(clusters) if c == best_cluster]
        
        optimal = {
            'temperature': np.mean([g.get('avg_temperature', 0) for g in best_grows]),
            'humidity': np.mean([g.get('avg_humidity', 0) for g in best_grows]),
            'ph': np.mean([g.get('avg_ph', 0) for g in best_grows]),
            'ec': np.mean([g.get('avg_ec', 0) for g in best_grows]),
            'light_hours': np.mean([g.get('light_hours', 0) for g in best_grows]),
        }
        
        return optimal
    
    def get_recommendations(self, current_readings, plant_type='general', target_yield='high'):
        """
        Get recommendations based on current conditions
        """
        recommendations = []
        
        # Get optimal for plant type
        optimal = self.optimal_configs.get(plant_type, {
            'temperature': 24.0,
            'humidity': 60.0,
            'ph': 6.5,
            'ec': 1.8,
            'light_hours': 18,
        })
        
        current_temp = current_readings.get('temperature', 0)
        current_humidity = current_readings.get('humidity', 0)
        current_ph = current_readings.get('ph', 0)
        current_ec = current_readings.get('tds', 0) / 500  # Convert TDS to EC
        
        # Temperature recommendations
        temp_diff = current_temp - optimal['temperature']
        if abs(temp_diff) > 2:
            recommendations.append({
                'parameter': 'temperature',
                'current': current_temp,
                'optimal': optimal['temperature'],
                'action': 'decrease' if temp_diff > 0 else 'increase',
                'priority': 'high' if abs(temp_diff) > 5 else 'medium',
                'suggestion': f"Adjust temperature by {abs(temp_diff):.1f}°C to reach optimal {optimal['temperature']}°C"
            })
        
        # pH recommendations
        ph_diff = current_ph - optimal['ph']
        if abs(ph_diff) > 0.3:
            recommendations.append({
                'parameter': 'ph',
                'current': current_ph,
                'optimal': optimal['ph'],
                'action': 'decrease' if ph_diff > 0 else 'increase',
                'priority': 'high' if abs(ph_diff) > 0.5 else 'medium',
                'suggestion': f"Adjust pH by {abs(ph_diff):.2f} to reach optimal {optimal['ph']}"
            })
        
        # Humidity recommendations
        humidity_diff = current_humidity - optimal['humidity']
        if abs(humidity_diff) > 10:
            recommendations.append({
                'parameter': 'humidity',
                'current': current_humidity,
                'optimal': optimal['humidity'],
                'action': 'decrease' if humidity_diff > 0 else 'increase',
                'priority': 'medium',
                'suggestion': f"Adjust humidity by {abs(humidity_diff):.1f}% to reach optimal {optimal['humidity']}%"
            })
        
        # Nutrient recommendations (EC/TDS)
        ec_diff = current_ec - optimal['ec']
        if abs(ec_diff) > 0.3:
            recommendations.append({
                'parameter': 'nutrients',
                'current': current_ec,
                'optimal': optimal['ec'],
                'action': 'decrease' if ec_diff > 0 else 'increase',
                'priority': 'medium',
                'suggestion': f"Adjust nutrient concentration (EC: {current_ec:.2f} → {optimal['ec']:.2f})"
            })
        
        return {
            'recommendations': recommendations,
            'overall_health_score': self._calculate_health_score(current_readings, optimal),
            'optimal_config': optimal
        }
    
    def _calculate_health_score(self, current, optimal):
        """Calculate overall health score (0-100)"""
        scores = []
        
        # Temperature score
        temp_diff = abs(current.get('temperature', 0) - optimal['temperature'])
        temp_score = max(0, 100 - (temp_diff * 10))
        scores.append(temp_score)
        
        # pH score
        ph_diff = abs(current.get('ph', 0) - optimal['ph'])
        ph_score = max(0, 100 - (ph_diff * 50))
        scores.append(ph_score)
        
        # Humidity score
        humidity_diff = abs(current.get('humidity', 0) - optimal['humidity'])
        humidity_score = max(0, 100 - (humidity_diff * 2))
        scores.append(humidity_score)
        
        return int(np.mean(scores))

