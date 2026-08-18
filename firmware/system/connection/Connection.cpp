#include "Connection.h"

Connection::Connection(String ssid, String password)
    : networkName(ssid), networkPassword(password) {}

bool Connection::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

bool Connection::connectWiFi() {
    WiFi.disconnect();
    WiFi.begin(networkName.c_str(), networkPassword.c_str());

    unsigned long startedAt = millis();
    const unsigned long timeout = 10000;

    while (!isConnected()) {
        delay(500);
        Serial.print(".");
        if (millis() - startedAt > timeout) {
            return false;
        }
    }
    return true;
}

String Connection::getStatusMessage() {
  if (isConnected()) {
    return "\nConnection: Wi-Fi connected! IP address: " + String(WiFi.localIP().toString());
  } else {
    return "Connection: Wi-Fi disconnected";
  }
}
