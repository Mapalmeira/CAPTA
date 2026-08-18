#include "Connection.h"

Connection::Connection(String ssid, String password)
    : networkName(ssid), networkPassword(password) {}

bool Connection::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

bool Connection::connectWiFi() {
    Serial.println("Connection: connecting to Wi-Fi SSID='" + networkName + "'...");
    WiFi.disconnect();
    WiFi.begin(networkName.c_str(), networkPassword.c_str());

    unsigned long startedAt = millis();
    const unsigned long timeout = 10000;

    while (!isConnected()) {
        delay(500);
        Serial.print(".");
        if (millis() - startedAt > timeout) {
            Serial.println();
            Serial.println(
                "Connection: Wi-Fi connection timed out after " +
                String(timeout) + " ms; status=" + String(WiFi.status())
            );
            return false;
        }
    }
    Serial.println();
    return true;
}

String Connection::getStatusMessage() {
  if (isConnected()) {
    return "Connection: Wi-Fi connected; ssid='" + WiFi.SSID() +
           "'; local_ip=" + WiFi.localIP().toString() +
           "; gateway=" + WiFi.gatewayIP().toString() +
           "; subnet=" + WiFi.subnetMask().toString() +
           "; dns=" + WiFi.dnsIP().toString() +
           "; rssi_dbm=" + String(WiFi.RSSI());
  } else {
    return "Connection: Wi-Fi disconnected; status=" + String(WiFi.status());
  }
}
