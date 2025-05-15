# Electrical Current Measurement Hardware with ESP32

This directory contains the files and instructions for assembling the current measurement system based on an ESP32, including an explanation of the measurement logic.

---

## Sensor Circuit Components

The reading circuit uses the SCT-013-000 current sensor with the following components:

- 1 × SCT-013-000 current sensor
- 2 × 10 kΩ resistors
- 1 × 75 Ω resistor (burden resistor)
- 1 × 10 µF capacitor

### Notes

The 75 Ω burden resistor was chosen considering a maximum current of 32 A, a typical value for main breakers in Brazilian households. This choice ensures that the sensor’s output voltage, even under high current conditions, remains within the safe operating range of the ESP32’s internal ADC, which supports up to 3.3 V. 

Lower resistor values can be used safely, although they alter the measurement sensitivity. Higher values **must not be used**, as they can generate voltages above the microcontroller's supported limit, risking damage to the system.

The circuit assembly follows the scheme proposed by the OpenEnergyMonitor project, a consolidated reference in energy monitoring with microcontrollers.

Illustrative images and wiring diagrams are available in the `imagens/` folder. The analog reading of the signal is performed on pin **36** of the ESP32 (identified as `SP` in the diagrams).

The assembly guide can be found at:
[https://docs.openenergymonitor.org/electricity-monitoring/ct-sensors/interface-with-arduino.html](https://docs.openenergymonitor.org/electricity-monitoring/ct-sensors/interface-with-arduino.html)

---

## Measurement Logic

The SCT-013-000 is a current transformer with a turns ratio of 2000:1. It converts an input current I into an output current of I divided by 2000.

Since the ESP32 performs **voltage** readings (between 0 and 3.3 V), the current must be converted into a compatible voltage range. This is done in two stages:

1. **Current to voltage conversion:**
   The sensor’s output current passes through a burden resistor. The generated voltage oscillates approximately between -I/2000 × √2 × R and +I/2000 × √2 × R, where R is the resistor value in ohms, carefully chosen to keep the amplitude between -3.3/2 and 3.3/2.

2. **Offset for unipolar reading:**
   Since the ESP32 cannot read negative voltages, an **offset of 1.65 V** is added to the signal so that it is centered between 0 and 3.3 V.

---

## Calibration

This section explains the auxiliary codes available in `hardware/`, which are essential for sensor calibration:

### 1. `offset.ino`

Used to measure the sensor’s offset when disconnected from any load.

**Steps:**
- Assemble the ESP32+sensor circuit according to the provided schematic.
- Open `offset.ino` in the Arduino IDE.
- Run the program with the sensor disconnected from any load.
- Read the serial output a few times and record the average observed offset voltage.

### 2. `calibracao.ino`

Allows the calibration of current measurement.

**Steps:**
- Open the code in the Arduino IDE.
- Insert the offset value obtained in the previous step into the `tensao_offset` variable.
- Adjust the `resistencia` variable to reflect the actual burden resistor value (measure it with a multimeter).
- Connect the sensor to different loads.
- Compare the ESP32 readings from the serial output with readings from a clamp-type ammeter.

### 3. `correcao_linear.py`

If the values obtained in the previous step show discrepancies, a linear correction must be applied.

This script adjusts the measured values to better match the real consumption.

**Steps:**
- Open the script in your preferred text editor and replace the `dados` list with a list of pairs (measured_value, expected_value).
- Run the script. A graph will appear showing the linear regression.
- The coefficients `a` and `b` will be shown in the legend in the form f(x) = ax + b.
- Insert these coefficients into the corresponding fields in `calibracao.ino`:

    double coeficiente_angular = ...; // Regression slope, the obtained `a`.
    double coeficiente_linear  = ...; // Regression intercept, the obtained `b`.

- Repeat the calibration step and check if the accuracy is within expectations.

**Note**:
Even without correction, the circuit is expected to perform reasonably well.
If the obtained coefficients are very high or if the regression graph points are very scattered, it is likely that a mistake was made in previous calibration steps.

---

## Measurement Limitations

Despite its simplicity and low cost, this type of measurement has some limitations:

### Lack of Instantaneous Voltage Measurement

The current system only measures **instantaneous current**. Power is estimated assuming a **constant voltage** (e.g., 110 V or 220 V).
For residential use, this approximation is usually sufficient, as voltage tends to be stable and current and voltage are typically in phase.

However, in industrial environments (with reactive loads), this approach can introduce significant errors.
In such cases, it is recommended to add a **real-time voltage measurement module** to the circuit.

### Internal ADC Precision

The ESP32’s analog-to-digital converter (ADC) has precision limitations and can introduce errors.
While acceptable for general use, the system can be improved by adding an external ADC, such as the ADS1115, if greater accuracy is required.

---

## Reference

- [OpenEnergyMonitor – Electricity Monitoring](https://docs.openenergymonitor.org/electricity-monitoring/index.html)

---

## Next Steps

Access the specific READMEs for detailed information:

- [`hardware/`](../hardware/README.md): sensors, modules, and measurement logic.
- [`envio/`](../envio/README.md): ESP32 firmware logic and data transmission.
- [`servidor/`](../servidor/README.md): Apps Script, Drive storage, and visualization spreadsheets.

---

## `hardware/` Contents

- `offset.ino` – Code to measure the circuit’s offset.
- `calibracao.ino` – Code to calibrate current measurement.
- `correcao_linear.py` – Code to obtain linear regression coefficients.
- `imagens/` – Wiring diagrams for connecting the modules to the ESP32.