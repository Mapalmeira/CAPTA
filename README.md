# CAPTA – Automatic Power Collection and Storage with Transmission

## Electrical Consumption Monitoring with ESP32 and FastAPI

This project is a complete system for measuring, sending, storing, and visualizing electrical consumption data, using an ESP32 as the measurement unit and a FastAPI application as the backend.

The system is divided into three main parts:

1. **Measurement and Calibration**: includes the current-sensing circuit, ADS1115 converter, and calibration utilities.
2. **ESP32 Firmware**: measures current and periodically sends the readings.
3. **Reception and Visualization**: a Python backend that validates structured requests, stores daily CSV files, and serves a small web dashboard.

## Components Used

- **ESP32**: the main microcontroller.
- **SCT-013-000 Sensor**: a current transformer for AC current measurement.
- **ADS1115**: an external 16-bit ADC used to digitize the conditioned sensor signal over I2C.
- **Electronic components**: one 10μF capacitor, one 75 Ω resistor, two 10kΩ resistors.

## General Operation

### Measurement

The SCT-013-000 signal is conditioned and read through the ADS1115. The firmware calculates RMS current, assumes a fixed voltage value such as 110 V or 220 V, and presumes current and voltage are in phase.

### Data Transmission

Measurements are sent as structured JSON through authenticated HTTP or HTTPS POST requests to the FastAPI backend.

Small samples are sent periodically throughout the day. This setup is ideal for real-time monitoring and continuous tracking in areas with internet access.

The system features:
- A measurement buffer to ensure reliability.
- Automatic retransmission logic in case of failure.
- Preventive reboots.

### Reception

The backend validates each request and stores measurements in one CSV file per day.

### Visualization

The backend serves a plain HTML and JavaScript dashboard that allows:

- Viewing instantaneous power over a day.
- Viewing daily consumption (kWh) for the 30 days from a selected initial date.
- Zooming and horizontally scrolling charts to inspect shorter periods.
- Exporting each day as CSV.


## Next Steps

Access the specific READMEs for detailed information:

- [`calibration/`](./calibration/README.md): circuit notes and calibration procedure.
- [`firmware/`](./firmware/README.md): ESP32 firmware configuration and data transmission.
- [`backend/`](./backend/README.md): FastAPI API, CSV storage, dashboard, and Podman instructions.

## References

- [OpenEnergyMonitor Docs](https://docs.openenergymonitor.org/electricity-monitoring/index.html)
- [Adafruit ADS1115 Guide](https://learn.adafruit.com/adafruit-4-channel-adc-breakouts/arduino-code)
