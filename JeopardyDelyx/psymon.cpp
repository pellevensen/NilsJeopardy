#include <Arduino.h>
#include <stdint.h>
#include "sounds.h"
#include "random.h"
#include "common.h"
#include "psymon.h"
#include "time.h"

typedef enum {
  PSYMON_PLAY = 0,
  PSYMON_FOLLOW = 1,
  PSYMON_GAME_OVER = 2,
} PsymonPhase;

static uint16_t timeOut;
static uint32_t lastButtonPress;
static int32_t seed;
static uint16_t baseSpeed;
static uint16_t speed;
static uint8_t speedUp;
static uint8_t buttons;
static uint16_t length;
static uint16_t position;
static PsymonPhase phase;
static uint32_t rng;

#define NOTE1 4
#define NOTE2 13
#define NOTE3 9
#define NOTE4 16

static const uint8_t NOTES[] = { NOTE1, NOTE2, NOTE3, NOTE4 };

static uint8_t startGame() {
  while (!isTM1638ButtonPressed(BUT_BACK) && !readGreenButton()) {
    // Do nothing...
  }
  if (isTM1638ButtonPressed(BUT_BACK)) {
    return 0;
  }
  waitForGreenFlank();
  displayText("        ");
  seed = nextRandom();
  Serial.print("Seed: ");
  Serial.println(seed);
  length = 1;
  phase = PSYMON_PLAY;
  rng = seed;
  speed = baseSpeed;
  position = 0;
  return 1;
}

#define MAX_SPEED 5
#define MIN_SPEED 1
#define MIN_DELAY 50

uint8_t initPsymon() {
  while (readTM1638Buttons() & 128)
    ;
  speed = getUserCursorValue("Speed", (MAX_SPEED + MIN_SPEED) / 2, MIN_SPEED, MAX_SPEED) + 1;
  baseSpeed = 4000 / (speed * speed);
  buttons = getUserCursorValue("Btns", 4, 2, 4);
  timeOut = getUserCursorValue("TmOut", 2, 1, 10);
  speedUp = getUserCursorValue("Faster", 0, 0, 10) * 2;
  speed = baseSpeed;
  return startGame();
}

static void adjustSpeed() {
  speed -= speedUp;
  speed = max(speed, MIN_DELAY);
}

static uint16_t soundDuration(int speed) {
  return speed;
}

static void flashAndSound(uint8_t button) {
  lamp(button, true);
  playNote(NOTES[button], soundDuration(speed));
  lamp(button, false);
}

static void playerFlashAndSound(uint8_t button) {
  lamp(button, true);
  playNoteNonBlocking(NOTES[button], soundDuration(speed));
}

static void playNotes() {
  for (uint32_t i = 0; i < length; i++) {
    uint8_t note = next32(&rng) % buttons;
    flashAndSound(note);
  }
}

static void gameOver() {
  playLoseSound();
  phase = PSYMON_GAME_OVER;
}

uint8_t doPsymonLoop() {
  static uint32_t lastNoteTimestamp = 0;
  if (getTime() > lastNoteTimestamp + 1000) {
    noteOff();
  }
  if (phase == PSYMON_PLAY) {
    playNotes();
    rng = seed;
    lastButtonPress = getTime();
    phase = PSYMON_FOLLOW;
    playNote(0, 0);
  } else if (phase == PSYMON_FOLLOW) {
    uint8_t buttonsPressed = readPlayerButtons();

    for (uint8_t i = 0; i < 4; i++) {
      if (buttonsPressed & (1 << i)) {
        int buttonDown = i;
        Serial.print("Button down: ");
        Serial.println(getTime());
        playerFlashAndSound(buttonDown);

        while (readPlayerButton(buttonDown)) {
          // Do nothing / wait for release.
          soundDelay();
        }
        Serial.print("Button up: ");
        Serial.println(getTime());

        uint32_t startTime = getTime();
        while (readPlayerButtons() == 0) {
          if (getTime() - startTime > soundDuration(speed)) {
            break;
          }
        }
        noteOff();
        lastButtonPress = getTime();

        lamp(buttonDown, false);
        uint8_t note = next32(&rng) % buttons;
        if (note != buttonDown) {
          gameOver();
          rng = seed;
          playNotes();
          break;
        }
        position++;
        if (position >= length) {
          playWinSound();
          adjustSpeed();
          displayNumber(length);
          delay(500);
          phase = PSYMON_PLAY;
          rng = seed;
          length++;
          position = 0;
        }
      }
    }

    if (getTime() - lastButtonPress > timeOut * 1000) {
      gameOver();
    }
  } else if (phase == PSYMON_GAME_OVER) {
    if (!startGame()) {
      return 1;
    }
  }

  return 0;
}
