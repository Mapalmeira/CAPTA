#include "Meter.h"

void Meter::setResistance(double value) { resistance = value; }
void Meter::setTransformerTurns(double turns) { transformerTurns = turns; }

Measurement Meter::measure() {
    int timestamp = getEpoch();
    if (timestamp <= 0) {
        Serial.println("Meter: time is not synchronized; measurement skipped.");
        return {};
    }

    double currentAmps = measureCurrent();
    if (!isfinite(currentAmps)) {
        Serial.println("Meter: current is not finite; measurement skipped.");
        return {};
    }

    float current = static_cast<float>(currentAmps);
    return {timestamp, current};
}

int Meter::getEpoch() {
    if (!ntpInitialized) {
        initializeNTP();
        ntpInitialized = true;
    }

    const int currentMillis = millis();
    struct tm timeinfo;

    if (getLocalTime(&timeinfo, 1000)) {
        referenceEpoch = mktime(&timeinfo);
        referenceMillis = currentMillis;
        hasTimeReference = true;
        return referenceEpoch;
    }

    if (hasTimeReference) {
        const int elapsedSeconds = (currentMillis - referenceMillis) / 1000;
        return referenceEpoch + elapsedSeconds;
    }

    return -1;
}

void Meter::initializeNTP() {
    configTime(0, 0, "pool.ntp.org");
}

void Meter::beginI2C() {
    Wire.begin();
    Wire.setClock(400000);

    if (!ads.begin()) {
      Serial.println("Meter: failed to initialize ADS1115!");
      return;
    }

    ads.setGain(GAIN_ONE);
    ads.setDataRate(RATE_ADS1115_860SPS);
}

double Meter::calculateMean(double *data, int size) {
    if (size <= 0) return 0;
    double sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i];
    }
    return sum / size;
}

double Meter::calculateStandardDeviation(double *data, int size, double mean) {
    if (size <= 0) return 0;
    double sum = 0;
    for (int i = 0; i < size; i++) {
      sum += pow(data[i] - mean, 2);
    }
    return sqrt(sum / size);
}

int Meter::filterAndCenterData(double *data, int size) {
    if (size <= 0) return 0;

    double mean = calculateMean(data, size);
    double standardDeviation = calculateStandardDeviation(data, size, mean);

    int validCount = 0;
    double filteredSum = 0;
    for (int i = 0; i < size; i++) {
        if (standardDeviation == 0 ||
            fabs(data[i] - mean) < 2 * standardDeviation) {
            filteredSum += data[i];
            data[validCount] = data[i];
            validCount++;
        }
    }

    if (validCount == 0) return 0;

    double filteredMean = filteredSum / validCount;
    for (int i = 0; i < validCount; i++) {
        data[i] -= filteredMean;
    }

    return validCount;
}

double Meter::calculateRMS(double *data, int size) {
    if (size == 0) return 0;
    double sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i] * data[i];
    }
    return sqrt(sum / size);
}

double Meter::adcToVoltage(int reading) {
    return reading * 0.000125;
}

void Meter::collectReadings(double *buffer, int size) {
    for (int i = 0; i < size; i++) {
        int16_t raw;
        raw = ads.readADC_SingleEnded(0);
        buffer[i] = adcToVoltage(raw);
    }
}

double Meter::measureCurrent() {
    double *readings = (double *)malloc(sampleCount * sizeof(double));

    if (!readings) {
        Serial.println("Meter: memory allocation error.");
        return 0;
    }

    collectReadings(readings, sampleCount);
    int validCount = filterAndCenterData(readings, sampleCount);
    double rmsVoltage = calculateRMS(readings, validCount);

    free(readings);

    return (rmsVoltage / resistance) * transformerTurns;
}
