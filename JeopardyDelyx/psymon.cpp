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

static int16_t timeOut;
static uint32_t lastButtonPress;
static int32_t seed;
static int16_t baseSpeed;
static uint16_t speed;
static int8_t speedUp;
static int8_t buttons;
static uint16_t length;
static uint16_t position;
static PsymonPhase phase;
static uint32_t rng;

static InitStatus startGame() {
  while (!isTM1638ButtonPressed(BUT_BACK) && !readGreenButton()) {
    // Do nothing...
  }
  if (isTM1638ButtonPressed(BUT_BACK)) {
    return INIT_CANCEL;
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
  return INIT_OK;
}

#define MAX_SPEED 5
#define MIN_SPEED 1
#define MIN_DELAY 50

InitStatus initPsymon() {
  int16_t tmpSpeed;
  if ((tmpSpeed = getUserCursorValue("Speed", (MAX_SPEED + MIN_SPEED) / 2, MIN_SPEED, MAX_SPEED)) < 0 || (buttons = getUserCursorValue("Btns", 4, 2, 4)) < 0 || (timeOut = getUserCursorValue("TmOut", 2, 1, 10)) < 0 || (speedUp = getUserCursorValue("Faster", 0, 0, 10) * 2) < 0) {
    return INIT_CANCEL;
  }
  tmpSpeed += 1;
  baseSpeed = 4000 / (tmpSpeed * tmpSpeed);
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

void flashAndSound(uint8_t button) {
  lamp(button, true);
  playStandardNote(button, soundDuration(speed));
  lamp(button, false);
}

static void playerFlashAndSound(uint8_t button) {
  lamp(button, true);
  playStandardNoteNonBlocking(button, soundDuration(speed));
}

static void playNotes() {
  for (uint32_t i = 0; i < length; i++) {
    uint8_t note = next32(&rng) % buttons;
    flashAndSound(note);
  }
}

static void gameOver() {
  rng = seed;
  playLoseSound();
  playNotes();
  phase = PSYMON_GAME_OVER;
}

LoopStatus doPsymonLoop() {
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
        lastButtonPress = getTime();
        Serial.print("lastButtonPress: ");
        Serial.println(lastButtonPress);
        playerFlashAndSound(buttonDown);

        while (readPlayerButton(buttonDown)) {
          // Do nothing / wait for release.
          soundDelay();
        }

        uint32_t startTime = getTime();
        while (readPlayerButtons() == 0) {
          if (getTime() - startTime > soundDuration(speed)) {
            break;
          }
        }
        noteOff();

        lamp(buttonDown, false);
        uint8_t note = next32(&rng) % buttons;
        if (note != buttonDown) {
          gameOver();
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

    if (phase == PSYMON_FOLLOW && getTime() - lastButtonPress > timeOut * 1000) {
      gameOver();
    }
  } else if (phase == PSYMON_GAME_OVER) {
    if (startGame() == INIT_CANCEL) {
      return LOOP_CANCELED;
    }
  }

  return LOOP_RUNNING;
}
