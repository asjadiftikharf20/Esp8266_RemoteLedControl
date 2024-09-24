#ifndef SENSORS_H
#define SENSORS_H

#include <Wire.h>
#include <MPU6050.h>
#include "JSONmanager.h"
#include "config.h"


// Function prototypes for sensor initialization and data retrieval
void initialize_sensors();
void task_sensor_lookout();
String task_read_mpu6050_data();     //Gyro and Accelerometer

#endif
