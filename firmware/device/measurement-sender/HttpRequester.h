#ifndef HTTP_REQUESTER_H
#define HTTP_REQUESTER_H

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "device/logger/Logger.h"

class HttpRequester {
private:
    String url;
    String token;
    String tlsCaCertificate;
    bool useHttps;
    Logger& logger;

    int performPost(const String& endpoint, const String& json);
    int performGet(const String& endpoint);

    template <typename Client>
    int executePost(Client& client, const String& endpoint, const String& json);

    template <typename Client>
    int executeGet(Client& client, const String& endpoint);

public:
    HttpRequester(Logger& logger, String url, String token, String tlsCaCertificate);
    bool sendMeasurements(const String& json);
    bool verifyBackend();
};

#endif
