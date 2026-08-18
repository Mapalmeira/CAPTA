#include <Wire.h>
#include <Adafruit_ADS1X15.h>

int SAMPLE_COUNT = 10000;
int SAMPLES_PER_SECOND = 750;

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

// Calculate a filtered mean after removing outliers.
double calculateVoltageOffset(int16_t *data, int size) {
  double mean = calculateMean(data, size);
  double standardDeviation = calculateStandardDeviation(data, size, mean);

  double sum = 0;
  int validCount = 0;
  for (int i = 0; i < size; i++) {
    if (abs(data[i] - mean) < 2 * standardDeviation) {
      sum += adcToVoltage(data[i]);
      validCount++;
    }
  }

  if (validCount == 0) return 0;
  return sum / validCount;
}

void setup() {
  Serial.begin(115200);
  delay(1000); // Wait for Serial initialization.

  Wire.begin();
  Wire.setClock(400000);
  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS1115.");
    return;
  }

  ads.setGain(GAIN_ONE);
  ads.setDataRate(RATE_ADS1115_860SPS);

  int16_t *readings = (int16_t *)malloc(SAMPLE_COUNT * sizeof(int16_t));
  if (readings == NULL) {
    Serial.println("Memory allocation error.");
    return;
  }

  unsigned long startedAt = micros();
  for (int i = 0; i < SAMPLE_COUNT; i++) {
    readings[i] = ads.readADC_SingleEnded(0);
    unsigned long nextSampleAt = startedAt + ((i + 1) * (1000000 / SAMPLES_PER_SECOND));
    while (micros() < nextSampleAt) {}
  }

  double offset = calculateVoltageOffset(readings, SAMPLE_COUNT);
  Serial.print("Voltage offset (V): ");
  Serial.println(offset, 6);

  free(readings);
}

void loop() {
}
