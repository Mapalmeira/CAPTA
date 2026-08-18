#include <Wire.h>
#include <Adafruit_ADS1X15.h>

int COLLECTION_TIME_MS = 1000;
int SAMPLES_PER_SECOND = 750;

double resistance = 75.0;             // Burden resistor resistance in ohms.
double transformerTurns = 2000.0;     // Current-transformer ratio (for example, SCT-013-000).
double voltageOffset = 1.535722;       // Voltage offset measured with offset.ino.
double scaleCoefficient = 1.0;        // Scale coefficient (1.0 by default).
double interceptCoefficient = 0.0;    // Offset correction (0.0 by default).

Adafruit_ADS1115 ads;

// Convert a raw ADS1115 reading to voltage.
double adcToVoltage(int16_t reading) {
  return ads.computeVolts(reading);
}

// Calculate the data mean.
double calculateMean(int16_t *data, int size) {
  double sum = 0;
  for (int i = 0; i < size; i++) {
    sum += data[i];
  }
  return sum / size;
}

// Calculate the data standard deviation.
double calculateStandardDeviation(int16_t *data, int size, double mean) {
  double sum = 0;
  for (int i = 0; i < size; i++) {
    sum += pow(data[i] - mean, 2);
  }
  return sqrt(sum / size);
}

// Remove outliers and subtract the voltage offset.
int filterData(int16_t *data, int size, double offset, double *filteredData) {
  double mean = calculateMean(data, size);
  double standardDeviation = calculateStandardDeviation(data, size, mean);

  int validCount = 0;
  for (int i = 0; i < size; i++) {
    if (abs(data[i] - mean) < 2 * standardDeviation) {
      double voltage = adcToVoltage(data[i]) - offset;
      filteredData[validCount] = voltage;
      validCount++;
    }
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
  int16_t *readings = (int16_t *)malloc(sampleCount * sizeof(int16_t));
  double *filteredData = (double *)malloc(sampleCount * sizeof(double));

  if (readings == NULL || filteredData == NULL) {
    Serial.println("Memory allocation error.");
    return;
  }

  // Collect samples.
  unsigned long startedAt = micros();
  for (int i = 0; i < sampleCount; i++) {
    readings[i] = ads.readADC_SingleEnded(0);
    unsigned long nextSampleAt = startedAt + ((i + 1) * (1000000 / SAMPLES_PER_SECOND));
    while (micros() < nextSampleAt) {}
  }

  // Filter the data and calculate RMS voltage.
  int validCount = filterData(readings, sampleCount, voltageOffset, filteredData);
  double rmsVoltage = calculateRMS(filteredData, validCount);

  // Release allocated memory.
  free(readings);
  free(filteredData);

  // Convert RMS voltage to RMS current.
  double rmsCurrent = (rmsVoltage / resistance) * transformerTurns;

  // Apply the linear correction.
  rmsCurrent = (rmsCurrent - interceptCoefficient) * scaleCoefficient;

  // Display the result.
  Serial.println(rmsCurrent, 5);
}
