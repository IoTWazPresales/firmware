# NeuroGrow ML/AI Features - Future Implementation

This folder contains all ML/AI related code and documentation for future implementation.

## ⚠️ Status: Not Yet Implemented

These files are kept here for reference and will be implemented after completing the core NeuroGrow features.

## Contents

### Python ML Service
- `api_service.py` - FastAPI service for ML predictions
- `anomaly_detector.py` - Isolation Forest anomaly detection
- `predictor.py` - LSTM time-series prediction
- `recommendation_engine.py` - Optimization recommendations
- `requirements.txt` - Python dependencies
- `DEPLOYMENT_GUIDE.md` - Complete deployment instructions

### ESP32 Edge ML
- `MLAnomalyDetector.h` - Lightweight C++ anomaly detector header
- `MLAnomalyDetector.cpp` - Statistical anomaly detection implementation

### Frontend Components
- `AIInsights.tsx` - React component for displaying ML insights

### Deployment Files
- `Dockerfile` - Docker container configuration
- `docker-compose.yml` - Docker Compose setup
- `railway.json` - Railway deployment config
- `setup_railway.sh` - Railway deployment script

## When Ready to Implement

1. Review `DEPLOYMENT_GUIDE.md` for setup instructions
2. Choose deployment platform (Railway recommended)
3. Set up Supabase database tables
4. Deploy ML service
5. Integrate with frontend
6. Train initial models

## Notes

- All code is ready to use but not integrated into main codebase
- No ESP32 memory impact until edge ML is integrated
- Cloud ML runs independently on separate server
- Frontend component ready but not imported anywhere

