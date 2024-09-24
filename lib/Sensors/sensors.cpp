#include "sensors.h"
#include "config.h"

MPU6050 mpu;

// initializing I2C for the mpu6050
void initialize_sensors() {
    Wire.begin();               // Initialize I2C communication
    mpu.initialize();               // Initialize the MPU6050 sensor
    // Add specific initialization for each GY80 sensor component here (ADXL345, etc.)
}



void task_sensor_lookout() {
  Serial. println ( "\nI2C Scanner" ) ;
  
  byte error, address;
  int nDevices;
  Serial. println ( "Scanning..." ) ;
  nDevices = 0;
  for ( address = 1; address < 127; address++ ) 
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmission to see if
    // a device did acknowledge to the address.
    Wire. beginTransmission ( address ) ;
    error = Wire. endTransmission () ;
    if ( error == 0 ) 
    {
      Serial. print ( "I2C device found at address 0x" ) ;
      if ( address < 16 )  
        Serial. print ( "0" ) ;
      Serial. print ( address,HEX ) ;
      Serial. println ( " !" ) ;
      nDevices++;
    }  
  }
  if ( nDevices == 0 ) 
    Serial. println ( "No I2C devices found\n" ) ;
  else
    Serial. println ( "done\n" ) ;
  delay ( 1000 ) ;
}

#define ACCELEROMETER_CALIBRATION_X         -473
#define ACCELEROMETER_CALIBRATION_Y         0
#define ACCELEROMETER_CALIBRATION_Z         -16390
#define GYROSCOPE_CALIBRATION_X             530
#define GYROSCOPE_CALIBRATION_Y             -260
#define GYROSCOPE_CALIBRATION_Z             17


// Function to read data from mpu6050 sensor
String task_read_mpu6050_data() {
  int16_t axRaw, ayRaw, azRaw;
  int16_t gxRaw, gyRaw, gzRaw;

  mpu.getAcceleration(&axRaw, &ayRaw, &azRaw);
  mpu.getRotation(&gxRaw, &gyRaw, &gzRaw);

  // Apply calibration offsets
  float ax = (axRaw + ACCELEROMETER_CALIBRATION_X) / 16384.0;
  float ay = (ayRaw + ACCELEROMETER_CALIBRATION_Y) / 16384.0;
  float az = (azRaw + ACCELEROMETER_CALIBRATION_Z) / 16384.0;

  float gx = (gxRaw + GYROSCOPE_CALIBRATION_X) / 131.0;
  float gy = (gyRaw + GYROSCOPE_CALIBRATION_Y) / 131.0;
  float gz = (gzRaw + GYROSCOPE_CALIBRATION_Z) / 131.0;

  // Get JSON formatted data
    String jsonData = getJsonData(ax, ay, az, gx, gy, gz);

    // Print the JSON data to the Serial Monitor
    Serial.println(jsonData);
    return jsonData;
    delay(5000);
}