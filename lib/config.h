// config.h

#ifndef CONFIG_H
#define CONFIG_H

#include <Wire.h>
#include <MPU6050.h>
#include <Arduino.h>
#include <EEPROM.h>
#include <WiFiClientSecure.h>
#include <base64.h>
#include <bearssl/bearssl.h>
#include <ESP8266WiFi.h>
#include <bearssl/bearssl_hmac.h>
#include <libb64/cdecode.h>
#include <Arduino_JSON.h>
#include <PubSubClient.h>
#include <sensors.h>
#include <TaskManager.h>
#include <MQTTmanager.h>

// Azure IoT SDK for C includes
#include <az_core.h>
#include <az_iot.h>
#include <azure_ca.h>

// C99 libraries
#include <cstdlib>
#include <stdbool.h>
#include <string.h>
#include <time.h>

// WiFi Credentials
#define IOT_CONFIG_WIFI_SSID "Hypernym_IoT"
#define IOT_CONFIG_WIFI_PASSWORD "Hypernym@ISB"

// Azure IoT
#define IOT_CONFIG_IOTHUB_FQDN "iothubdevuae.azure-devices.net"
#define IOT_CONFIG_DEVICE_ID "Asjad-Led_testModule"
#define IOT_CONFIG_DEVICE_KEY "YgeF8n1ai+IkxtzS8NglZ+m6V8odoqCTPAIoTJyHoVs="

// Publish 1 message every 2 seconds
#define TELEMETRY_FREQUENCY_MILLISECS 2000
#endif

// When developing for your own Arduino-based platform,
// please follow the format '(ard;<platform>)'.
#define AZURE_SDK_CLIENT_USER_AGENT "c%2F" AZ_SDK_VERSION_STRING "(ard;esp8266)"

// Utility macros and defines
#define LED_PIN D4
#define sizeofarray(a) (sizeof(a) / sizeof(a[0]))
#define ONE_HOUR_IN_SECS 3600
#define NTP_SERVERS "pool.ntp.org", "time.nist.gov"
#define MQTT_PACKET_SIZE 1024

// Sensor-related configurations
#define MPU6050_SDA_PIN D2                 // SDA pin for I2C communication
#define MPU6050_SCL_PIN D1                 // SCL pin for I2C communication
#define ledPin D4                         // The pin connected to the LED
#define EEPROM_SIZE 1

// Calibration constants for the accelerometer and gyroscope
#define ACCELEROMETER_CALIBRATION_X         -473
#define ACCELEROMETER_CALIBRATION_Y         0
#define ACCELEROMETER_CALIBRATION_Z         -16390
#define GYROSCOPE_CALIBRATION_X             530
#define GYROSCOPE_CALIBRATION_Y             -260
#define GYROSCOPE_CALIBRATION_Z             17

#endif // CONFIG_H
