#include "HttpRequester.h"

// Constructor
HttpRequester::HttpRequester(Logger& logger, String url, String token, String tlsCaCertificate)
    : url(url), token(token), tlsCaCertificate(tlsCaCertificate),
      useHttps(url.startsWith("https://")), logger(logger) {
    while (this->url.endsWith("/")) {
        this->url.remove(this->url.length() - 1);
    }
}

String HttpRequester::getBackendHost() const {
    int hostStart = url.indexOf("://");
    hostStart = hostStart >= 0 ? hostStart + 3 : 0;

    int hostEnd = url.indexOf('/', hostStart);
    if (hostEnd < 0) {
        hostEnd = url.length();
    }

    String authority = url.substring(hostStart, hostEnd);
    if (authority.startsWith("[")) {
        int closingBracket = authority.indexOf(']');
        return closingBracket > 0 ? authority.substring(1, closingBracket) : authority;
    }

    int portSeparator = authority.lastIndexOf(':');
    return portSeparator >= 0 ? authority.substring(0, portSeparator) : authority;
}

int HttpRequester::getBackendPort() const {
    int hostStart = url.indexOf("://");
    hostStart = hostStart >= 0 ? hostStart + 3 : 0;

    int authorityEnd = url.indexOf('/', hostStart);
    if (authorityEnd < 0) {
        authorityEnd = url.length();
    }

    String authority = url.substring(hostStart, authorityEnd);
    int portSeparator = authority.lastIndexOf(':');
    if (portSeparator >= 0 && !authority.endsWith("]")) {
        int configuredPort = authority.substring(portSeparator + 1).toInt();
        if (configuredPort > 0) {
            return configuredPort;
        }
    }

    return useHttps ? 443 : 80;
}

String HttpRequester::getBackendIp() const {
    String host = getBackendHost();
    IPAddress backendIp;

    if (host.isEmpty() || WiFi.status() != WL_CONNECTED ||
        WiFi.hostByName(host.c_str(), backendIp) != 1) {
        return "unresolved";
    }

    return backendIp.toString();
}

String HttpRequester::getRequestUrl(const String& endpoint) const {
    if (endpoint.startsWith("/")) {
        return url + endpoint;
    }
    return url + "/" + endpoint;
}

void HttpRequester::logBackendConfiguration() const {
    String protocol = useHttps ? "HTTPS" : "HTTP";
    logger.logMessage(
        "HttpRequester: backend configuration; mode=" + protocol +
        "; host=" + getBackendHost() +
        "; resolved_ip=" + getBackendIp() +
        "; port=" + String(getBackendPort()) +
        "; base_url=" + url +
        "; tls_verification=" + String(useHttps ? "enabled" : "disabled")
    );

    if (!url.startsWith("http://") && !url.startsWith("https://")) {
        logger.logMessage("HttpRequester: ERROR - BACKEND_URL must start with http:// or https://.");
    }
}

void HttpRequester::logRequest(
    const String& method,
    const String& endpoint,
    int payloadBytes
) const {
    String message =
        "HttpRequester: request; method=" + method +
        "; url=" + getRequestUrl(endpoint) +
        "; backend_ip=" + getBackendIp() +
        "; port=" + String(getBackendPort()) +
        "; wifi_ip=" + WiFi.localIP().toString() +
        "; rssi_dbm=" + String(WiFi.RSSI());

    if (payloadBytes > 0) {
        message += "; payload_bytes=" + String(payloadBytes);
    }

    logger.logMessage(message);
}

void HttpRequester::logResponse(
    const String& method,
    const String& endpoint,
    const HttpResult& result
) const {
    String message =
        "HttpRequester: response; method=" + method +
        "; endpoint=" + endpoint +
        "; status=" + String(result.statusCode) +
        "; response_bytes=" + String(result.responseBody.length());

    if (result.statusCode < 0) {
        message += "; transport_error=" + HTTPClient::errorToString(result.statusCode);
    }

    bool failed = result.statusCode < 200 || result.statusCode >= 300;
    if (failed && !result.responseBody.isEmpty()) {
        const int maxBodyLogLength = 2048;
        String responsePreview = result.responseBody.substring(0, maxBodyLogLength);
        message += "; body=" + responsePreview;
        if (result.responseBody.length() > maxBodyLogLength) {
            message += "...[truncated]";
        }
    }

    logger.logMessage(message);
}

template <typename Client>
HttpResult HttpRequester::executePost(Client& client, const String& endpoint, const String& json) {
    HTTPClient http;
    String requestUrl = getRequestUrl(endpoint);
    logRequest("POST", endpoint, json.length());

    if (!http.begin(client, requestUrl)) {
        HttpResult result = {-1, "HTTP client initialization failed"};
        logResponse("POST", endpoint, result);
        return result;
    }
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + token);

    HttpResult result;
    result.statusCode = http.POST(json);
    if (result.statusCode > 0) {
        result.responseBody = http.getString();
    }
    http.end();
    logResponse("POST", endpoint, result);
    return result;
}

template <typename Client>
HttpResult HttpRequester::executeGet(Client& client, const String& endpoint) {
    HTTPClient http;
    String requestUrl = getRequestUrl(endpoint);
    logRequest("GET", endpoint);

    if (!http.begin(client, requestUrl)) {
        HttpResult result = {-1, "HTTP client initialization failed"};
        logResponse("GET", endpoint, result);
        return result;
    }
    HttpResult result;
    result.statusCode = http.GET();
    if (result.statusCode > 0) {
        result.responseBody = http.getString();
    }
    http.end();
    logResponse("GET", endpoint, result);
    return result;
}

HttpResult HttpRequester::performPost(const String& endpoint, const String& json) {
    if (useHttps) {
        if (tlsCaCertificate.isEmpty()) {
            logger.logMessage("HttpRequester: TLS_CA_CERTIFICATE is required for HTTPS.");
            return {-1, "TLS CA certificate is missing"};
        }
        WiFiClientSecure client;
        client.setCACert(tlsCaCertificate.c_str());
        return executePost(client, endpoint, json);
    }

    WiFiClient client;
    return executePost(client, endpoint, json);
}

HttpResult HttpRequester::performGet(const String& endpoint) {
    if (useHttps) {
        if (tlsCaCertificate.isEmpty()) {
            logger.logMessage("HttpRequester: TLS_CA_CERTIFICATE is required for HTTPS.");
            return {-1, "TLS CA certificate is missing"};
        }
        WiFiClientSecure client;
        client.setCACert(tlsCaCertificate.c_str());
        return executeGet(client, endpoint);
    }

    WiFiClient client;
    return executeGet(client, endpoint);
}

bool HttpRequester::verifyBackend() {
    return performGet("/").statusCode == 200;
}

bool HttpRequester::sendMeasurements(const String& json) {
    if (json.isEmpty()) {
        logger.logMessage("HttpRequester: ERROR - Empty measurement.");
        return false;
    }

    HttpResult result = performPost("/api/measurements", json);

    if (result.statusCode == 201) {
        return true;
    } else {
        if (result.statusCode == 422) {
            logger.logMessage(
                "HttpRequester: HTTP 422 means the backend rejected the measurement payload. "
                "Inspect 'body' in the response log for the invalid field and value."
            );
            logger.logMessage("HttpRequester: rejected payload=" + json);
        }
        return false;
    }
}
