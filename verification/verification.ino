#include <Wire.h>
#include <Adafruit_ADS1X15.h>

int COLLECTION_TIME_MS = 1000;
int SAMPLES_PER_SECOND = 750;

double resistance = 75.0;             // Burden resistor resistance in ohms.
double transformerTurns = 2000.0;     // Current-transformer ratio (for example, SCT-013-000).

Adafruit_ADS1115 ads;

// Convert a raw ADS1115 reading to voltage.
double adcToVoltage(int16_t reading) {
  return ads.computeVolts(reading);
}

// Calculate the data mean.
double calculateMean(double *data, int size) {
  double sum = 0;
  for (int i = 0; i < size; i++) {
    sum += data[i];
  }
  return sum / size;
}

// Calculate the data standard deviation.
double calculateStandardDeviation(double *data, int size, double mean) {
  double sum = 0;
  for (int i = 0; i < size; i++) {
    sum += pow(data[i] - mean, 2);
  }
  return sqrt(sum / size);
}

// Remove outliers and center the accepted samples around their own mean.
int filterAndCenterData(double *data, int size) {
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

// Calculate the RMS value of a voltage array.
double calculateRMS(double *data, int size) {
  if (size == 0) return 0;

  double sumOfSquares = 0;
  for (int i = 0; i < size; i++) {
    sumOfSquares += data[i] * data[i];
  }
  return sqrt(sumOfSquares / size);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin();
  Wire.setClock(400000);
  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS1115.");
    while (true) delay(1000);
  }

  ads.setGain(GAIN_ONE);
  ads.setDataRate(RATE_ADS1115_860SPS);
}

void loop() {
  int sampleCount = (COLLECTION_TIME_MS * SAMPLES_PER_SECOND) / 1000;
  double *readings = (double *)malloc(sampleCount * sizeof(double));

  if (readings == NULL) {
    Serial.println("Memory allocation error.");
    return;
  }

  // Collect samples.
  unsigned long startedAt = micros();
  for (int i = 0; i < sampleCount; i++) {
    readings[i] = adcToVoltage(ads.readADC_SingleEnded(0));
    unsigned long nextSampleAt = startedAt + ((i + 1) * (1000000 / SAMPLES_PER_SECOND));
    while (micros() < nextSampleAt) {}
  }

  // Filter the data and calculate RMS voltage.
  int validCount = filterAndCenterData(readings, sampleCount);
  double rmsVoltage = calculateRMS(readings, validCount);

  // Release allocated memory.
  free(readings);

  // Convert RMS voltage to RMS current.
  double rmsCurrent = (rmsVoltage / resistance) * transformerTurns;

  // Display the result.
  Serial.println(rmsCurrent, 5);
}
