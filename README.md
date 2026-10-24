# Music Christmas Tree Lights

An Arduino project that plays Christmas carols on a buzzer while two strings of tree lights flash in time with the music, all controlled from an IR remote.

## Features

- Three built-in carols: **Carol of the Bells**, **O Christmas Tree** and **Joy to the World**
- Two light strings, switched by relays, alternate on every note of the song
- IR remote control: choose a song, turn the lights steadily on, or switch everything off
- Mute / unmute the buzzer while keeping the light show running
- Songs loop until another command is received; a new command interrupts the current song immediately after the note that is playing
- Every received IR code is printed on the Serial Monitor (9600 baud), which makes it easy to map a different remote

## Hardware

| Component | Notes |
|---|---|
| AVR-based Arduino board (e.g. Arduino Uno) | |
| IR receiver module + IR remote | |
| Passive buzzer | driven with `tone()` |
| 2-channel relay module | active LOW, one channel per light string |
| 2 light strings | switched by the relays |

### Pin mapping

| Arduino pin | Connected to |
|---|---|
| D4 | Buzzer |
| D5 | IR receiver signal |
| D7 | Relay 1 (light string 1) |
| D8 | Relay 2 (light string 2) |

## Remote control

| Raw IR code | Action |
|---|---|
| `0xBA45FF00` | Play Carol of the Bells |
| `0xB946FF00` | Play O Christmas Tree |
| `0xB847FF00` | Play Joy to the World |
| `0xF20DFF00` | Lights on (steady, no music) |
| `0xE916FF00` | Everything off |
| `0xAD52FF00` | Mute the buzzer |
| `0xE718FF00` | Unmute the buzzer |

To use another remote, open the Serial Monitor, press its buttons, and replace the `IR_CODE_*` constants in the sketch with the codes that are printed.

## How it works

- Each melody is stored as pairs of `{note frequency, duration}`, where the duration is a divider of a whole note (4 = quarter note, 8 = eighth note, negative = dotted note). The whole-note length is derived from the song's tempo in BPM.
- `playMelody()` walks through the notes, plays each one with `tone()` for 90% of its length (leaving a short gap), and toggles the two relays so the light strings alternate on every note.
- After each note the sketch polls the IR receiver, so a button press switches songs or modes without waiting for the song to finish.
- The IR receiver runs on Timer1 (`IR_USE_AVR_TIMER1`) so that it does not clash with `tone()`, which uses Timer2.
- Note frequencies live in `pitches.h`.

## Getting started

1. Install the [Arduino IDE](https://www.arduino.cc/en/software).
2. Install the **IRremote** library (v3.x or newer) from *Tools > Manage Libraries...*.
3. Wire the components according to the pin mapping above.
4. Open `MusicChristmasTreeLights/MusicChristmasTreeLights.ino`, select your board and port, and click **Upload**.

Or with [arduino-cli](https://arduino.github.io/arduino-cli/):

```bash
arduino-cli core install arduino:avr
arduino-cli lib install IRremote
arduino-cli compile --fqbn arduino:avr:uno MusicChristmasTreeLights
arduino-cli upload -p <PORT> --fqbn arduino:avr:uno MusicChristmasTreeLights
```

## Project structure

```
MusicChristmasTreeLights/
├── MusicChristmasTreeLights.ino   # main sketch: IR handling, modes, melodies
└── pitches.h                      # note frequency definitions
```

## Author

George-Armand Ursei - [GitHub](https://github.com/armandursei)
