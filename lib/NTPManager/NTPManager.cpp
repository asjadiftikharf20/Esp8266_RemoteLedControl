#include "NTPManager.h"

NTPManager::NTPManager() : timeClient(ntpUDP, "pool.ntp.org", 0, 60000) {}

void NTPManager::initialize() {
    timeClient.begin();
}

String NTPManager::getTimestamp() {
    timeClient.update();
    return timeClient.getFormattedTime();
}
