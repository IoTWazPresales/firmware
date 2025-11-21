# NeuroGrow ML Service - Complete Deployment Guide

## Quick Start: Railway (Recommended for Beginners)

### Step 1: Prepare Your Code
1. Create a new GitHub repository
2. Copy all files from `ml_service_examples/` to the repo
3. Commit and push

### Step 2: Deploy to Railway
1. Go to https://railway.app
2. Click "New Project" → "Deploy from GitHub repo"
3. Select your repository
4. Railway will auto-detect Python and start building

### Step 3: Configure Environment Variables
In Railway dashboard, go to "Variables" tab and add:
```
SUPABASE_URL=https://your-project.supabase.co
SUPABASE_KEY=your-anon-key-here
TRAIN_ON_STARTUP=false
```

### Step 4: Get Your Service URL
1. Railway will provide a URL like: `https://your-service.railway.app`
2. Copy this URL

### Step 5: Update Frontend
In `fissioninterface/src/api/apiConfig.ts`, add:
```typescript
ML_SERVICE: {
  BASE_URL: 'https://your-service.railway.app',
  PREDICT: `${ML_SERVICE.BASE_URL}/api/ml/predict`,
  RECOMMENDATIONS: `${ML_SERVICE.BASE_URL}/api/ml/recommendations`,
  ANOMALY: `${ML_SERVICE.BASE_URL}/api/ml/anomaly/detect`,
  TRAIN: `${ML_SERVICE.BASE_URL}/api/ml/train`,
}
```

## Alternative: Docker Deployment

### Step 1: Build Docker Image
```bash
cd ml_service_examples
docker build -t neurogrow-ml .
```

### Step 2: Run Container
```bash
docker run -d \
  -p 8000:8000 \
  -e SUPABASE_URL=your_url \
  -e SUPABASE_KEY=your_key \
  --name neurogrow-ml \
  neurogrow-ml
```

### Step 3: Deploy to Cloud
- **DigitalOcean**: Use App Platform or Droplets
- **AWS**: Use ECS or EC2
- **Google Cloud**: Use Cloud Run or Compute Engine
- **Azure**: Use Container Instances

## Database Setup (Supabase)

### Step 1: Create Tables
Run this SQL in Supabase SQL Editor:

```sql
-- Sensor logs table (if not exists)
CREATE TABLE IF NOT EXISTS sensor_logs (
  id BIGSERIAL PRIMARY KEY,
  device_id TEXT NOT NULL,
  timestamp TIMESTAMPTZ NOT NULL DEFAULT NOW(),
  temperature FLOAT,
  humidity FLOAT,
  ph FLOAT,
  tds FLOAT,
  moisture FLOAT,
  created_at TIMESTAMPTZ DEFAULT NOW()
);

-- Create index for faster queries
CREATE INDEX IF NOT EXISTS idx_sensor_logs_device_timestamp 
ON sensor_logs(device_id, timestamp DESC);

-- Enable Row Level Security (optional)
ALTER TABLE sensor_logs ENABLE ROW LEVEL SECURITY;

-- Policy to allow service role access
CREATE POLICY "Service role can access all" 
ON sensor_logs FOR ALL 
USING (auth.role() = 'service_role');
```

### Step 2: Get API Credentials
1. Go to Supabase Dashboard → Settings → API
2. Copy:
   - Project URL (SUPABASE_URL)
   - Service Role Key (SUPABASE_KEY) - for ML service
   - Anon Key - for frontend

## Testing the Service

### Step 1: Check Health
```bash
curl https://your-service.railway.app/health
```

Expected response:
```json
{
  "status": "healthy",
  "service": "NeuroGrow ML",
  "models_trained": false
}
```

### Step 2: Train Models
```bash
curl -X POST https://your-service.railway.app/api/ml/train \
  -H "Content-Type: application/json" \
  -d '{"device_id": "your-device-id"}'
```

### Step 3: Get Predictions
```bash
curl -X POST https://your-service.railway.app/api/ml/predict \
  -H "Content-Type: application/json" \
  -d '{
    "device_id": "your-device-id",
    "sensor_type": "temperature",
    "hours_ahead": 6
  }'
```

## Monitoring & Maintenance

### Automatic Model Retraining
Set up a cron job or scheduled task to retrain models:

```bash
# Daily retraining at 2 AM
0 2 * * * curl -X POST https://your-service.railway.app/api/ml/train \
  -H "Content-Type: application/json" \
  -d '{"device_id": "your-device-id"}'
```

### Health Monitoring
Use Railway's built-in monitoring or set up:
- Uptime monitoring (UptimeRobot, Pingdom)
- Error tracking (Sentry)
- Log aggregation (Logtail, Papertrail)

## Cost Estimates

| Platform | Free Tier | Paid Tier |
|----------|-----------|-----------|
| Railway | $5 credit/month | $0.01/GB RAM-hour |
| AWS Lambda | 1M requests/month | $0.20 per 1M requests |
| DigitalOcean | - | $12/month (basic droplet) |
| Supabase | 500MB database | $25/month (Pro) |

**Estimated monthly cost**: $0-25 depending on usage

## Troubleshooting

### Models not training
- Check Supabase connection
- Verify device_id exists in database
- Ensure at least 100 data points available

### Predictions inaccurate
- Retrain models with more data
- Check data quality in Supabase
- Verify sensor readings are valid

### Service not responding
- Check Railway logs
- Verify environment variables
- Check Supabase API limits

## Next Steps

1. ✅ Deploy ML service
2. ✅ Train initial models
3. ✅ Integrate with frontend
4. ✅ Set up automatic retraining
5. ✅ Monitor performance

