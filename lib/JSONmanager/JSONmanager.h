#ifndef JSON_MANAGER_H
#define JSON_MANAGER_H

#include "Arduino_JSON.h"

void initializeNTP();
String getJsonData(float ax, float ay, float az, float gx, float gy, float gz);
String getChipId();

#endif
