# Current Measurement and Calibration

The calibration utilities determine the voltage offset and correction coefficients used by the ESP32 firmware. The measurement chain uses an SCT-013-000 current transformer and an ADS1115 external ADC.

## Relevant Components

- 1 × ESP32
- 1 × SCT-013-000 current transformer
- 1 × ADS1115 ADC module
- 2 × 10 kΩ resistors
- 1 × 75 Ω burden resistor
- 1 × 10 µF capacitor
- A clamp meter for calibration reference

The conditioning circuit converts the SCT-013-000 output current into a voltage and adds an offset so that the signal remains positive. Connect that signal to ADS1115 channel A0; the ADS1115 communicates with the ESP32 over I2C.

Use the [OpenEnergyMonitor current-transformer interface guide](https://docs.openenergymonitor.org/electricity-monitoring/ct-sensors/interface-with-arduino.html) for the sensing circuit and the [Adafruit ADS1115 Arduino guide](https://learn.adafruit.com/adafruit-4-channel-adc-breakouts/arduino-code) for ADS1115 wiring and library installation. Power the ADS1115 consistently with the ESP32 logic level and keep its analog input within the module's permitted range.

Both Arduino calibration sketches require the `Adafruit ADS1X15` library and read the conditioned signal from ADS1115 channel A0.

## Calibration Procedure

### 1. Measure the offset

Use `offset.ino` to measure the circuit's idle voltage:

1. Assemble the sensing circuit and ADS1115.
2. Open `offset.ino` in Arduino IDE.
3. Run it with the sensor disconnected from any load.
4. Record the voltage printed in the Serial Monitor.

### 2. Calibrate current measurement

Use `calibration.ino` to compare measurements against a reference:

1. Set `voltageOffset` to the result from `offset.ino`.
2. Set `resistance` to the measured burden-resistor value.
3. Measure several loads while also reading them with a clamp meter.
4. Record pairs of measured and expected current values.

### 3. Calculate linear correction

If a systematic difference remains, edit the `data` list in `linear_correction.py` with `(measured_value, expected_value)` pairs and run the script. Copy the resulting slope and intercept to `SCALE_COEFFICIENT` and `INTERCEPT_COEFFICIENT` in [`firmware.ino`](../firmware/firmware.ino), then repeat the comparison.

Large coefficients or widely scattered samples usually indicate a circuit or earlier calibration problem.

## Limitations

The project measures current only. Power and energy are estimated using a fixed voltage and assuming current and voltage are in phase, so reactive loads and voltage variation introduce error.

## References

- [OpenEnergyMonitor – Electricity Monitoring](https://docs.openenergymonitor.org/electricity-monitoring/index.html)
- [Adafruit ADS1115 Arduino Guide](https://learn.adafruit.com/adafruit-4-channel-adc-breakouts/arduino-code)
