#include <Arduino.h>
#include "taskmanager.h"

void setup() {
    Serial.begin(9600);
    run_tasks_OneTime();  // Call the task manager to initialize the sensor
}

void loop() {
    run_tasks_continuous();  // Call the task manager to handle sensor data
}