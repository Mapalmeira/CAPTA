# Current Measurement Verification

The verification sketch checks the current calculated by the ESP32 firmware. The measurement chain uses an SCT-013-000 current transformer and an ADS1115 external ADC.

## Relevant Components

- 1 × ESP32
- 1 × SCT-013-000 current transformer
- 1 × ADS1115 ADC module
- 2 × 10 kΩ resistors
- 1 × 75 Ω burden resistor
- 1 × 10 µF capacitor
- A clamp meter for reference measurements

The conditioning circuit converts the SCT-013-000 output current into a voltage and adds an offset so that the signal remains positive. Connect that signal to ADS1115 channel A0; the ADS1115 communicates with the ESP32 over I2C.

Use the [OpenEnergyMonitor current-transformer interface guide](https://docs.openenergymonitor.org/electricity-monitoring/ct-sensors/interface-with-arduino.html) for the sensing circuit and the [Adafruit ADS1115 Arduino guide](https://learn.adafruit.com/adafruit-4-channel-adc-breakouts/arduino-code) for ADS1115 wiring and library installation. Power the ADS1115 consistently with the ESP32 logic level and keep its analog input within the module's permitted range.

The Arduino verification sketch requires the `Adafruit ADS1X15` library and reads the conditioned signal from ADS1115 channel A0.

## How the Measurement Works

Each measurement window rejects samples more than two standard deviations from its initial mean. The remaining samples are centered around their mean before the RMS voltage is calculated.

RMS current is then calculated directly from the circuit parameters:

```text
current = (RMS voltage / burden resistance) * transformer turns
```

## Verification Procedure

Use `verification.ino` to compare measurements against a clamp meter:

1. Assemble the sensing circuit and ADS1115.
2. Set `resistance` to the measured burden-resistor value and `transformerTurns` to the current-transformer ratio.
3. Upload the sketch and open the Serial Monitor at 115200 baud.
4. Measure several loads while also reading them with the clamp meter.
5. If a consistent proportional difference remains, verify the burden resistance and transformer ratio.

## Limitations

The project measures current only. Power and energy are estimated using a fixed voltage and assuming current and voltage are in phase, so reactive loads and voltage variation introduce error.

## References

- [OpenEnergyMonitor – Electricity Monitoring](https://docs.openenergymonitor.org/electricity-monitoring/index.html)
- [Adafruit ADS1115 Arduino Guide](https://learn.adafruit.com/adafruit-4-channel-adc-breakouts/arduino-code)
