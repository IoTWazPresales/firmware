"""
FastAPI Service for NeuroGrow ML/AI
Main API endpoint that serves predictions and recommendations
"""
from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
from typing import List, Optional
from datetime import datetime
import supabase
from anomaly_detector import NeuroGrowAnomalyDetector
from predictor import NeuroGrowPredictor
from recommendation_engine import NeuroGrowRecommendationEngine

app = FastAPI(title="NeuroGrow ML Service")

# CORS middleware
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Initialize ML models
anomaly_detector = NeuroGrowAnomalyDetector()
predictor = NeuroGrowPredictor()
recommendation_engine = NeuroGrowRecommendationEngine()

# Supabase client (for fetching historical data)
supabase_client = supabase.create_client(
    "YOUR_SUPABASE_URL",
    "YOUR_SUPABASE_KEY"
)

class SensorReading(BaseModel):
    temperature: Optional[float] = None
    humidity: Optional[float] = None
    ph: Optional[float] = None
    tds: Optional[float] = None
    moisture: Optional[float] = None
    timestamp: datetime

class PredictionRequest(BaseModel):
    device_id: str
    sensor_type: str = "temperature"
    hours_ahead: int = 6

@app.get("/health")
async def health_check():
    return {"status": "healthy", "service": "NeuroGrow ML"}

@app.post("/api/ml/anomaly/detect")
async def detect_anomalies(readings: List[SensorReading]):
    """Detect anomalies in sensor readings"""
    try:
        readings_dict = [r.dict() for r in readings]
        results = anomaly_detector.detect_anomalies(readings_dict)
        return {
            "anomalies": results,
            "anomaly_count": sum(1 for r in results if r['is_anomaly'])
        }
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@app.post("/api/ml/predict")
async def predict_future(request: PredictionRequest):
    """Predict future sensor values"""
    try:
        # Fetch recent historical data from Supabase
        response = supabase_client.table('sensor_logs')\
            .select('*')\
            .eq('device_id', request.device_id)\
            .order('timestamp', desc=True)\
            .limit(48)\
            .execute()
        
        historical_data = [dict(row) for row in response.data]
        
        if len(historical_data) < 24:
            raise HTTPException(
                status_code=400,
                detail="Not enough historical data. Need at least 24 hours."
            )
        
        # Get predictions
        predictions = predictor.predict(historical_data, request.sensor_type)
        insights = predictor.get_prediction_insights(historical_data, request.sensor_type)
        
        return {
            "predictions": predictions,
            "insights": insights,
            "sensor_type": request.sensor_type,
            "forecast_hours": request.hours_ahead
        }
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@app.post("/api/ml/recommendations")
async def get_recommendations(
    current_readings: SensorReading,
    plant_type: str = "general"
):
    """Get optimization recommendations"""
    try:
        readings_dict = current_readings.dict()
        recommendations = recommendation_engine.get_recommendations(
            readings_dict,
            plant_type
        )
        return recommendations
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@app.post("/api/ml/train")
async def train_models(device_id: str):
    """Retrain models with latest data"""
    try:
        # Fetch all historical data
        response = supabase_client.table('sensor_logs')\
            .select('*')\
            .eq('device_id', device_id)\
            .order('timestamp', desc=False)\
            .limit(10000)\
            .execute()
        
        historical_data = [dict(row) for row in response.data]
        
        if len(historical_data) < 100:
            raise HTTPException(
                status_code=400,
                detail="Not enough data for training. Need at least 100 readings."
            )
        
        # Train models
        anomaly_detector.train(historical_data)
        predictor.train(historical_data, 'temperature')
        
        return {
            "status": "success",
            "message": "Models trained successfully",
            "data_points": len(historical_data)
        }
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)

