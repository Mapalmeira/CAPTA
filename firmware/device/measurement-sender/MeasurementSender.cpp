#include "MeasurementSender.h"

// Constructor
MeasurementSender::MeasurementSender(String backendUrl, Logger& logger, String token, String tlsCaCertificate,
                             std::function<Measurement(void)> measurementFunction,
                             std::vector<int> decimalPlaces)
    : logger(logger), requester(logger, backendUrl, token, tlsCaCertificate), measurementFunction(measurementFunction), decimalPlaces(decimalPlaces)
{}

// Serialize one measurement as a JSON object.
String MeasurementSender::formatMeasurementAsJson(const Measurement& measurement) {
    if (measurement.size() != 2 || decimalPlaces.size() != 2) {
        logger.logMessage("MeasurementSender: expected a timestamp and a current value.");
        return "";
    }

    return "{\"timestamp\":" + String(std::get<int>(measurement[0])) +
           ",\"current_amps\":" +
           String(std::get<float>(measurement[1]), decimalPlaces[1]) + "}";
}

// Build the structured JSON request body.
String MeasurementSender::buildBatchJson(int rows) {
    String batch = "{\"measurements\":[";
    int rowsAdded = 0;

    while (rowsAdded < rows && !buffer.empty()) {
        String formattedMeasurement = formatMeasurementAsJson(buffer.at(rowsAdded));
        if (formattedMeasurement.isEmpty()) {
            return "";
        }

        if (rowsAdded > 0) { batch += ","; }

        batch += formattedMeasurement;
        rowsAdded++;
    }

    if (rowsAdded == 0) {
        logger.logMessage("MeasurementSender: no valid measurements to send.");
        return "";
    }

    return batch + "]}";
}

// Take a measurement and append it to the buffer.
bool MeasurementSender::takeMeasurement() {
    if (measurementFunction == nullptr) {
        logger.logMessage("MeasurementSender: no measurement function was configured!");
        return false;
    }

    Serial.println("MeasurementSender: taking a measurement...");
    Measurement data = measurementFunction();

    if (data.empty()) {
        logger.logMessage("MeasurementSender: the measurement function returned no data!");
        return false;
    }

    buffer.push_back(data);
    return true;
}

// Send buffered measurements through a POST request.
String MeasurementSender::sendBatch(int rows) {
    if (buffer.empty()) {
        logger.logMessage("MeasurementSender: no measurements are available to send.");
        return "empty buffer";
    }

    if (rows > buffer.size()) {
        rows = buffer.size();
    }

    String batch = buildBatchJson(rows);

    if (batch.isEmpty()) {
        logger.logMessage("MeasurementSender: empty transmission batch. Cancelling transmission.");
        return "measurement error";
    }

    bool sent = requester.sendMeasurements(batch);

    if (sent) {
        Serial.println("MeasurementSender: batch sent.");
        buffer.erase(buffer.begin(), buffer.begin() + rows);
        return "sent";
    } else {
        logger.logMessage("MeasurementSender: failed to send measurements to the backend.");
        return "connection error";
    }
}

bool MeasurementSender::checkBackendStatus() {
    return requester.verifyBackend();
}

int MeasurementSender::getBufferSize() {
    return buffer.size();
}
