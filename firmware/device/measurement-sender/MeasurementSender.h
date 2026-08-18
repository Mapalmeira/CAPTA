#ifndef MEASUREMENT_SENDER_H
#define MEASUREMENT_SENDER_H

#include <deque>
#include <variant>
#include "HttpRequester.h"
#include "device/logger/Logger.h"

using MeasurementValue = std::variant<int, float>;
using Measurement = std::vector<MeasurementValue>; // Mixed values sent by the meter.
using MeasurementDecimalPlaces = std::vector<int>; // Decimal places for each value; -1 represents integers.

class MeasurementSender {
private:
    std::deque<Measurement> buffer; // Pending measurements.
    std::function<Measurement(void)> measurementFunction;
    HttpRequester requester;
    MeasurementDecimalPlaces decimalPlaces;
    Logger& logger;

    String formatMeasurementAsJson(const Measurement& measurement);
    String buildBatchJson(int rows);

public:
    MeasurementSender(String backendUrl, Logger& logger, String token, String tlsCaCertificate,
              std::function<Measurement(void)> measurementFunction,
              std::vector<int> decimalPlaces);

    // Measurement
    bool takeMeasurement();

    // Transmission
    String sendBatch(int rows);
    bool checkBackendStatus();
    int getBufferSize();
};

#endif // MEASUREMENT_SENDER_H
