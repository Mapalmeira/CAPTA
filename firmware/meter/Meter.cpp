#include "Meter.h"

Meter::Meter() {}

void Meter::setResistance(double value) { resistance = value; }
void Meter::setTransformerTurns(double turns) { transformerTurns = turns; }
void Meter::setVoltageOffset(double offset) { voltageOffset = offset; }
void Meter::setCorrectionCoefficients(double scale, double intercept) {
    scaleCoefficient = scale;
    interceptCoefficient = intercept;
}

Measurement Meter::measure() {
    int timestamp = getEpoch();
    if (timestamp <= 0) {
        Serial.println("Meter: time is not synchronized; measurement skipped.");
        return {};
    }

    double correctedCurrent = measureCorrectedCurrent();
    if (!isfinite(correctedCurrent)) {
        Serial.println("Meter: corrected current is not finite; measurement skipped.");
        return {};
    }

    // RMS current cannot be negative. A negative calibration intercept can
    // otherwise produce invalid values close to zero and trigger HTTP 422.
    if (correctedCurrent < 0) {
        Serial.println(
            "Meter: corrected current " + String(correctedCurrent, 4) +
            " A clamped to 0 A."
        );
        correctedCurrent = 0;
    }

    float current = static_cast<float>(correctedCurrent);
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

int Meter::filterData(double *data, int size, double offset, double *filteredData) {
    double mean = calculateMean(data, size);
    double standardDeviation = calculateStandardDeviation(data, size, mean);

    int validCount = 0;
    for (int i = 0; i < size; i++) {
      if (abs(data[i] - mean) < 2 * standardDeviation) {
        double voltage = data[i] - offset;
        filteredData[validCount] = voltage;
        validCount++;
      }
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

double Meter::measureRawCurrent() {
    double *readings = (double *)malloc(sampleCount * sizeof(double));
    double *filteredData = (double *)malloc(sampleCount * sizeof(double));

    if (!readings || !filteredData) {
        Serial.println("Meter: memory allocation error.");
        if (readings) free(readings);
        if (filteredData) free(filteredData);
        return 0;
    }

    collectReadings(readings, sampleCount);
    int validCount = filterData(readings, sampleCount, voltageOffset, filteredData);
    double rmsVoltage = calculateRMS(filteredData, validCount);

    free(readings);
    free(filteredData);

    return (rmsVoltage / resistance) * transformerTurns;
}

double Meter::measureCorrectedCurrent() {
    double rmsCurrent = measureRawCurrent();
    return rmsCurrent * scaleCoefficient + interceptCoefficient;
}
