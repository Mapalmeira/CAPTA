// Header includes
#include "secrets.h"
#include "device/DeviceController.h"
#include "meter/Meter.h"
#include "device/physical-interface/PhysicalInterface.h"
#include "device/measurement-sender/MeasurementSender.h"
#include "device/measurement-sender/HttpRequester.h"
#include "device/logger/Logger.h"
#include "device/connection/Connection.h"

// Implementation includes
#include "device/DeviceController.cpp"
#include "meter/Meter.cpp"
#include "device/physical-interface/PhysicalInterface.cpp"
#include "device/measurement-sender/MeasurementSender.cpp"
#include "device/measurement-sender/HttpRequester.cpp"
#include "device/logger/Logger.cpp"
#include "device/connection/Connection.cpp"

// Device settings
constexpr const char* BACKEND_URL = "";
constexpr const char* TLS_CA_CERTIFICATE = ""; // Required only when BACKEND_URL uses https://.
constexpr bool ENABLE_LOGS = true;  // Reserved for a future implementation.
constexpr bool PERIODIC_RESTART = true;
constexpr int TRANSMISSIONS_PER_BATCH = 10;
constexpr int ADDITIONAL_DELAY_MS = 7500;
constexpr bool PROGRESSIVE_DELAY = false;
const std::vector<int> DECIMAL_PLACES = {-1, 2};

// Meter settings
constexpr double BURDEN_RESISTANCE = 75;
constexpr double TRANSFORMER_TURNS = 2000.0;
constexpr double VOLTAGE_OFFSET = 1.65;
constexpr double SCALE_COEFFICIENT = 1;
constexpr double INTERCEPT_COEFFICIENT = -0;

Meter meter;

auto measurementFunction = []() {
    return meter.measure();
};

DeviceController deviceController(
    WIFI_SSID,
    WIFI_PASSWORD,
    BACKEND_URL,
    AUTH_TOKEN,
    TLS_CA_CERTIFICATE,
    measurementFunction,
    DECIMAL_PLACES // Decimal places for the timestamp and current. -1 means integer.
);

void setup() {
    Serial.begin(115200);
    delay(4000);

    // Meter settings
    meter.beginI2C();
    meter.setResistance(BURDEN_RESISTANCE);
    meter.setTransformerTurns(TRANSFORMER_TURNS);
    meter.setVoltageOffset(VOLTAGE_OFFSET);
    meter.setCorrectionCoefficients(SCALE_COEFFICIENT, INTERCEPT_COEFFICIENT);

    // Optional system settings
    deviceController.setLogging(ENABLE_LOGS);
    deviceController.setPeriodicRestart(PERIODIC_RESTART);
    deviceController.setTransmissionsPerBatch(TRANSMISSIONS_PER_BATCH);
    deviceController.setAdditionalDelay(ADDITIONAL_DELAY_MS);
    deviceController.setProgressiveDelay(PROGRESSIVE_DELAY);

    // Start the system.
    deviceController.begin();
}

void loop() {
    // Check the serial input for commands.
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        deviceController.handleCommand(command);
    }

    // Run the measurement and transmission system.
    deviceController.loop();
    delay(100);
}
