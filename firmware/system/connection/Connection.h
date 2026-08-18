#ifndef CONNECTION_H
#define CONNECTION_H

#include <WiFi.h>
#include "system/logger/Logger.h"

class Connection {
private:
    String networkName;
    String networkPassword;

public:
    Connection(String ssid, String password);
    bool connectWiFi();
    bool isConnected();
    String getStatusMessage();
};

#endif
