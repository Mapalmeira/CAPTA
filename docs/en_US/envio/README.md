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

### Installation Steps

1. Download the `envio/` folder.
2. Open `envio/` as a project in Arduino IDE.
3. Configure the parameters in `envio.ino`, as described in the **System Configuration** section.
4. Connect the ESP32 to your computer.
5. Compile and upload the code to the ESP32.
6. Check the Serial Monitor to track the device status.

### System Configuration

Before using the system, configure the `envio.ino` file located in `envio/`:

- **Wi-Fi Credentials**: set `SSID` and `PASSWORD`.
- **Web App URL**: set `ENDERECO_APPSCRIPT`, as obtained in [`servidor/`](../servidor/README.md).
- **Authentication Token**: set the `TOKEN` value, which must be identical on both the ESP32 and the web application to ensure secure and authorized communication.

You can also configure:

- Periodic reboot (`REINICIALIZACAO_PERIODICA`).
- Number of transmissions per batch (`ENVIOS_POR_VEZ`).
- Additional delay between measurements (`DELAY_ADICIONAL`).
- Progressive delay on disconnections (`DELAY_PROGRESSIVO`).
- Decimal places to send (`CASAS_DECIMAIS`).
  *(Example: {-1, 2} means sending an integer and a decimal with 2 decimal places.)*

For proper operation of the meter, you must also set:

- Burden resistor value (`RESISTENCIA`).
- Number of turns in the current transformer (`VOLTAS_TRANSFORMADOR`).
- Voltage offset (`TENSAO_OFFSET`).
- Linear correction coefficients (`COEFICIENTE_ANGULAR` and `COEFICIENTE_LINEAR`).

These values should be obtained through the calibration procedure described in [`hardware/`](../hardware/README.md).

---

## Notes

- The system includes connection verification with the Web App and reschedules transmissions in case of failures.
- The WiFiClientSecure library is used in insecure mode (`client.setInsecure()`), without certificate validation, because Google Drive's Apps Script uses dynamic certificates that are hard to validate at runtime.

---

## Next Steps

Access the specific READMEs for detailed information:

- [`hardware/`](../hardware/README.md): sensors, modules, and measurement logic.
- [`envio/`](../envio/README.md): ESP32 firmware logic and data transmission.
- [`servidor/`](../servidor/README.md): Apps Script, Drive storage, and visualization spreadsheets.

---

## References

- [OpenEnergyMonitor – Electricity Monitoring](https://docs.openenergymonitor.org/electricity-monitoring/index.html)
