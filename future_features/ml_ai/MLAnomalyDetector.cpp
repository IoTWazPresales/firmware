// MLAnomalyDetector.cpp
// Lightweight statistical anomaly detection implementation

#include "MLAnomalyDetector.h"
#include <cmath>
#include <algorithm>

MLAnomalyDetector::MLAnomalyDetector() {
    _history.reserve(MAX_HISTORY);
}

MLAnomalyDetector::~MLAnomalyDetector() {
    _history.clear();
}

void MLAnomalyDetector::addReading(const SensorReading& reading) {
    _history.push_back(reading);
    
    // Keep only last MAX_HISTORY readings
    if (_history.size() > MAX_HISTORY) {
        _history.erase(_history.begin());
    }
}

float MLAnomalyDetector::calculateMean(const std::vector<float>& values) {
    if (values.empty()) return 0.0;
    
    float sum = 0.0;
    for (float v : values) {
        sum += v;
    }
    return sum / values.size();
}

float MLAnomalyDetector::calculateStdDev(const std::vector<float>& values, float mean) {
    if (values.size() < 2) return 0.0;
    
    float variance = 0.0;
    for (float v : values) {
        variance += (v - mean) * (v - mean);
    }
    variance /= (values.size() - 1);
    
    return sqrt(variance);
}

std::vector<float> MLAnomalyDetector::extractSensorValues(const String& sensorName) {
    std::vector<float> values;
    
    for (const auto& reading : _history) {
        if (sensorName == "temperature") {
            values.push_back(reading.temperature);
        } else if (sensorName == "humidity") {
            values.push_back(reading.humidity);
        } else if (sensorName == "ph") {
            values.push_back(reading.ph);
        } else if (sensorName == "tds") {
            values.push_back(reading.tds);
        } else if (sensorName == "moisture") {
            values.push_back(reading.moisture);
        }
    }
    
    return values;
}

float MLAnomalyDetector::getZScore(const String& sensorName, float value) {
    if (_history.size() < 10) return 0.0;  // Need at least 10 readings
    
    std::vector<float> values = extractSensorValues(sensorName);
    if (values.empty()) return 0.0;
    
    float mean = calculateMean(values);
    float stdDev = calculateStdDev(values, mean);
    
    if (stdDev == 0.0) return 0.0;
    
    return (value - mean) / stdDev;
}

bool MLAnomalyDetector::detectSuddenChange(const String& sensorName, float currentValue) {
    if (_history.size() < 5) return false;
    
    float movingAvg = getMovingAverage(sensorName, 5);
    if (movingAvg == 0.0) return false;
    
    float changePercent = abs((currentValue - movingAvg) / movingAvg);
    return changePercent > _changeThreshold;
}

float MLAnomalyDetector::getMovingAverage(const String& sensorName, int window) {
    std::vector<float> values = extractSensorValues(sensorName);
    
    if (values.size() < window) {
        return calculateMean(values);
    }
    
    // Get last 'window' values
    std::vector<float> recent(values.end() - window, values.end());
    return calculateMean(recent);
}

AnomalyResult MLAnomalyDetector::detectAnomaly(const SensorReading& current) {
    AnomalyResult result;
    result.isAnomaly = false;
    result.anomalyScore = 0.0;
    
    // Check each sensor
    String sensors[] = {"temperature", "humidity", "ph", "tds", "moisture"};
    float values[] = {current.temperature, current.humidity, current.ph, current.tds, current.moisture};
    
    for (int i = 0; i < 5; i++) {
        float zScore = getZScore(sensors[i], values[i]);
        bool suddenChange = detectSuddenChange(sensors[i], values[i]);
        
        if (abs(zScore) > _zScoreThreshold || suddenChange) {
            result.isAnomaly = true;
            result.anomalyScore = abs(zScore);
            result.sensorName = sensors[i];
            
            if (abs(zScore) > _zScoreThreshold) {
                result.reason = "Statistical outlier detected (Z-score: " + String(zScore, 2) + ")";
            } else {
                result.reason = "Sudden change detected";
            }
            
            break;  // Report first anomaly found
        }
    }
    
    return result;
}

void MLAnomalyDetector::reset() {
    _history.clear();
}

