#include "HttpRequester.h"

// Constructor
HttpRequester::HttpRequester(Logger& logger, String url, String token, String tlsCaCertificate)
    : url(url), token(token), tlsCaCertificate(tlsCaCertificate),
      useHttps(url.startsWith("https://")), logger(logger) {}

template <typename Client>
int HttpRequester::executePost(Client& client, const String& endpoint, const String& json) {
    HTTPClient http;

    http.begin(client, url + endpoint);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + token);

    int httpCode = http.POST(json);
    http.end();
    return httpCode;
}

template <typename Client>
int HttpRequester::executeGet(Client& client, const String& endpoint) {
    HTTPClient http;

    http.begin(client, url + endpoint);
    int httpCode = http.GET();
    http.end();
    return httpCode;
}

int HttpRequester::performPost(const String& endpoint, const String& json) {
    if (useHttps) {
        if (tlsCaCertificate.isEmpty()) {
            logger.logMessage("HttpRequester: TLS_CA_CERTIFICATE is required for HTTPS.");
            return -1;
        }
        WiFiClientSecure client;
        client.setCACert(tlsCaCertificate.c_str());
        return executePost(client, endpoint, json);
    }

    WiFiClient client;
    return executePost(client, endpoint, json);
}

int HttpRequester::performGet(const String& endpoint) {
    if (useHttps) {
        if (tlsCaCertificate.isEmpty()) {
            logger.logMessage("HttpRequester: TLS_CA_CERTIFICATE is required for HTTPS.");
            return -1;
        }
        WiFiClientSecure client;
        client.setCACert(tlsCaCertificate.c_str());
        return executeGet(client, endpoint);
    }

    WiFiClient client;
    return executeGet(client, endpoint);
}

bool HttpRequester::verifyBackend() {
    return performGet("/") == 200;
}

bool HttpRequester::sendMeasurements(const String& json) {
    if (json.isEmpty()) {
        logger.logMessage("HttpRequester: ERROR - Empty measurement.");
        return false;
    }

    int httpCode = performPost("/api/measurements", json);

    if (httpCode == 201) {
        return true;
    } else {
        logger.logMessage("HttpRequester: backend returned HTTP code " + String(httpCode));
        return false;
    }
}
