#ifndef METER_H
#define METER_H

#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <WiFi.h>
#include <time.h>
#include <vector>
#include <variant>

using MeasurementValue = std::variant<int, float>;
using Measurement = std::vector<MeasurementValue>;

class Meter {
    private:
        Adafruit_ADS1115 ads;

        int sampleCount = 750;

        double resistance;
        double transformerTurns;
        double voltageOffset;
        double scaleCoefficient;
        double interceptCoefficient;

        bool ntpInitialized = false;
        bool hasTimeReference = false;
        time_t referenceEpoch = 0;
        uint32_t referenceMillis = 0;
        void initializeNTP();
        int getEpoch();

        void collectReadings(double *buffer, int size);
        double adcToVoltage(int reading);
        double calculateMean(double *data, int size);
        double calculateStandardDeviation(double *data, int size, double mean);
        int filterData(double *data, int size, double offset, double *filteredData);
        double calculateRMS(double *data, int size);

        double measureRawCurrent();
        double measureCorrectedCurrent();

    public:
        Meter();

        void beginI2C();
        void setResistance(double value);
        void setTransformerTurns(double turns);
        void setVoltageOffset(double offset);
        void setCorrectionCoefficients(double scale, double intercept);

        Measurement measure(); // Returns [timestamp, corrected current].
};

#endif // METER_H
