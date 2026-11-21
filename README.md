# Smart Helmet / Smart Bike Safety System (ARM7 LPC2148)

A simulation-based two-wheeler rider safety system on the ARM7 LPC2148, written in Embedded C (Keil uVision) and tested in Proteus.

![Proteus schematic](images/proteus_schematic.png)

## Features

- **RFID access control** over UART0. The system stays locked until a valid tag (`1`, `2` or `3`) is read.
- **Alcohol detection.** Above the threshold, the system shows an alert and cancels access, so the rider must scan the RFID tag again.
- **Gas leak alert** from the gas sensor.
- **Accident detection** when tilt and vibration both cross their thresholds. Tilt alone shows `BIKE TILTED`, and vibration alone shows `HIGH VIBRATION`.
- **Automatic rain canopy.** When rain is detected, a servo driven by PWM opens the canopy, and it closes again when the rain stops.
- **Live readings** of rain, alcohol, gas and vibration on a 16x2 LCD, plus an LED that blinks on every alert.

## How it works

1. On power-up, the LCD shows `SMART BIKE SYS / SHOW RFID`.
2. A valid RFID tag unlocks the system (`ACCESS GRANTED`), and an invalid tag shows `ACCESS DENIED`.
3. Once unlocked, the main loop reads five analog sensors through the on-chip ADC, compares each one to its threshold, and updates the LCD, LED and servo.

## Pin connections

| Function | LPC2148 pin |
|---|---|
| RFID reader (UART0 RXD / TXD) | P0.1 / P0.0 |
| Alert LED | P0.3 |
| LCD data D0–D7 | P1.24 – P1.31 |
| LCD RS / RW / EN | P1.16 / P1.17 / P1.18 |
| Servo (PWM5) | P0.21 |
| Alcohol sensor | AD0.0 (P0.27) |
| Tilt sensor (potentiometer) | AD0.1 (P0.28) |
| Gas sensor | AD0.2 (P0.29) |
| Vibration sensor | AD0.3 (P0.30) |
| Rain sensor | AD0.4 (P0.25) |

## Alert thresholds (10-bit ADC)

| Sensor | Threshold |
|---|---|
| Gas | 800 |
| Rain | 700 |
| Vibration | 600 |
| Alcohol | 650 |
| Tilt | 700 |

## Repository layout

```
src/main.c                    firmware source (Embedded C, Keil uVision)
hex/smart_bike_system.hex     compiled firmware to load into the MCU in Proteus
proteus/                      Proteus project (.pdsprj)
images/                       schematic and simulation screenshots
```

## Running the simulation

1. Open the Proteus project from `proteus/`.
2. Double-click the microcontroller and set **Program File** to `hex/smart_bike_system.hex`.
3. Run the simulation, send a tag ID (`1`, `2` or `3`) through the RFID virtual terminal, then vary the sensor potentiometers to trigger each alert.

To rebuild the hex, create a Keil uVision project for the LPC2148, add `src/main.c`, enable **Create HEX File** under Options for Target → Output, and build.

## Notes and future work

- Sensors are modelled as analog inputs in the simulation, and tilt is simulated with a potentiometer.
- The schematic includes a SIM900D GSM module, but the firmware does not use it yet. Sending an SMS alert on accident detection over UART1 is planned as future work.
