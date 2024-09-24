#include "MPU6050_Calibration.h"

// Constructor: Initialize with the MPU6050 sensor instance
MPU6050Calibration::MPU6050Calibration(MPU6050& sensor) : accelgyro(sensor) {}

// Public function to start calibration
void MPU6050Calibration::calibrate() {
    Wire.begin();  // Start I2C communication
    Serial.println("Starting MPU6050 Calibration...");

    // Reset offsets
    accelgyro.setXAccelOffset(0);
    accelgyro.setYAccelOffset(0);
    accelgyro.setZAccelOffset(0);
    accelgyro.setXGyroOffset(0);
    accelgyro.setYGyroOffset(0);
    accelgyro.setZGyroOffset(0);

    // Step 1: Read sensors and calculate means
    Serial.println("Reading sensor data...");
    meansensors();
    
    // Step 2: Perform calibration
    Serial.println("Calibrating offsets...");
    calibration();
    
    // Step 3: Display results
    Serial.println("Calibration complete!");
    Serial.println("Final sensor readings with offsets:");
    Serial.print("Accel X: "); Serial.println(mean_ax);
    Serial.print("Accel Y: "); Serial.println(mean_ay);
    Serial.print("Accel Z: "); Serial.println(mean_az);
    Serial.print("Gyro X: "); Serial.println(mean_gx);
    Serial.print("Gyro Y: "); Serial.println(mean_gy);
    Serial.print("Gyro Z: "); Serial.println(mean_gz);

    // Print calculated offsets
    Serial.println("Your offsets are:");
    Serial.print("Accel X Offset: "); Serial.println(ax_offset);
    Serial.print("Accel Y Offset: "); Serial.println(ay_offset);
    Serial.print("Accel Z Offset: "); Serial.println(az_offset);
    Serial.print("Gyro X Offset: "); Serial.println(gx_offset);
    Serial.print("Gyro Y Offset: "); Serial.println(gy_offset);
    Serial.print("Gyro Z Offset: "); Serial.println(gz_offset);
}

void MPU6050Calibration::meansensors() {
    long buff_ax = 0, buff_ay = 0, buff_az = 0, buff_gx = 0, buff_gy = 0, buff_gz = 0;
    for (int i = 0; i < buffersize + 100; i++) {
        accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
        if (i > 100) {  // Discard first 100 readings
            buff_ax += ax;
            buff_ay += ay;
            buff_az += az;
            buff_gx += gx;
            buff_gy += gy;
            buff_gz += gz;
        }
        delay(2);
    }
    mean_ax = buff_ax / buffersize;
    mean_ay = buff_ay / buffersize;
    mean_az = buff_az / buffersize;
    mean_gx = buff_gx / buffersize;
    mean_gy = buff_gy / buffersize;
    mean_gz = buff_gz / buffersize;
}

void MPU6050Calibration::calibration() {
    ax_offset = -mean_ax / 8;
    ay_offset = -mean_ay / 8;
    az_offset = (16384 - mean_az) / 8;
    gx_offset = -mean_gx / 4;
    gy_offset = -mean_gy / 4;
    gz_offset = -mean_gz / 4;

    while (true) {
        int ready = 0;
        accelgyro.setXAccelOffset(ax_offset);
        accelgyro.setYAccelOffset(ay_offset);
        accelgyro.setZAccelOffset(az_offset);
        accelgyro.setXGyroOffset(gx_offset);
        accelgyro.setYGyroOffset(gy_offset);
        accelgyro.setZGyroOffset(gz_offset);

        meansensors();

        if (abs(mean_ax) <= acel_deadzone) ready++;
        else ax_offset = ax_offset - mean_ax / acel_deadzone;

        if (abs(mean_ay) <= acel_deadzone) ready++;
        else ay_offset = ay_offset - mean_ay / acel_deadzone;

        if (abs(16384 - mean_az) <= acel_deadzone) ready++;
        else az_offset = az_offset + (16384 - mean_az) / acel_deadzone;

        if (abs(mean_gx) <= giro_deadzone) ready++;
        else gx_offset = gx_offset - mean_gx / (giro_deadzone + 1);

        if (abs(mean_gy) <= giro_deadzone) ready++;
        else gy_offset = gy_offset - mean_gy / (giro_deadzone + 1);

        if (abs(mean_gz) <= giro_deadzone) ready++;
        else gz_offset = gz_offset - mean_gz / (giro_deadzone + 1);

        if (ready == 6) break;  // Exit when all axes are calibrated
    }
}
