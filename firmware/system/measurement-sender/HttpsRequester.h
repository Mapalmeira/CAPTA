#ifndef HTTPS_REQUESTER_H
#define HTTPS_REQUESTER_H

#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "system/logger/Logger.h"

class HttpsRequester {
private:
    String url;
    String token;
    Logger& logger;

    String performPost(const String& data, int &httpCode);

public:
    HttpsRequester(Logger& logger, String url, String token);
    bool sendMeasurements(const String& data);
    bool verifyPost();
};

#endif
