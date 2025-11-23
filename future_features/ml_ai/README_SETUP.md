# NeuroGrow ML Service Setup Guide

## Overview
This guide walks you through setting up the cloud-based ML service for NeuroGrow. The service runs independently and provides predictions, anomaly detection, and recommendations via REST API.

## Architecture

```
┌─────────────┐
│  NeuroGrow  │ → Sends data to Supabase
│   (ESP32)   │
└──────┬──────┘
       │
       ▼
┌─────────────┐
│  Supabase   │ ← ML Service reads from here
│  (Database)  │
└──────┬──────┘
       │
       ▼
┌─────────────┐
│  ML Service │ → Provides predictions via API
│  (Python)   │
└──────┬──────┘
       │
       ▼
┌─────────────┐
│  Frontend   │ ← Displays insights
│  (React)    │
└─────────────┘
```

## Option 1: Deploy on Supabase Edge Functions (Recommended)

### Why Supabase Edge Functions?
- ✅ Free tier available
- ✅ Automatic scaling
- ✅ Integrated with your existing Supabase database
- ✅ No server management
- ✅ Fast deployment

### Setup Steps

1. **Install Supabase CLI**
```bash
npm install -g supabase
```

2. **Initialize Supabase Functions**
```bash
supabase init
supabase functions new ml-service
```

3. **Create Function Structure**
```
supabase/
  functions/
    ml-service/
      index.ts          # Main function handler
      requirements.txt   # Python dependencies
      anomaly_detector.py
      predictor.py
      recommendation_engine.py
```

4. **Deploy**
```bash
supabase functions deploy ml-service
```

## Option 2: Deploy on Railway (Easiest)

### Why Railway?
- ✅ One-click deployment from GitHub
- ✅ Free tier ($5 credit/month)
- ✅ Automatic HTTPS
- ✅ Simple environment variables

### Setup Steps

1. **Create Railway Account**
   - Go to https://railway.app
   - Sign up with GitHub

2. **Create New Project**
   - Click "New Project"
   - Select "Deploy from GitHub repo"

3. **Configure Environment Variables**
```
SUPABASE_URL=your_supabase_url
SUPABASE_KEY=your_supabase_key
PORT=8000
```

4. **Deploy**
   - Railway auto-detects Python
   - Runs `pip install -r requirements.txt`
   - Starts FastAPI server

## Option 3: Deploy on AWS Lambda (Serverless)

### Why AWS Lambda?
- ✅ Pay per request (very cheap)
- ✅ Auto-scaling
- ✅ No server management
- ✅ Integrates with API Gateway

### Setup Steps

1. **Install Serverless Framework**
```bash
npm install -g serverless
```

2. **Create serverless.yml**
```yaml
service: neurogrow-ml

provider:
  name: aws
  runtime: python3.9
  region: us-east-1
  environment:
    SUPABASE_URL: ${env:SUPABASE_URL}
    SUPABASE_KEY: ${env:SUPABASE_KEY}

functions:
  predict:
    handler: handler.predict
    events:
      - http:
          path: /api/ml/predict
          method: post
  recommendations:
    handler: handler.recommendations
    events:
      - http:
          path: /api/ml/recommendations
          method: post
```

3. **Deploy**
```bash
serverless deploy
```

## Option 4: Deploy on Docker (Any Cloud)

### Why Docker?
- ✅ Works anywhere (AWS, GCP, Azure, DigitalOcean)
- ✅ Consistent environment
- ✅ Easy to scale

### Setup Steps

1. **Create Dockerfile**
```dockerfile
FROM python:3.9-slim

WORKDIR /app

COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

COPY . .

EXPOSE 8000

CMD ["uvicorn", "api_service:app", "--host", "0.0.0.0", "--port", "8000"]
```

2. **Create docker-compose.yml**
```yaml
version: '3.8'

services:
  ml-service:
    build: .
    ports:
      - "8000:8000"
    environment:
      - SUPABASE_URL=${SUPABASE_URL}
      - SUPABASE_KEY=${SUPABASE_KEY}
    restart: unless-stopped
```

3. **Deploy**
```bash
docker-compose up -d
```

## Recommended: Railway Setup (Step-by-Step)

Let me create the complete Railway deployment package.

