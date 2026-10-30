#ifndef METER_H
#define METER_H

#include <Wire.h>
#include <Adafruit_ADS1X15.h>
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
        int filterAndCenterData(double *data, int size);
        double calculateRMS(double *data, int size);

        double measureCurrent();

    public:
        void beginI2C();
        void setResistance(double value);
        void setTransformerTurns(double turns);

        Measurement measure(); // Returns [timestamp, current].
};

#endif // METER_H
