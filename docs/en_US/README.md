# CAPTA – Automatic Power Collection and Storage with Transmission

## Electrical Consumption Monitoring with ESP32 and Google Drive

This project is a complete system for measuring, sending, storing, and visualizing electrical consumption data, using an ESP32 as the measurement unit and Google Drive as the storage and visualization backend.

The system is divided into three main parts:

1. **Hardware and Measurement**: includes the circuit connected to the ESP32 and the measurement logic.
2. **Data Transmission**: embedded firmware on the ESP32 responsible for periodically sending the measurements.
3. **Reception and Visualization**: scripts hosted on Google Apps Script that receive the data, store it in spreadsheets or `.csv` files, and display interactive visualizations.

---

## Components Used

- **ESP32**: the main microcontroller.
- **SCT-013-000 Sensor**: a current transformer for AC current measurement.
- **Electronic components**: one 10μF capacitor, one 75 Ω resistor, two 10kΩ resistors.
- **microSD card (and external module)** (optional): for local data storage and configuration files.

---

## General Operation

### Measurement

The ESP32 reads electrical current using the SCT-013-000 sensor, following recommendations from the OpenEnergyMonitor project. Measurement is based on the root mean square (RMS) of the current, assumes a fixed voltage value (such as 110V or 220V), and presumes current and voltage are in phase — which is usually valid for typical residential environments.

### Data Transmission

All data is sent to Google Drive exclusively via **POST** requests, directed to a Web App developed with Google Apps Script.

Small samples are sent periodically throughout the day. This setup is ideal for real-time monitoring and continuous tracking in areas with internet access.

The system features:
- A measurement buffer to ensure reliability.
- Automatic retransmission logic in case of failure.
- Preventive reboots.

### Reception

On the server side, the Apps Script handles the POST requests, storing the received data in daily spreadsheets inside the `dadosPart/` folder.

### Visualization

The data is visualized through a Google Sheets spreadsheet with an associated script. The spreadsheet allows:

- Viewing **instantaneous power over a day**.
- Viewing **daily consumption (kWh)** for the past 30 days.
- Filtering by specific time intervals (e.g., only between 9 AM and 6 PM).
- Triggering graph generation through simple cell triggers.

---

## Important Notes

- The system currently sends only `[timestamp, current]` pairs, but the codebase is structured to easily allow expansion for other types of measurements.
- Accuracy is acceptable for residential environments, but technical limitations exist (see the hardware README).

---

## Next Steps

Access the specific READMEs for detailed information:

- [`hardware/`](./hardware/README.md): sensors, modules, and measurement logic.
- [`envio/`](./envio/README.md): ESP32 firmware logic and data transmission.
- [`servidor/`](./servidor/README.md): Apps Script, Drive storage, and visualization spreadsheets.

---

## References

- [OpenEnergyMonitor Docs](https://docs.openenergymonitor.org/electricity-monitoring/index.html)
