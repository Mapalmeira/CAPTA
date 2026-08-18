#include "HttpsRequester.h"

// Constructor
HttpsRequester::HttpsRequester(Logger& logger, String url, String token): url(url), logger(logger), token(token) {}

String HttpsRequester::performPost(const String& data, int &httpCode) {
    WiFiClientSecure client;
    HTTPClient http;

    client.setInsecure();  // Skip SSL certificate verification.
    http.begin(client, url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    httpCode = http.POST(data);
    String response = http.getString();
    response.trim();
    http.end();

    // Follow a redirect to Googleusercontent when necessary.
    if (response.indexOf("Moved Temporarily") != -1) {
        int startPosition = response.indexOf("https://script.googleusercontent.com");
        int endPosition = response.indexOf("\"", startPosition);

        if (startPosition > 0 && endPosition > startPosition) {
            String redirectUrl = response.substring(startPosition, endPosition);
            redirectUrl.replace("&amp;", "&");  // Decode the HTML entity.

            // Repeat the request as GET at the redirect URL.
            client.setInsecure();
            http.begin(client, redirectUrl);
            httpCode = http.GET();
            response = http.getString();
            response.trim();
            http.end();
        }
    }

    return response;
}


// Verify the connection with a POST request.
bool HttpsRequester::verifyPost() {
    int httpCode;
    String response = performPost("mode=verify&token=" + token, httpCode);

    if (response == "Data saved!") {
        return true;
    } else {
        return false;
    }
}

// Send measurement rows.
bool HttpsRequester::sendMeasurements(const String& data) {
    if (data.isEmpty()) {
        logger.logMessage("HttpsRequester: ERROR - Empty measurement.");
        return false;
    }

    String body = "mode=append-lines&data=" + data + "&token=" + token;
    logger.logMessage("HttpsRequester: sending via POST: " + body);

    int httpCode;
    String response = performPost(body, httpCode);

    response.toLowerCase();
    if (httpCode == 200 && response.indexOf("data saved") != -1) {
        return true;
    } else {
        logger.logMessage("HttpsRequester: ERROR while appending rows. HTTP code: " +
                              String(httpCode) + ". Response: " + response);
        return false;
    }
}
