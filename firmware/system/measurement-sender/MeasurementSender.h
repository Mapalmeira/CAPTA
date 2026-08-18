#ifndef MEASUREMENT_SENDER_H
#define MEASUREMENT_SENDER_H

#include <deque>
#include <variant>
#include "HttpsRequester.h"
#include "system/logger/Logger.h"

using MeasurementValue = std::variant<int, float>;
using Measurement = std::vector<MeasurementValue>; // Mixed values sent by the meter.
using MeasurementDecimalPlaces = std::vector<int>; // Decimal places for each value; -1 represents integers.

class MeasurementSender {
private:
    std::deque<Measurement> buffer; // Pending measurements.
    std::function<Measurement(void)> measurementFunction;
    HttpsRequester requester;
    MeasurementDecimalPlaces decimalPlaces;
    Logger& logger;

    String formatMeasurement(const Measurement& measurement);
    String buildRowBatch(int rows);

public:
    MeasurementSender(String appsScriptUrl, Logger& logger, String token,
              std::function<Measurement(void)> measurementFunction,
              std::vector<int> decimalPlaces);

    // Measurement
    bool takeMeasurement();

    // Transmission
    String sendRowsByPost(int rows);
    bool checkPostStatus();
    int getBufferSize();
};

#endif // MEASUREMENT_SENDER_H
