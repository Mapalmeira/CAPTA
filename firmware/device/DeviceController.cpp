#include "DeviceController.h"

// Constructor
DeviceController::DeviceController(String ssid, String password, String url, String token, String tlsCaCertificate,
                 std::function<Measurement(void)> measurementFunction,
                 std::vector<int> decimalPlaces)
    : SSID(ssid),
      password(password),
      backendUrl(url),
      connection(ssid, password),
      measurementSender(url, logger, token, tlsCaCertificate, measurementFunction, decimalPlaces),
      physicalInterface(),
      operatingMode("sending")
{}


void DeviceController::begin() {
    Serial.println("DeviceController: starting...");

    // Start the logger.
    logger.begin(this->saveLogs);

    // Start Wi-Fi.
    connectWiFi();
    logger.logMessage(connection.getStatusMessage());

    // Start the physical interface.
    auto getModeFunction = [this]() { return this->getMode(); };
    auto commandFunction = [this](String command) { this->handleCommand(command); };
    physicalInterface.begin(getModeFunction, commandFunction);

    this->startTimeMs = millis();
}


// Connect to Wi-Fi.
bool DeviceController::connectWiFi() {
    bool connected = connection.connectWiFi();
    if (connected) {
        connectionFailures = 0;
    } else {
        connectionFailures += 1;
    }
    return connected;
}

bool DeviceController::takeMeasurement() {
    bool measured = measurementSender.takeMeasurement();
    if (!measured) {
        measurementFailures += 1;
    }

    delay(additionalDelayMs);
    return measured;
}

// Transmission
String DeviceController::sendMeasurements(int rows) {
    String status = measurementSender.sendBatch(rows);
    if (status == "measurement error") {
        measurementFailures += 1;
    } else if (status == "connection error") {
        connectionFailures += 1;
    } else if (status == "sent") {
        connectionFailures = 0;
    }

    return status;
}

// Customizations
void DeviceController::setAdditionalDelay(int delayMs) {
    this->additionalDelayMs = delayMs;
}

void DeviceController::setProgressiveDelay(bool enabled) {
    this->progressiveDelay = enabled;
}

void DeviceController::setLogging(bool enabled) {
    this->saveLogs = enabled;
}

void DeviceController::setPeriodicRestart(bool enabled) {
    this->periodicRestart = enabled;
}

void DeviceController::setTransmissionsPerBatch(int transmissions) {
    this->transmissionsPerBatch = transmissions;
}

// Modes
void DeviceController::setMode(String mode) {
    if (mode == "sending" || mode == "pending" || mode == "disconnected" || mode == "shutdown" || mode == "restart") {
        this->operatingMode = mode;
        return;
    }
    logger.logMessage("DeviceController: invalid mode!");
}

void DeviceController::loop() {
    // Switch between operating modes.
    if (connectionFailures >= 3 && !connection.isConnected()) {operatingMode = "disconnected";}
    if (operatingMode == "disconnected" && connection.isConnected()) {operatingMode = "sending";}
    if (operatingMode == "sending" && measurementSender.getBufferSize() >= MAX_TRANSMISSIONS_PER_BATCH) {operatingMode = "pending";}
    if (operatingMode == "pending" && measurementSender.getBufferSize() == 0) {operatingMode = "sending";}

    // Restart when recovery is no longer safe.
    if (measurementFailures >= 3) {operatingMode = "restart";} // Measurements should never fail.
    if (connectionFailures >= 3 && connection.isConnected() && !measurementSender.checkBackendStatus()) {operatingMode = "restart";}
    if (periodicRestart && (millis() - startTimeMs >= 1 * 60 * 60 * 1000)) {operatingMode = "restart";}

    // Print useful status information.
    logger.logMessage("\nDeviceController: mode '" + operatingMode + "'");
    Serial.println("DeviceController: packets remaining in buffer: " + String(measurementSender.getBufferSize()));

    // Execute the current mode.
    if (operatingMode == "sending") {runSendingMode();}
    else if (operatingMode == "pending") {runPendingMode();}
    else if (operatingMode == "disconnected") {runDisconnectedMode();}
    else if (operatingMode == "shutdown") {runShutdownMode();}
    else if (operatingMode == "restart") {runRestartMode();}
    else {
        logger.logMessage("DeviceController: invalid operating mode. Reverting to sending mode.");
        operatingMode = "sending";
    }
}

void DeviceController::runSendingMode() {
    takeMeasurement();

    if (measurementSender.getBufferSize() >= transmissionsPerBatch) {
        sendMeasurements(MAX_TRANSMISSIONS_PER_BATCH);
    }
}
void DeviceController::runPendingMode() {
    sendMeasurements(MAX_TRANSMISSIONS_PER_BATCH);
}

void DeviceController::runDisconnectedMode() {
    takeMeasurement();
    if (connectWiFi()) {
        connectionFailures = 0;
    }

    if (progressiveDelay) {
        if (connectionFailures < 50) {
          delay(1000);
        } else if (connectionFailures < 100) {
            delay(5000);
        } else if (connectionFailures < 150) {
            delay(25000);
        } else if (connectionFailures < 200) {
            delay(120000);
        } else if (connectionFailures < 250) {
            delay(300000);
        } else {
            delay(600000);
        }
    }
}

void DeviceController::runShutdownMode() {
    sendMeasurements(MAX_TRANSMISSIONS_PER_BATCH);
    logger.logMessage("DeviceController: shutting down ESP32...");
    delay(1000);
    esp_deep_sleep_start();
}

void DeviceController::runRestartMode() {
    sendMeasurements(MAX_TRANSMISSIONS_PER_BATCH);
    logger.logMessage("DeviceController: restarting ESP32...");
    delay(1000);
    ESP.restart();
}

String DeviceController::getMode() {
    return operatingMode;
}

// User interaction
void DeviceController::handleCommand(String command) {
    command.trim();

    if (command.equalsIgnoreCase("stop")) {
        operatingMode = "shutdown";
    } else {
        logger.logMessage("DeviceController: unknown command.");
    }
}
