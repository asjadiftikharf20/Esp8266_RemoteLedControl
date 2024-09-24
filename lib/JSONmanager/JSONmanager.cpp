#include "JSONmanager.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Wire.h>

// Initialize NTPClient with a UTC offset (e.g., 0 for UTC)
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000); // UTC 0, Update interval 60 seconds

void initializeNTP() {
    timeClient.begin();
}

String getFormattedTime() {
    timeClient.update();
    unsigned long epochTime = timeClient.getEpochTime();
    int utcOffset = 5; // UTC offset in hours

    // Convert the epoch time to hours, minutes, and seconds
    int hours = (epochTime % 86400L) / 3600;  // 86400 seconds in a day
    hours = hours + utcOffset;
    int minutes = (epochTime % 3600) / 60;
    int seconds = epochTime % 60;

    String formattedTime = String(hours) + ":" + 
                           String(minutes) + ":" + 
                           String(seconds) + " UTC" + 
                           (utcOffset >= 0 ? "+" : "") + String(utcOffset);
    return formattedTime;
}

String getJsonData(float ax, float ay, float az, float gx, float gy, float gz) {
    // Get formatted time
    String timestamp = getFormattedTime();
    
    // Get ESP8266 MAC address
    String macAddress = getChipId(); // MAC address of the device
    
    // Format the output line by line
    String output = "";
    output += "t: " + timestamp + "\n";   // Timestamp line with UTC time
    output += "id: " + macAddress + "\n"; // MAC address line
    
    // Add acceleration values each on a separate line
    output += "ax: " + String(ax, 6) + "\n";  // Acceleration x
    output += "ay: " + String(ay, 4) + "\n";  // Acceleration y
    output += "az: " + String(az, 4) + "\n";  // Acceleration z
    
    // Add rotation values each on a separate line
    output += "gx: " + String(gx, 4) + "\n";  // Rotation x
    output += "gy: " + String(gy, 4) + "\n";  // Rotation y
    output += "gz: " + String(gz, 4) + "\n";  // Rotation z
    
    return output;
}

String getChipId() {
  uint32_t chipId = ESP.getChipId();
  String chipIdStr = String(chipId, HEX);
  return chipIdStr;
}
