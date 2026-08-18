# Data Transmission System Configuration

This directory contains the firmware embedded in the ESP32, responsible for collecting, storing, and sending electric current measurements to a Web App on Google Drive, in an automated and secure manner.

The system is divided into three main layers:

- **Electrical Measurement**: current sensing using the SCT-013-000 sensor.
- **Transmission Management**: organization of measurements and HTTPS POST transmission.
- **Operation Management**: control of operating modes, logs, and physical interface.

## Overview of Operation

### Measurement

The ESP32 reads the electric current via the SCT-013-000 sensor, using the `Meter` module. The readings are processed (RMS calculation) and stored in a buffer for later transmission.

### Intelligent Management

The system automatically adapts its operation:

- Measurement buffer for retransmission in case of failure.
- Automatic Wi-Fi reconnection.
- Preventive reboot in case of critical errors.
- Configurable operation modes:
  - Sending
  - Pending
  - Disconnected
  - Shutdown
  - Reboot

### Physical Interface

A physical button allows simple interactions (such as sending a shutdown command), and an integrated LED indicates the current operation mode:

- LED blinking at 1 Hz -> Normal sending.
- LED blinking at 0.5 Hz -> Connection issue detected.
- LED blinking at 2 Hz -> Pending transmission.

## Installation

### Installation Steps

1. Download the `firmware/` folder.
2. Open `firmware/` as a project in Arduino IDE.
3. Configure the parameters in `firmware.ino`, as described in the `System Configuration` section.
4. Connect the ESP32 to your computer.
5. Compile and upload the code to the ESP32.
6. Check the Serial Monitor to track the device status.

### System Configuration

Before using the system, configure the `firmware.ino` file located in `firmware/`:

- **Wi-Fi Credentials**: set `SSID` and `PASSWORD`.
- **Web App URL**: deploy the scripts in [`server/reception/`](../../server/reception/) and set the resulting URL in `APPS_SCRIPT_URL`.
- **Authentication Token**: set the `TOKEN` value, which must be identical on both the ESP32 and the web application to ensure secure and authorized communication.

You can also configure:

- Periodic reboot (`PERIODIC_RESTART`).
- Number of transmissions per batch (`TRANSMISSIONS_PER_BATCH`).
- Additional delay between measurements (`ADDITIONAL_DELAY_MS`).
- Progressive delay on disconnections (`PROGRESSIVE_DELAY`).
- Decimal places to send (`DECIMAL_PLACES`).
  *(Example: {-1, 2} means sending an integer and a decimal with 2 decimal places.)*

For proper operation of the meter, you must also set:

- Burden resistor value (`BURDEN_RESISTANCE`).
- Number of turns in the current transformer (`TRANSFORMER_TURNS`).
- Voltage offset (`VOLTAGE_OFFSET`).
- Linear correction coefficients (`SCALE_COEFFICIENT` and `INTERCEPT_COEFFICIENT`).

These values should be obtained through the calibration procedure described in [`hardware/`](../hardware/README.md).

## Notes

- The system includes connection verification with the Web App and reschedules transmissions in case of failures.
- The WiFiClientSecure library is used in insecure mode (`client.setInsecure()`), without certificate validation.

## Next Steps

Access the specific READMEs for detailed information:

- [`hardware/`](../hardware/README.md): sensors, modules, and measurement logic.
- [`data-transmission/`](../data-transmission/README.md): ESP32 firmware logic and data transmission.
- [`server/`](../../server/): Apps Script for Drive storage and visualization spreadsheets.

## References

- [OpenEnergyMonitor – Electricity Monitoring](https://docs.openenergymonitor.org/electricity-monitoring/index.html)
