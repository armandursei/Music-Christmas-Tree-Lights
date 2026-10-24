/**
 * Music Christmas Tree Lights
 *
 * Plays Christmas carols on a buzzer while two relay-controlled light strings
 * flash in time with the music. Everything is controlled with an IR remote:
 * pick one of three songs, turn the lights steadily on or off, or mute the
 * buzzer and keep only the light show.
 *
 * Hardware:
 *   - AVR-based Arduino board (e.g. Arduino Uno)
 *   - IR receiver module + IR remote
 *   - Passive buzzer
 *   - 2-channel relay module (active LOW), one channel per light string
 *
 * Pin map:
 *   D4  Buzzer
 *   D5  IR receiver signal
 *   D7  Relay 1 (light string 1)
 *   D8  Relay 2 (light string 2)
 *
 * Library: IRremote (v3.x or newer, uses the IrReceiver API).
 */

// Run the IR receiver on Timer1 so it does not conflict with tone(), which uses Timer2.
#define IR_USE_AVR_TIMER1
#include <IRremote.h>

#include "pitches.h"

const int IR_RECEIVER_PIN = 5;
#define BUZZER_PIN 4
const int RELAY1_PIN = 7;
const int RELAY2_PIN = 8;

// The relay module is active LOW: LOW switches a light string on, HIGH switches it off.

// Constructing this object sets the receive pin used by the global IrReceiver instance.
IRrecv irrecv(IR_RECEIVER_PIN);

// Operating modes (values of `currentMode`).
const int MODE_OFF = 0;                   // both light strings off, no music
const int MODE_LIGHTS_ON = -1;            // both light strings steadily on, no music
const int MODE_CAROL_OF_THE_BELLS = 1;
const int MODE_O_CHRISTMAS_TREE = 2;
const int MODE_JOY_TO_THE_WORLD = 3;

// Raw IR codes of the remote buttons, as printed on the Serial Monitor.
const uint32_t IR_CODE_CAROL_OF_THE_BELLS = 0xBA45FF00;
const uint32_t IR_CODE_O_CHRISTMAS_TREE = 0xB946FF00;
const uint32_t IR_CODE_JOY_TO_THE_WORLD = 0xB847FF00;
const uint32_t IR_CODE_OFF = 0xE916FF00;
const uint32_t IR_CODE_LIGHTS_ON = 0xF20DFF00;
const uint32_t IR_CODE_MUTE = 0xAD52FF00;    // "#" key on the original remote
const uint32_t IR_CODE_UNMUTE = 0xE718FF00;

volatile int currentMode = MODE_OFF;
volatile int soundOn = 1;

void setup() {
  Serial.begin(9600);

  IrReceiver.enableIRIn();
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  // Start with both light strings off.
  digitalWrite(RELAY1_PIN, HIGH);
  digitalWrite(RELAY2_PIN, HIGH);
}

/**
 * Plays a melody once while alternating the two light strings on every note.
 *
 * melody     pairs of {note frequency, duration divider}; a divider of 4 is a
 *            quarter note, 8 an eighth note, and a negative value is a dotted
 *            note (1.5x as long)
 * length     number of elements in `melody` (twice the number of notes)
 * songTempo  tempo in beats per minute (uint8_t, so 0-255)
 * mode       the mode this melody belongs to; playback stops as soon as an IR
 *            command switches to a different mode
 */
void playMelody(const int16_t *melody, uint16_t length, uint8_t songTempo, int mode) {
  uint16_t notes = length / 2;
  uint16_t wholenote = (60000 * 4) / songTempo;
  int8_t divider;
  uint16_t noteDuration;
  for (uint16_t i = 0; i < notes * 2; i += 2) {
    // Alternate the light strings on every note.
    if (i % 4 == 0) {
      digitalWrite(RELAY1_PIN, HIGH);
      digitalWrite(RELAY2_PIN, LOW);
    } else {
      digitalWrite(RELAY1_PIN, LOW);
      digitalWrite(RELAY2_PIN, HIGH);
    }
    divider = melody[i + 1];
    if (divider > 0) {
      noteDuration = wholenote / divider;
    } else {
      noteDuration = wholenote / abs(divider) * 1.5;
    }
    // Play the note for 90% of its duration to leave a short gap between notes.
    if (soundOn == 1) {
      tone(BUZZER_PIN, melody[i], noteDuration * 0.9);
    }
    delay(noteDuration);
    checkIrCommand();
    if (currentMode != mode) break;
  }
}

