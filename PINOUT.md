# Pinout Reference — Laser Harp Control System

This document defines the **logical and physical pin assignments** for the laser harp controller based on an **Arduino Mega 2560**.

Pin choices prioritise:
- clarity and grouping,
- avoidance of conflicts with USB serial,
- future expandability.

---

## Controller Board

- **Board:** Arduino Mega 2560
- **MCU:** ATmega2560
- **Voltage:** 5V logic
- **USB Serial:** Used for debugging only

---

## Digital Inputs — String Sensors

Each string sensor produces a **digital on/off signal** derived from:
photodiode → op-amp comparator → reed relay.

| String | Arduino Pin | Direction | Notes |
|------:|------------:|-----------|-------|
| 1 | D22 | Input | Beam state |
| 2 | D23 | Input | Beam state |
| 3 | D24 | Input | Beam state |
| 4 | D25 | Input | Beam state |
| 5 | D26 | Input | Beam state |
| 6 | D27 | Input | Beam state |
| 7 | D28 | Input | Beam state |
| 8 | D29 | Input | Beam state |
| 9 | D30 | Input | Beam state |
| 10 | D31 | Input | Beam state |

- Typically configured as `INPUT_PULLUP`
- Software treats **LOW = beam broken** by default (configurable)

---

## Digital Outputs — Laser Beam Enable

Each beam can be enabled or disabled independently for visual effects.

| Beam | Arduino Pin | Direction | Notes |
|-----:|------------:|-----------|-------|
| 1 | D32 | Output | Laser enable |
| 2 | D33 | Output | Laser enable |
| 3 | D34 | Output | Laser enable |
| 4 | D35 | Output | Laser enable |
| 5 | D36 | Output | Laser enable |
| 6 | D37 | Output | Laser enable |
| 7 | D38 | Output | Laser enable |
| 8 | D39 | Output | Laser enable |
| 9 | D40 | Output | Laser enable |
| 10 | D41 | Output | Laser enable |

- Logic level and driver circuitry depend on laser hardware
- Timing of these outputs is non-critical relative to MIDI

---

## Footswitch (Kill Switch)

| Function | Arduino Pin | Direction | Notes |
|---------|-------------|-----------|-------|
| Kill switch | D42 | Input | Active LOW |

Behaviour:
- Immediately sends Note Off for all active strings
- Suppresses further MIDI output while engaged

---

## LCD User Interface (I²C)

- **Device:** Adafruit RGB LCD Shield Kit (16×2)
- **Bus:** I²C

| Signal | Arduino Pin |
|------|-------------|
| SDA | D20 |
| SCL | D21 |

Notes:
- LCD updates are rate-limited in firmware
- Shield keypad (if used) also communicates via I²C
- Default I²C address: `0x20`

---

## MIDI Interface (Olimex SHIELD-MIDI)

- **UART:** Serial1
- **Baud rate:** 31,250 bps (standard MIDI)

| MIDI Signal | Arduino Pin |
|------------|-------------|
| TX (MIDI OUT) | D18 (TX1) |
| RX (MIDI IN) | D19 (RX1) |

Notes:
- USB Serial (`Serial`) remains free for debugging
- No SoftwareSerial is used for MIDI

---

## Reserved / Unused Pins

- D0 / D1: USB Serial (do not use for MIDI or sensors)
- Additional digital and analog pins remain available for expansion

---

## Assumptions

- Sensor outputs are electrically clean but may exhibit relay bounce
- Software debounce is applied regardless of analog hysteresis
- All timing-critical logic resides in play mode only

---
