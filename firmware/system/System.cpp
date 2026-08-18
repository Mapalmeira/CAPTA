#include "System.h"

// Constructor
System::System(String ssid, String password, String url, String token,
                 std::function<Measurement(void)> measurementFunction,
                 std::vector<int> decimalPlaces)
    : SSID(ssid),
      password(password),
      appsScriptUrl(url),
      connection(ssid, password),
      measurementSender(url, logger, token, measurementFunction, decimalPlaces),
      physicalInterface(),
      operatingMode("sending")
{}


void System::begin() {
    Serial.println("System: starting...");

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
bool System::connectWiFi() {
    bool connected = connection.connectWiFi();
    if (connected) {
        connectionFailures = 0;
    } else {
        connectionFailures += 1;
    }
    return connected;
}

bool System::takeMeasurement() {
    bool measured = measurementSender.takeMeasurement();
    if (!measured) {
        measurementFailures += 1;
    }

    delay(additionalDelayMs);
    return measured;
}

// Transmission
String System::sendSpreadsheetRows(int rows) {
    String status = measurementSender.sendRowsByPost(rows);
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
void System::setAdditionalDelay(int delayMs) {
    this->additionalDelayMs = delayMs;
}

void System::setProgressiveDelay(bool enabled) {
    this->progressiveDelay = enabled;
}

void System::setLogging(bool enabled) {
    this->saveLogs = enabled;
}

void System::setPeriodicRestart(bool enabled) {
    this->periodicRestart = enabled;
}

void System::setTransmissionsPerBatch(int transmissions) {
    this->transmissionsPerBatch = transmissions;
}

// Modes
void System::setMode(String mode) {
    if (mode == "sending" || mode == "pending" || mode == "disconnected" || mode == "shutdown" || mode == "restart") {
        this->operatingMode = mode;
        return;
    }
    logger.logMessage("System: invalid mode!");
}

void System::loop() {
    // Switch between operating modes.
    if (connectionFailures >= 3 && !connection.isConnected()) {operatingMode = "disconnected";}
    if (operatingMode == "disconnected" && connection.isConnected()) {operatingMode = "sending";}
    if (operatingMode == "sending" && measurementSender.getBufferSize() >= MAX_TRANSMISSIONS_PER_BATCH) {operatingMode = "pending";}
    if (operatingMode == "pending" && measurementSender.getBufferSize() == 0) {operatingMode = "sending";}

    // Restart when recovery is no longer safe.
    if (measurementFailures >= 3) {operatingMode = "restart";} // Measurements should never fail.
    if (connectionFailures >= 3 && connection.isConnected() && !measurementSender.checkPostStatus()) {operatingMode = "restart";}
    if (periodicRestart && (millis() - startTimeMs >= 1 * 60 * 60 * 1000)) {operatingMode = "restart";}

    // Print useful status information.
    logger.logMessage("\nSystem: mode '" + operatingMode + "'");
    Serial.println("System: packets remaining in buffer: " + String(measurementSender.getBufferSize()));

    // Execute the current mode.
    if (operatingMode == "sending") {runSendingMode();}
    else if (operatingMode == "pending") {runPendingMode();}
    else if (operatingMode == "disconnected") {runDisconnectedMode();}
    else if (operatingMode == "shutdown") {runShutdownMode();}
    else if (operatingMode == "restart") {runRestartMode();}
    else {
        logger.logMessage("System: invalid operating mode. Reverting to sending mode.");
        operatingMode = "sending";
    }
}

void System::runSendingMode() {
    takeMeasurement();

    if (measurementSender.getBufferSize() >= transmissionsPerBatch) {
        sendSpreadsheetRows(MAX_TRANSMISSIONS_PER_BATCH);
    }
}
void System::runPendingMode() {
    sendSpreadsheetRows(MAX_TRANSMISSIONS_PER_BATCH);
}

void System::runDisconnectedMode() {
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

void System::runShutdownMode() {
    sendSpreadsheetRows(MAX_TRANSMISSIONS_PER_BATCH);
    logger.logMessage("System: shutting down ESP32...");
    delay(1000);
    esp_deep_sleep_start();
}

void System::runRestartMode() {
    sendSpreadsheetRows(MAX_TRANSMISSIONS_PER_BATCH);
    logger.logMessage("System: restarting ESP32...");
    delay(1000);
    ESP.restart();
}

String System::getMode() {
    return operatingMode;
}

// User interaction
void System::handleCommand(String command) {
    command.trim();

    if (command.equalsIgnoreCase("stop")) {
        operatingMode = "shutdown";
    } else {
        logger.logMessage("System: unknown command.");
    }
}