void loop() {
  checkIrCommand();

  // While a song mode is active, loop() keeps calling playMelody(), so the song repeats.
  if (currentMode == MODE_CAROL_OF_THE_BELLS) {
    int16_t carolOfTheBells[]{
      NOTE_AS4, 4, NOTE_A4, 8, NOTE_AS4, 8, NOTE_G4, 4, NOTE_AS4, 4, NOTE_A4, 8, NOTE_AS4, 8, NOTE_G4, 4, NOTE_AS4, 4, NOTE_A4, 8, NOTE_AS4, 8, NOTE_G4, 4, NOTE_AS4, 4, NOTE_A4, 8, NOTE_AS4, 8, NOTE_G4, 4, NOTE_AS4, 4, NOTE_A4, 8, NOTE_AS4, 8, NOTE_G4, 4, NOTE_AS4, 4, NOTE_A4, 8, NOTE_AS4, 8, NOTE_G4, 4,
      NOTE_D5, 4, NOTE_C5, 8, NOTE_D5, 8, NOTE_AS4, 4, NOTE_D5, 4, NOTE_C5, 8, NOTE_D5, 8, NOTE_AS4, 4, NOTE_D5, 4, NOTE_C5, 8, NOTE_D5, 8, NOTE_AS4, 4, NOTE_D5, 4, NOTE_C5, 8, NOTE_D5, 8, NOTE_AS4, 4, NOTE_G5, 4, NOTE_G5, 8, NOTE_G5, 8, NOTE_F5, 8, NOTE_DS5, 8, NOTE_D5, 4, NOTE_D5, 8, NOTE_D5, 8, NOTE_C5, 8, NOTE_AS4, 8,
      NOTE_C5, 4, NOTE_C5, 8, NOTE_C5, 8, NOTE_D5, 8, NOTE_C5, 8, NOTE_AS4, 4, NOTE_A4, 8, NOTE_AS4, 8, NOTE_G4, 4, NOTE_D4, 8, NOTE_E4, 8, NOTE_FS4, 8, NOTE_G4, 8, NOTE_A4, 8, NOTE_AS4, 8, NOTE_C5, 8, NOTE_D5, 8, NOTE_C5, 4, NOTE_AS4, 4, NOTE_D5, 8, NOTE_D5, 8, NOTE_FS5, 8, NOTE_G5, 8, NOTE_A5, 8, NOTE_AS5, 8,
      NOTE_C6, 8, NOTE_D6, 8, NOTE_C6, 4, NOTE_AS5, 4, NOTE_AS5, 4, NOTE_A5, 8, NOTE_AS5, 8, NOTE_G5, 4, NOTE_AS5, 4, NOTE_A5, 8, NOTE_AS5, 8, NOTE_G5, 4, NOTE_AS5, 4, NOTE_A5, 8, NOTE_AS5, 8, NOTE_G5, 4, NOTE_AS5, 4, NOTE_A5, 8, NOTE_AS5, 8, NOTE_G5, 4,
      NOTE_AS5, -4, NOTE_A5, -8, NOTE_AS5, -8, NOTE_G5, 2
    };
    playMelody(carolOfTheBells, sizeof(carolOfTheBells) / sizeof(carolOfTheBells[0]), 135, MODE_CAROL_OF_THE_BELLS);
  }

  else if (currentMode == MODE_O_CHRISTMAS_TREE) {
    int16_t oChristmasTree[]{
      NOTE_D4, 4, NOTE_G4, -8, NOTE_G4, 16, NOTE_G4, 4, NOTE_A4, 4, NOTE_B4, -8, NOTE_B4, 16, NOTE_B4, -4, NOTE_B4, 8, NOTE_A4, 8, NOTE_B4, 8, NOTE_C5, 4, NOTE_F4, 4, NOTE_A4, 4, NOTE_G4, 4, NOTE_D4, 2, REST, 16,
      NOTE_G4, -8, NOTE_G4, 16, NOTE_G4, 4, NOTE_A4, 4, NOTE_B4, -8, NOTE_B4, 16, NOTE_B4, -4, NOTE_B4, 8, NOTE_A4, 8, NOTE_B4, 8, NOTE_C5, 4, NOTE_F4, 4, NOTE_A4, 4, NOTE_G4, -4, REST, 4, NOTE_D5, 8,
      NOTE_D5, 8, NOTE_B4, 8, NOTE_E5, -4, NOTE_D5, 8, NOTE_D5, 8, NOTE_C5, 8, NOTE_C5, -4, NOTE_C5, 8, NOTE_C5, 8, NOTE_A4, 8, NOTE_D5, -4, NOTE_C5, 8, NOTE_C5, 8, NOTE_B4, 8, NOTE_B4, 4, NOTE_D4, 4,
      NOTE_G4, -8, NOTE_G4, 16, NOTE_G4, 4, NOTE_A4, 4, NOTE_B4, -8, NOTE_B4, 16, NOTE_B4, -4, NOTE_B4, 8, NOTE_A4, 8, NOTE_B4, 8, NOTE_C5, 4, NOTE_F4, 4, NOTE_A4, 4, NOTE_G4, 2
    };
    playMelody(oChristmasTree, sizeof(oChristmasTree) / sizeof(oChristmasTree[0]), 115, MODE_O_CHRISTMAS_TREE);
  }

  else if (currentMode == MODE_JOY_TO_THE_WORLD) {
    int16_t joyToTheWorld[]{
      NOTE_F5, 2, NOTE_E5, -4, NOTE_D5, 8, NOTE_C5, -2, NOTE_AS4, 4, NOTE_A4, 2, NOTE_G4, 2, NOTE_F4, -2,
      NOTE_C5, 4, NOTE_D5, -2, NOTE_D5, 4, NOTE_E5, -2, NOTE_E5, 4, NOTE_F5, 1,
      NOTE_F5, 4, NOTE_F5, 4, NOTE_E5, 4, NOTE_D5, 4, NOTE_C5, 4, NOTE_C5, -4, NOTE_AS4, 8, NOTE_A4, 4, NOTE_F5, 4, NOTE_F5, 4, NOTE_E5, 4, NOTE_D5, 4,
      NOTE_C5, 4, NOTE_C5, -4, NOTE_AS4, 8, NOTE_A4, 4, NOTE_A4, 4, NOTE_A4, 4, NOTE_A4, 4, NOTE_A4, 4, NOTE_A4, 8, NOTE_AS4, 8, NOTE_C5, -2,
      NOTE_AS4, 8, NOTE_A4, 8, NOTE_G4, 4, NOTE_G4, 4, NOTE_G4, 4, NOTE_G4, 8, NOTE_A4, 8, NOTE_AS4, -2,
      NOTE_A4, 8, NOTE_G4, 8, NOTE_F4, 4, NOTE_F5, 2, NOTE_D5, 4, NOTE_C5, -4, NOTE_AS4, 8, NOTE_A4, 4, NOTE_C5, 4, NOTE_E4, 2, NOTE_D4, 2, NOTE_C4, 1
    };
    playMelody(joyToTheWorld, sizeof(joyToTheWorld) / sizeof(joyToTheWorld[0]), 160, MODE_JOY_TO_THE_WORLD);
  }

  else if (currentMode == MODE_LIGHTS_ON) {
    digitalWrite(RELAY1_PIN, LOW);
    digitalWrite(RELAY2_PIN, LOW);
  }

  else if (currentMode == MODE_OFF) {
    digitalWrite(RELAY1_PIN, HIGH);
    digitalWrite(RELAY2_PIN, HIGH);
  }
}

