# ESP32 Firmware

The ESP32 firmware reads the SCT-013-000 through an ADS1115, buffers the measurements, and sends structured JSON to the FastAPI backend over HTTP or HTTPS.

Its main responsibilities are:

- Calculate RMS current through the `Meter` module.
- Buffer measurements and retry failed transmissions.
- Reconnect to Wi-Fi and handle operating modes.
- Control the onboard LED and physical button.

## Requirements

- ESP32 support installed in Arduino IDE.
- `Adafruit ADS1X15` installed through the Arduino Library Manager. Accept its prompted dependencies, including `Adafruit BusIO`.
- ADS1115 connected to the ESP32 over I2C, with the conditioned sensor signal connected to channel A0.

For ADS1115 wiring and library installation, use the [Adafruit ADS1115 Arduino guide](https://learn.adafruit.com/adafruit-4-channel-adc-breakouts/arduino-code). The project does not vendor these libraries.

## Configuration

1. Copy `secrets.example.h` to `secrets.h`.
2. Set `WIFI_SSID`, `WIFI_PASSWORD`, and `AUTH_TOKEN` in `secrets.h`. This file is ignored by Git.
3. Set `BACKEND_URL` in `firmware.ino`. The URL scheme selects HTTP (`http://`) or HTTPS (`https://`).
4. When using HTTPS, set `TLS_CA_CERTIFICATE` in `firmware.ino` to a CA certificate trusted by the backend certificate chain. Leave it empty for HTTP.
5. Set `BURDEN_RESISTANCE` to the measured burden-resistor value and `TRANSFORMER_TURNS` to the current-transformer ratio. Use the [measurement verification procedure](../verification/README.md) to check them against a clamp meter.

The firmware token must match the backend's `CAPTA_AUTH_TOKEN`.

Example HTTPS configuration:

```cpp
constexpr const char* BACKEND_URL = "https://capta.example.com";
constexpr const char* TLS_CA_CERTIFICATE = R"EOF(
-----BEGIN CERTIFICATE-----
... CA certificate ...
-----END CERTIFICATE-----
)EOF";
```

The server certificate may be renewed without changing this setting when it remains signed through the same CA chain. If the backend changes CA, update the firmware. HTTPS never falls back to an insecure connection.

Optional settings in `firmware.ino` control periodic restart, batch size, measurement delay, progressive reconnection delay, logging, and decimal precision.

## Upload

1. Open `firmware.ino` in Arduino IDE.
2. Select the ESP32 board and port.
3. Compile and upload the firmware.
4. Use the Serial Monitor at 115200 baud to follow its status.

See the [backend documentation](../backend/README.md) for API and deployment instructions.
