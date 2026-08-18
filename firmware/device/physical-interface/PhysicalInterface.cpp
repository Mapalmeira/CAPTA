#include "PhysicalInterface.h"
#include <Arduino.h>

PhysicalInterface::PhysicalInterface() {}

void PhysicalInterface::begin(GetModeFunction getModeFunction, CommandFunction commandFunction) {
    this->getMode = getModeFunction;
    this->command = commandFunction;

    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    xTaskCreatePinnedToCore(
        PhysicalInterface::ledTask,
        "LedTask",
        1024,
        this,
        1,
        nullptr,
        0
    );

    xTaskCreatePinnedToCore(
        PhysicalInterface::buttonTask,
        "ButtonTask",
        1024,
        this,
        1,
        nullptr,
        0
    );
}

void PhysicalInterface::ledTask(void* parameter) {
    PhysicalInterface* self = static_cast<PhysicalInterface*>(parameter);
    while (true) {
        self->runLedLoop();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void PhysicalInterface::buttonTask(void* parameter) {
    PhysicalInterface* self = static_cast<PhysicalInterface*>(parameter);
    while (true) {
        self->runButtonLoop();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void PhysicalInterface::runLedLoop() {
    static int lastBlinkAt = 0;
    int currentTime = millis();
    int interval = 1000;

    String mode = getMode();

    if (mode == "sending") {
        interval = 1000;
    } else if (mode == "disconnected") {
        interval = ledState ? 50 : 2000;
    } else if (mode == "pending") {
        interval = 500;
    }

    if (currentTime - lastBlinkAt >= interval) {
        lastBlinkAt = currentTime;
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState);
    }
}

void PhysicalInterface::runButtonLoop() {
    bool currentState = digitalRead(BUTTON_PIN);

    if (previousButtonState == HIGH && currentState == LOW) {
        if (millis() - lastPressedAt > 500) {
            command("stop");
            lastPressedAt = millis();
        }
    }

    previousButtonState = currentState;
}
