#!/bin/bash
# Railway Deployment Script for NeuroGrow ML Service

echo "🚀 Setting up NeuroGrow ML Service on Railway..."

# Check if Railway CLI is installed
if ! command -v railway &> /dev/null; then
    echo "❌ Railway CLI not found. Installing..."
    npm install -g @railway/cli
fi

# Login to Railway
echo "📝 Logging into Railway..."
railway login

# Create new project
echo "🆕 Creating new Railway project..."
railway init

# Set environment variables
echo "🔐 Setting environment variables..."
read -p "Enter your Supabase URL: " SUPABASE_URL
read -p "Enter your Supabase Key: " SUPABASE_KEY

railway variables set SUPABASE_URL=$SUPABASE_URL
railway variables set SUPABASE_KEY=$SUPABASE_KEY
railway variables set ENVIRONMENT=production
railway variables set TRAIN_ON_STARTUP=true

# Deploy
echo "🚢 Deploying to Railway..."
railway up

echo "✅ Deployment complete!"
echo "📊 Your ML service is now running at:"
railway domain

echo ""
echo "🔗 Next steps:"
echo "1. Update your frontend API_CONFIG to point to the Railway URL"
echo "2. Test the /health endpoint"
echo "3. Train models by calling /api/ml/train"

