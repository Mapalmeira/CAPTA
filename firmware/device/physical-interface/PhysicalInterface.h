#ifndef PHYSICAL_INTERFACE_H
#define PHYSICAL_INTERFACE_H

class PhysicalInterface {
    public:
        using GetModeFunction = std::function<String(void)>;
        using CommandFunction = std::function<void(String)>;

        PhysicalInterface();

        void begin(GetModeFunction getMode, CommandFunction command);

    private:
        GetModeFunction getMode;
        CommandFunction command;

        void runLedLoop();
        void runButtonLoop();

        static void ledTask(void* parameter);
        static void buttonTask(void* parameter);

        bool ledState = false;
        bool previousButtonState = HIGH;
        unsigned long lastPressedAt = 0;

        static const int LED_PIN = 2;
        static const int BUTTON_PIN = 0;
};

#endif
