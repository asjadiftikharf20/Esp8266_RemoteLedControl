#ifndef MPU6050_CALIBRATION_H
#define MPU6050_CALIBRATION_H

#include "MPU6050.h"
#include "Wire.h"

class MPU6050Calibration {
  public:
    MPU6050Calibration(MPU6050& sensor);
    void calibrate();  // The function to call from main.cpp
    
  private:
    MPU6050& accelgyro;
    int buffersize = 1000;    // Amount of readings for averaging
    int acel_deadzone = 8;    // Error allowed for accelerometer
    int giro_deadzone = 1;    // Error allowed for gyroscope

    int16_t ax, ay, az, gx, gy, gz;
    int mean_ax, mean_ay, mean_az, mean_gx, mean_gy, mean_gz;
    int ax_offset, ay_offset, az_offset, gx_offset, gy_offset, gz_offset;

    void meansensors();
    void calibration();
};

#endif
