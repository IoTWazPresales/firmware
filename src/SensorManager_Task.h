#ifndef SENSOR_MANAGER_TASK_H
#define SENSOR_MANAGER_TASK_H

#include <Arduino.h>
class SensorManager;

// Prototype your FreeRTOS entry point here:
void Sensor_Manager(void* parameter);
void startSensorManagerTask(SensorManager* manager);


#endif  // SENSOR_MANAGER_TASK_H
