#include "Logger.h"

Logger::Logger() {}

// Reserved for a future implementation.

void Logger::begin(boolean saveLogs) {
    if (!saveLogs) {
        return;
    }
}

void Logger::logMessage(String message) {
    // Display the message in the Serial Monitor.
    Serial.println(message);
}
