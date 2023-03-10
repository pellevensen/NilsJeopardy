#include <Arduino.h>
#include "common.h"
#include "sounds.h"
#include "numbum.h"

typedef enum {
  NB_MISMATCH = 0,
  NB_SINGLE = 1,
  NB_DOUBLE = 2
} NumBumMode;

static uint32_t rngState;
static NumBumMode mode;

static NumBumMode idxToMode(int8_t idx) {
  NumBumMode modes[] = {NB_MISMATCH, NB_SINGLE, NB_DOUBLE};
  return modes[idx];
}

InitStatus initNumBum() {
  const char* modeNames[] = { "MISMATCH", "SINGLE", "DOUBLE" };
  int modeIdx = selectString(modeNames, 3);
  mode = idxToMode(modeIdx);
  if (modeIdx < 0) {
    return INIT_CANCEL;
  }
  // goal = getUserCursorValue("TID", 10, 5, 7200) * 1000;
  //players = getUserCursorValue("Spelare", 4, 2, 4);

  return INIT_OK;
  //return startGame();
}

LoopStatus doNumBumLoop() {
  for (int i = 0; i < 4; i++) {
    playStandardNote(i, 1000);
  }
  Serial.println("numBumLoop");
#if 0
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
#endif
  if (readTM1638Buttons() & BUT_BACK) {
    return LOOP_CANCELED;
  }
  return LOOP_RUNNING;
}
