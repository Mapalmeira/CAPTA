# Data Transmission System Configuration

This directory contains the firmware embedded in the ESP32, responsible for collecting, storing, and sending electric current measurements to a Web App on Google Drive, in an automated and secure manner.

The system is divided into three main layers:

- **Electrical Measurement**: current sensing using the SCT-013-000 sensor.
- **Transmission Management**: organization of measurements and HTTPS POST transmission.
- **Operation Management**: control of operating modes, logs, and physical interface.

---

## Overview of Operation

### Measurement

The ESP32 reads the electric current via the SCT-013-000 sensor, using the `Medidor` module. The readings are processed (RMS calculation) and stored in a buffer for later transmission.

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

- LED blinking at 1 Hz → Normal sending.
- LED blinking at 0.5 Hz → Connection issue detected.
- LED blinking at 2 Hz → Pending transmission.

---

## Installation

### System Configuration

Before using the system, configure the `envio_esp32.ino` file located in `envio/`:

- **Wi-Fi Credentials**: set `SSID` and `PASSWORD`.
- **Web App URL**: configure `ENDERECO_SHEETS`.
- **Authentication Token**: adjust the value of `TOKEN`.

You can also configure:

- Periodic reboot (`REINICIALIZACAO_PERIODICA`).
- Number of transmissions per batch (`ENVIOS_POR_VEZ`).
- Additional delay between measurements (`DELAY_ADICIONAL`).
- Progressive delay on disconnections (`DELAY_PROGRESSIVO`).
- Decimal places definition for transmissions (`CASAS_DECIMAIS`).
  *(Example: {-1, 2} means sending an integer and a decimal with 2 decimal places.)*

For proper meter operation, you must also set:

- Burden resistor resistance (`RESISTENCIA`).
- Number of turns of the current transformer (`VOLTAS_TRANSFORMADOR`).
- Voltage offset (`TENSAO_OFFSET`).
- Linear correction coefficients (`COEFICIENTE_ANGULAR` and `COEFICIENTE_LINEAR`).

These values can be obtained through the calibration procedure described in `hardware/`.

### Installation Steps

1. Add the contents of `envio/` to a folder named "envio_esp32".
2. Open the "envio_esp32" folder as a project in the Arduino IDE.
3. Configure `envio_esp32.ino` as described in the previous section.
4. Connect the ESP32 to your computer.
5. Compile and upload the code to the ESP32.
6. Use the Serial Monitor to track the device status.

---

## Notes

- The system includes connection verification with the Web App and reschedules transmissions in case of failures.
- The WiFiClientSecure library is used in insecure mode (`client.setInsecure()`), without certificate validation, because Google Drive's Apps Script uses dynamic certificates that are hard to validate at runtime.

---

## References

- [OpenEnergyMonitor – Electricity Monitoring](https://docs.openenergymonitor.org/electricity-monitoring/index.html)
