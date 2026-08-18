#include "MeasurementSender.h"

// Constructor
MeasurementSender::MeasurementSender(String appsScriptUrl, Logger& logger, String token,
                             std::function<Measurement(void)> measurementFunction,
                             std::vector<int> decimalPlaces)
    : logger(logger), requester(logger, appsScriptUrl, token), measurementFunction(measurementFunction), decimalPlaces(decimalPlaces)
{}

// Format one measurement using the wire protocol.
String MeasurementSender::formatMeasurement(const Measurement& measurement) {
    if (measurement.size() > decimalPlaces.size()) {
        logger.logMessage("MeasurementSender: the number of decimal-place settings does not match the measurement values.");
        return "";
    }

    String row = "";
    for (size_t i = 0; i < measurement.size(); i++) {
        if (i > 0) { row += "|"; }

        if (decimalPlaces[i] == -1) {
            // Treat -1 as an integer.
            row += String(std::get<int>(measurement[i]));
        } else {
            // Treat any other setting as a floating-point precision.
            row += String(std::get<float>(measurement[i]), decimalPlaces[i]);
        }
    }
    return row;
}

// Build a batch with multiple measurements.
String MeasurementSender::buildRowBatch(int rows) {
    String batch = "";
    int rowsAdded = 0;

    while (rowsAdded < rows && !buffer.empty()) {
        String formattedMeasurement = formatMeasurement(buffer.at(rowsAdded));

        if (rowsAdded > 0) { batch += "_"; }

        batch += formattedMeasurement;
        rowsAdded++;
    }

    if (rowsAdded == 0) {
        logger.logMessage("MeasurementSender: no valid measurements to send.");
        return "";
    }

    return batch;
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

// Send rows through a POST request.
String MeasurementSender::sendRowsByPost(int rows) {
    if (buffer.empty()) {
        logger.logMessage("MeasurementSender: no measurements are available to send.");
        return "empty buffer";
    }

    if (rows > buffer.size()) {
        rows = buffer.size();
    }

    String batch = buildRowBatch(rows);

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
        logger.logMessage("MeasurementSender: failed to send measurements to Drive.");
        return "connection error";
    }
}

bool MeasurementSender::checkPostStatus() {
    return requester.verifyPost();
}

int MeasurementSender::getBufferSize() {
    return buffer.size();
}
