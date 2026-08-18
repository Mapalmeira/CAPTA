// Header includes
#include "system/System.h"
#include "meter/Meter.h"
#include "system/physical-interface/PhysicalInterface.h"
#include "system/measurement-sender/MeasurementSender.h"
#include "system/measurement-sender/HttpsRequester.h"
#include "system/logger/Logger.h"
#include "system/connection/Connection.h"

// Implementation includes
#include "system/System.cpp"
#include "meter/Meter.cpp"
#include "system/physical-interface/PhysicalInterface.cpp"
#include "system/measurement-sender/MeasurementSender.cpp"
#include "system/measurement-sender/HttpsRequester.cpp"
#include "system/logger/Logger.cpp"
#include "system/connection/Connection.cpp"

// System settings
constexpr const char* SSID = "";
constexpr const char* PASSWORD = "";
constexpr const char* APPS_SCRIPT_URL = "";
constexpr const char* TOKEN = "";
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

System systemController(
    SSID,
    PASSWORD,
    APPS_SCRIPT_URL,
    TOKEN,
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
    systemController.setLogging(ENABLE_LOGS);
    systemController.setPeriodicRestart(PERIODIC_RESTART);
    systemController.setTransmissionsPerBatch(TRANSMISSIONS_PER_BATCH);
    systemController.setAdditionalDelay(ADDITIONAL_DELAY_MS);
    systemController.setProgressiveDelay(PROGRESSIVE_DELAY);

    // Start the system.
    systemController.begin();
}

void loop() {
    // Check the serial input for commands.
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        systemController.handleCommand(command);
    }

    // Run the measurement and transmission system.
    systemController.loop();
    delay(100);
}
