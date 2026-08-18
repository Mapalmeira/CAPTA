int ANALOG_PIN = 36;
int SAMPLE_COUNT = 10000;
int SAMPLES_PER_SECOND = 750;

// Convert an ADC reading to voltage (0 to 3.3 V).
double adcToVoltage(int reading) {
  return (reading / 4095.0) * 3.3;
}

// Calculate the data mean.
double calculateMean(int *data, int size) {
  double sum = 0;
  for (int i = 0; i < size; i++) {
    sum += data[i];
  }
  return sum / size;
}

// Calculate the data standard deviation.
double calculateStandardDeviation(int *data, int size, double mean) {
  double sum = 0;
  for (int i = 0; i < size; i++) {
    sum += pow(data[i] - mean, 2);
  }
  return sqrt(sum / size);
}

// Calculate a filtered mean after removing outliers.
double calculateVoltageOffset(int *data, int size) {
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

  int *readings = (int *)malloc(SAMPLE_COUNT * sizeof(int));
  if (readings == NULL) {
    Serial.println("Memory allocation error.");
    return;
  }

  for (int i = 0; i < SAMPLE_COUNT; i++) {
    readings[i] = analogRead(ANALOG_PIN);
    delayMicroseconds(1000000 / SAMPLES_PER_SECOND);
  }

  double offset = calculateVoltageOffset(readings, SAMPLE_COUNT);
  Serial.print("Voltage offset (V): ");
  Serial.println(offset, 6);

  free(readings);
}

void loop() {
}
