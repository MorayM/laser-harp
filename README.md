# Laser Harp Control System

## Overview

This repository contains the firmware and supporting documentation for a **laser harp control system** built around an **Arduino Mega 2560**.
The system reads multiple optical string sensors, generates MIDI events in real time, controls laser beam visibility for visual effects, and provides a local user interface via an I²C LCD.

The design prioritises:

* deterministic, low-latency sensor scanning,
* reliable MIDI output,
* clear separation between **setup/configuration** and **performance/play** behaviour,
* robustness against sensor noise and mechanical relay chatter.

---

## Hardware Summary

### Core Controller

* **Arduino Mega 2560**

  * Chosen for abundant GPIO, multiple hardware UARTs, and development simplicity.
  * USB serial is reserved for debugging.
  * `Serial1` is used exclusively for MIDI I/O.

### Optical String Sensors

* **Photodiodes** feeding **reverse-biased op-amp comparator circuits**
* Output drives **reed relays**, producing a clean digital on/off signal per string.
* Logic represents **beam intact vs beam interrupted** (polarity configurable in software).
* Sensors are read as **digital inputs**, one per string.

> Note: Although the original circuit was derived from *The Art of Electronics*, the firmware assumes only a clean digital signal and does not depend on the analog implementation details.

### Laser Beam Control

* Each beam has an associated **digital output** used to enable/disable it for visual effects.
* Beam control is logically independent from sensor input.

### User Interface

* **Adafruit RGB LCD Shield Kit with 16×2 character display**
  * https://www.adafruit.com/product/714
  * Communicates over **I²C** (SDA/SCL).
  * Used to display system state and adjust configuration (e.g. MIDI mode, note mapping).
  * LCD updates are rate-limited to avoid impacting real-time performance.

### MIDI Interface

* **DataPoint MIDI Serial Interface**
  * https://datapoint.uk/product/midi-serial-interface-board-3-3v-or-5v-with-thru-led-arduino-raspberry-pi/
  * Standard 5-pin DIN MIDI IN/OUT.
  * Connected to **Serial1** on the Mega (TX1/RX1).
  * MIDI baud rate: **31,250 bps**.

### Footswitch

* Single digital input.
* Acts as a **kill switch**:

  * Immediately sends Note Off events for all active strings.
  * Suppresses further note generation while engaged.

---

## I/O Allocation (Conceptual)

| Function                | Quantity | Interface Type |
| ----------------------- | -------- | -------------- |
| String sensors          | 10       | Digital input  |
| Laser beam enable lines | 10       | Digital output |
| Footswitch              | 1        | Digital input  |
| LCD UI                  | 1        | I²C            |
| MIDI IN / OUT           | 1        | UART (Serial1) |

Total digital usage fits comfortably within the Mega’s capabilities, with headroom for future expansion.

---

## Software Architecture

### Operating Modes

The firmware is explicitly divided into two high-level modes:

1. **Setup Mode**

   * Handles UI interaction (LCD + buttons).
   * Allows configuration of MIDI behaviour and string-to-note mappings.
   * Timing constraints are relaxed.

2. **Play Mode**

   * Optimised for real-time performance.
   * UI activity is minimised and rate-limited.
   * Sensor scanning and MIDI output take priority.

Mode separation prevents UI logic from introducing latency or jitter during performance.

---

## Play Mode Processing Model

In play mode, the main loop follows a deterministic scan-and-diff pattern:

1. **Scan sensors**

   * All string inputs are sampled in a tight loop.
   * Results are stored in a **bitmask** (one bit per string).

2. **Debounce / filter**

   * Each channel must remain stable for a short, fixed time window before being accepted.
   * This suppresses relay chatter and optical noise.

3. **Edge detection**

   * Current stable bitmask is XORed with the previous stable bitmask.
   * Result identifies exactly which strings changed state.

4. **MIDI event generation**

   * Rising edges → Note On
   * Falling edges → Note Off
   * MIDI messages are emitted via `Serial1`.

5. **Display update (throttled)**

   * LCD is refreshed at a fixed low rate (e.g. 10–20 Hz).
   * Prevents I²C/LCD latency from affecting scan timing.

6. **State commit**

   * Current bitmask becomes the previous bitmask for the next loop.

This approach guarantees:

* consistent event ordering,
* predictable latency,
* clean handling of simultaneous string changes.

---

## MIDI Design Notes

* Standard 3-byte MIDI Note On / Note Off messages are used.
* Channel is configurable in firmware.
* USB serial remains available for debugging and logging.
* Optional global “All Notes Off” logic can be applied via CC 123 if required.

---

## Design Principles

* **Determinism over micro-optimisation**
  Sampling all inputs before emitting events ensures consistency and avoids scan-order artefacts.

* **Hardware UART for MIDI**
  Avoids timing issues associated with SoftwareSerial.

* **Non-blocking UI**
  Display updates are explicitly prevented from interfering with real-time logic.

* **Explicit state tracking**
  Bitmasks and edge detection keep logic simple, fast, and auditable.

---

## Future Extensions (Non-Goals for Initial Version)

* Shift-register or I/O-expander based sensor multiplexing.
* Velocity or continuous control derived from beam interruption timing.
* Polyphonic aftertouch or MPE output.
* Host-side configuration tools.

---

## Repository Intent

This repository is intended to be:

* a stable reference implementation for the laser harp controller,
* suitable for public workshops and demonstrations,
* readable by humans and LLMs alike without excessive context overhead.

---

If you want, next steps could include:

* a `PINOUT.md` with exact pin numbers,
* a timing/latency analysis section,
* or a short “hardware assumptions” appendix documenting sensor polarity and relay behaviour.