/**
 * Reads a pending IR code (if any), prints it on the serial port and updates
 * the mode or the mute flag accordingly. Unknown codes are ignored.
 */
void checkIrCommand() {
  if (IrReceiver.decode()) {
    Serial.print("IR code received: ");
    Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);

    if (IrReceiver.decodedIRData.decodedRawData == IR_CODE_CAROL_OF_THE_BELLS) { currentMode = MODE_CAROL_OF_THE_BELLS; }
    else if (IrReceiver.decodedIRData.decodedRawData == IR_CODE_O_CHRISTMAS_TREE) { currentMode = MODE_O_CHRISTMAS_TREE; }
    else if (IrReceiver.decodedIRData.decodedRawData == IR_CODE_JOY_TO_THE_WORLD) { currentMode = MODE_JOY_TO_THE_WORLD; }
    else if (IrReceiver.decodedIRData.decodedRawData == IR_CODE_OFF) { currentMode = MODE_OFF; }
    else if (IrReceiver.decodedIRData.decodedRawData == IR_CODE_LIGHTS_ON) { currentMode = MODE_LIGHTS_ON; }
    else if (IrReceiver.decodedIRData.decodedRawData == IR_CODE_MUTE) { soundOn = 0; }
    else if (IrReceiver.decodedIRData.decodedRawData == IR_CODE_UNMUTE) { soundOn = 1; }

    Serial.print("mode: ");
    Serial.println(currentMode);
    IrReceiver.resume();
  }
}
