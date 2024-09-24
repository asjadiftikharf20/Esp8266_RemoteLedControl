#ifndef NTPMANAGER_H
#define NTPMANAGER_H

#include <NTPClient.h>
#include <WiFiUdp.h>

class NTPManager {
  public:
    NTPManager();
    void initialize();
    String getTimestamp();

  private:
    WiFiUDP ntpUDP;
    NTPClient timeClient;
};

#endif
