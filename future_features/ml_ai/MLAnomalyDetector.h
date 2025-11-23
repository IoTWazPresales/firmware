// MLAnomalyDetector.h
// Lightweight statistical anomaly detection for ESP32
// No ML libraries needed - pure statistical algorithms

#ifndef ML_ANOMALY_DETECTOR_H
#define ML_ANOMALY_DETECTOR_H

#include <Arduino.h>
#include <vector>

struct SensorReading {
    float temperature;
    float humidity;
    float ph;
    float tds;
    float moisture;
    unsigned long timestamp;
};

struct AnomalyResult {
    bool isAnomaly;
    float anomalyScore;
    String sensorName;
    String reason;
};

class MLAnomalyDetector {
public:
    MLAnomalyDetector();
    ~MLAnomalyDetector();
    
    // Add reading to history (keeps last 100 readings)
    void addReading(const SensorReading& reading);
    
    // Detect anomalies using statistical methods
    AnomalyResult detectAnomaly(const SensorReading& current);
    
    // Get Z-score for a value (statistical outlier detection)
    float getZScore(const String& sensorName, float value);
    
    // Check for sudden changes (rate of change detection)
    bool detectSuddenChange(const String& sensorName, float currentValue);
    
    // Get moving average for a sensor
    float getMovingAverage(const String& sensorName, int window = 10);
    
    // Reset history
    void reset();

private:
    std::vector<SensorReading> _history;
    static const int MAX_HISTORY = 100;
    
    // Statistical calculations
    float calculateMean(const std::vector<float>& values);
    float calculateStdDev(const std::vector<float>& values, float mean);
    std::vector<float> extractSensorValues(const String& sensorName);
    
    // Thresholds (can be adjusted)
    float _zScoreThreshold = 3.0;  // 3 standard deviations = anomaly
    float _changeThreshold = 0.2;  // 20% change = sudden change
};

#endif

