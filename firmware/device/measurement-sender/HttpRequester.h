#ifndef HTTP_REQUESTER_H
#define HTTP_REQUESTER_H

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "device/logger/Logger.h"

struct HttpResult {
    int statusCode;
    String responseBody;
};

class HttpRequester {
private:
    String url;
    String token;
    String tlsCaCertificate;
    bool useHttps;
    Logger& logger;

    HttpResult performPost(const String& endpoint, const String& json);
    HttpResult performGet(const String& endpoint);
    String getBackendHost() const;
    int getBackendPort() const;
    String getBackendIp() const;
    String getRequestUrl(const String& endpoint) const;
    void logRequest(const String& method, const String& endpoint, int payloadBytes = 0) const;
    void logResponse(const String& method, const String& endpoint, const HttpResult& result) const;

    template <typename Client>
    HttpResult executePost(Client& client, const String& endpoint, const String& json);

    template <typename Client>
    HttpResult executeGet(Client& client, const String& endpoint);

public:
    HttpRequester(Logger& logger, String url, String token, String tlsCaCertificate);
    bool sendMeasurements(const String& json);
    bool verifyBackend();
    void logBackendConfiguration() const;
};

#endif
