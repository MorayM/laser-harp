# Timing and Latency Analysis — Laser Harp Control System

This document describes the **timing characteristics, latency sources, and design decisions**
for the laser harp controller firmware.

The goal is **predictable, musically stable behaviour**, not absolute minimum latency at the expense of robustness.

---

## System Timing Model

The firmware operates in a **single main loop** with no RTOS and no blocking delays in play mode.

Key principles:
- All sensors are sampled first
- State changes are detected via bitmask diffing
- MIDI events are emitted after state determination
- UI updates are explicitly rate-limited

---

## Sensor Path Latency

### Hardware Contributors
1. Photodiode response time
2. Op-amp comparator response
3. Reed relay pull-in / release time

These dominate total latency and are typically in the **hundreds of microseconds to low milliseconds** range.

### Software Contributors
- Digital read loop over 10 inputs
- Software debounce filter

Digital reads across 10 pins complete in **microseconds** on an ATmega2560 and are negligible relative to hardware latency.

---

## Software Debounce Strategy

Each sensor channel uses a **time-stable debounce**:

- A new state must persist continuously for `N` milliseconds before being accepted.
- Typical value: **3–5 ms**.

Purpose:
- Suppress relay chatter
- Suppress analog noise near threshold
- Prevent spurious Note On/Off bursts

This introduces a **bounded, deterministic latency** that is musically acceptable and perceptually stable.

---

## Scan Loop Timing

A typical play-mode loop iteration consists of:

1. Sensor scan (10 digital reads)
2. Debounce evaluation
3. XOR against previous state
4. MIDI event emission (if required)
5. Optional display update check (no I²C unless timer elapsed)

Worst-case loop time (no MIDI, no LCD update):
- << 1 ms

---

## MIDI Transmission Latency

MIDI runs at **31,250 bits/s**.

| Item | Bytes | Time |
|----|------|------|
| One byte | 1 | ~320 µs |
| Note On / Note Off | 3 | ~960 µs |

If multiple strings change state simultaneously:
- Messages are sent sequentially
- Ordering is deterministic within a single loop iteration

This latency dominates **after** sensor detection but is inherent to the MIDI protocol.

---

## Event Ordering and Determinism

The system intentionally:
- Samples all inputs first
- Computes all state changes
- Emits MIDI events as a batch

This ensures:
- No scan-order artefacts
- Simultaneous string breaks are treated as simultaneous within a loop tick
- Behaviour is repeatable across runs

Immediate-on-scan MIDI emission was rejected as it provides no meaningful latency benefit.

---

## Display (LCD) Timing

- LCD communicates over I²C
- Character LCD updates are slow and library calls may block

Mitigation:
- LCD updates occur at a fixed low rate (e.g. 10–20 Hz)
- No LCD operations occur in the critical scan path

Result:
- UI cannot introduce jitter into sensor scanning or MIDI output

---

## Kill Switch Timing

The footswitch is checked at the top of the loop.

On activation:
- All active notes are immediately sent Note Off
- Optional MIDI CC 123 (All Notes Off) may be sent
- Normal scanning is suspended while engaged

Latency is limited only by:
- loop execution time
- MIDI transmission time

---

## Summary of Latency Contributors

| Stage | Typical Contribution |
|-----|----------------------|
| Optical + analog front-end | 0.1–2 ms |
| Relay mechanics | 0.5–3 ms |
| Software debounce | 3–5 ms |
| MIDI transmission | ~1 ms per note |

Total end-to-end latency is **stable, bounded, and dominated by hardware**, not firmware.

---

## Design Trade-offs (Explicit)

Chosen:
- Deterministic timing
- Noise immunity
- Simple, auditable logic

Rejected:
- Interrupt-driven sensor capture
- SoftwareSerial MIDI
- Continuous LCD updates during play

These trade-offs reflect the musical and physical realities of a laser harp instrument.

---
