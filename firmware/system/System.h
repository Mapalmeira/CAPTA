#ifndef SYSTEM_H
#define SYSTEM_H

#include <Arduino.h>
#include <functional>
#include <vector>
#include "connection/Connection.h"
#include "measurement-sender/MeasurementSender.h"
#include "logger/Logger.h"
#include "physical-interface/PhysicalInterface.h"

class System {
private:
    // Connection
    String SSID;
    String password;
    String appsScriptUrl;

    // Operating mode
    String operatingMode;

    // Customizations
    bool progressiveDelay = false; // In disconnected mode, the interval grows exponentially up to 10 minutes per measurement.
    bool saveLogs = false;
    bool periodicRestart = false;
    int transmissionsPerBatch = 1;
    int additionalDelayMs;
    int startTimeMs;

    // Components
    Connection connection;
    Logger logger;
    MeasurementSender measurementSender;
    PhysicalInterface physicalInterface;

    // Error counters
    int connectionFailures = 0;
    int measurementFailures = 0;

    // Transmission
    int MAX_TRANSMISSIONS_PER_BATCH = 50;

public:
    System(String ssid, String password, String url, String token,
        std::function<Measurement(void)> measurementFunction,
        std::vector<int> decimalPlaces);

    void begin();
    void loop();

    // Connection
    bool connectWiFi();

    // Measurement
    bool takeMeasurement();

    // Transmission
    String sendSpreadsheetRows(int rows);

    // Customizations
    void setAdditionalDelay(int delayMs);
    void setProgressiveDelay(bool enabled);
    void setLogging(bool enabled);
    void setPeriodicRestart(bool enabled);
    void setTransmissionsPerBatch(int transmissions);

    // Modes
    void setMode(String mode);
    void runSendingMode();
    void runPendingMode();
    void runDisconnectedMode();
    void runShutdownMode();
    void runRestartMode();
    String getMode();

    // User interaction
    void handleCommand(String command);
};

#endif
