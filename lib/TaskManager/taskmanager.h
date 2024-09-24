#ifndef TASKMANAGER_H
#define TASKMANAGER_H

// Include the sensor library
#include "sensors.h"
#include "mpu6050_calibration.h"
#include "config.h"
#include "WifiConnection.h"
#include "NTPManager.h"
#include "MQTTmanager.h"

#define ledPin D4

// Function prototypes
void run_tasks_OneTime();
void run_tasks_continuous();


#endif
