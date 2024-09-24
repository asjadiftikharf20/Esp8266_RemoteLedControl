#include "taskmanager.h"


void run_tasks_OneTime() {
    initialize_sensors();               // Call the function to initialize the sensor
    pinMode(ledPin, OUTPUT);
    connectToWiFi();
    establishConnect();
    task_sensor_lookout();
    initializeNTP();                    // Call the function to initialize the NTP client
    
}

void run_tasks_continuous() {
    MQTT_loop();

}
