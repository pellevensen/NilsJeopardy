#include <Arduino.h>
#include <TM1638plus.h>
#include "volume2.h"
#include "random.h"
#include "common.h"

static const uint8_t STROBE_PIN = A1;  // strobe = GPIO connected to strobe line of module
static const uint8_t CLOCK_PIN = A2;   // clock = GPIO connected to clock line of module
static const uint8_t DIO_PIN = A0;     // data = GPIO connected to data line of module
static const uint8_t LAMP_PINS[] = { 3, 12, 10, 8 };
static const uint8_t GREEN_BUTTON_PIN = 4;
static const uint8_t RED_BUTTON_PIN = 2;
static const uint8_t PLAYER_BUTTON_PINS[] = { 9, 11, 5, 7 };
static const uint8_t AUDIO_PIN = 6;

#define CONFIRM_BUTTON (1 << 7)
#define DECREASE_BUTTON (1 << 0)
#define INCREASE_BUTTON (1 << 1)

#define STEP_DELAY 100000

static char GAME_NAMES[][10] = { "JPRDY", "TIMEBAND" };

static TM1638plus tm(STROBE_PIN, CLOCK_PIN, DIO_PIN, false);
static Volume vol;

void initIO() {
  tm.displayBegin();
  vol.noTone();
  for (uint8_t i = 0; i < sizeof(LAMP_PINS); i++) {
    pinMode(LAMP_PINS[i], OUTPUT);
  }
  pinMode(AUDIO_PIN, OUTPUT);

  for (uint8_t i = 0; i < sizeof(PLAYER_BUTTON_PINS); i++) {
    pinMode(PLAYER_BUTTON_PINS[i], INPUT);
  }
  pinMode(GREEN_BUTTON_PIN, INPUT);
  pinMode(RED_BUTTON_PIN, INPUT);
}

uint8_t readTM1638Buttons() {
  return tm.readButtons();
}

void lamp(uint8_t lampIdx, bool on) {
  digitalWrite(LAMP_PINS[lampIdx], !on);
}

uint8_t readPlayerButton(uint8_t playerIdx) {
  return digitalRead(PLAYER_BUTTON_PINS[playerIdx]);
}

void lightsOut() {
  for (uint8_t i = 0; i < sizeof(LAMP_PINS); i++) {
    digitalWrite(LAMP_PINS[i], 1);
  }
}

void flashWinner(int p, int blinks) {
  lightsOut();
  for (int i = 0; i < blinks; i++) {
    digitalWrite(LAMP_PINS[p], 0);
    delay(100);
    digitalWrite(LAMP_PINS[p], 1);
    delay(50);
  }
}

uint8_t readRedButton() {
  return digitalRead(RED_BUTTON_PIN);
}

uint8_t readGreenButton() {
  return digitalRead(GREEN_BUTTON_PIN);
}

uint16_t getUserValue(const char* text, uint16_t min, uint16_t max) {
  uint16_t incSpeed = 20;
  uint16_t val = min;
  uint8_t lastButtons = 255;
  uint32_t lastActionTimeStamp = 0;

  while (1) {
    uint8_t buttons = tm.readButtons();
    if (lastButtons != buttons || micros() - lastActionTimeStamp >= STEP_DELAY) {
      if (lastButtons == buttons) {
        incSpeed *= 1.05;
      } else {
        incSpeed = 20;
      }
      lastButtons = buttons;
      lastActionTimeStamp = micros();
      if (buttons & CONFIRM_BUTTON) {
        break;
      }
      if (buttons & INCREASE_BUTTON) {
        val += incSpeed / 20;
        val = val > max ? max : val;
      }
      if (buttons & DECREASE_BUTTON) {
        val -= incSpeed / 20;
        val = val < min ? min : val;
      }
      tm.displayIntNum(val, false, TMAlignTextRight);
    }
  }
  return val;
}

void displayBinary(uint8_t value) {
  for (uint8_t LEDposition = 0; LEDposition < 8; LEDposition++) {
    tm.setLED(LEDposition, value & 1);
    value = value >> 1;
  }
}

uint8_t checkDoubleClick() {
  static uint32_t lastClick = 0;
  static uint8_t lastState = 0;
  static uint32_t lastHandled = 0;

  if (millis() - lastHandled > 50) {
    lastHandled = millis();
    uint8_t redButton = readRedButton();
    if (lastState != redButton) {
      lastState = redButton;
      if (redButton == 0) {
        if (millis() - lastClick < 1000) {
          lastClick = 0;
          return 1;
        }
        lastClick = millis();
      }
    }
  }
  return 0;
}

void playWinSound() {
  int freqs[] = { 262, 330, 392, 494 };

  for (int octave = 0; octave < 3; octave++) {
    for (uint8_t notes = 0; notes < sizeof(freqs) / sizeof(int); notes++) {
      int f = freqs[notes] << octave;
      if (f < 4000) {}
      vol.tone(f, TRIANGLE, 255 - octave * 30);
      vol.delay(100);
    }
  }
  vol.noTone();
  delay(100);
}

void playLoseSound() {
  int freqs[] = { 700, 500 };
  uint32_t rs = 125;
  for (int octave = 0; octave < 5; octave++) {
    for (uint8_t notes = 0; notes < sizeof(freqs) / sizeof(int); notes++) {
      vol.tone(freqs[notes] >> (next32(&rs) & 3), SAWTOOTH, 255 - octave * 40);
      vol.delay(120);
      vol.noTone();
    }
  }
  vol.noTone();
  delay(100);
}

void displayNumber(uint32_t v) {
  tm.displayIntNum(v, true, TMAlignTextLeft);
}
