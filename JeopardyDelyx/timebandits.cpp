#include <Arduino.h>
#include <stdint.h>
#include "time.h"
#include "sounds.h"
#include "random.h"
#include "common.h"
#include "timebandits.h"

static uint32_t startTime;
static int32_t playerTimes[4];
static int32_t goal;
static uint16_t flashTimers[4];

static void countDown() {
  for (int i = 0; i < 4; i++) {
    lamp(i, true);
  }

  for (int i = 0; i < 4; i++) {
    playNote(0, 1000);
    lamp(3 - i, false);
  }
  uint32_t noteMillis = playNote(12, 1000);
  startTime = getTime() - noteMillis;
}

static uint8_t startGame() {
  while(!isTM1638ButtonPressed(BUT_BACK) && !readGreenButton()) {
    // Do nothing...
  }
  if(isTM1638ButtonPressed(BUT_BACK)) {
    return 0;
  }
  waitForGreenFlank();

  for (int i = 0; i < 4; i++) {
    playerTimes[i] = 0;
  }
  countDown();
  displayText("        ");
  return 1;
}

uint8_t initTimeBandits() {
  while (readTM1638Buttons() & 128)
    ;
  goal = getUserCursorValue("TID", 10, 5, 7200) * 1000;
  return startGame();
}

static void aliveIndicator() {
  if (0x3F == (getTime() & 0x3F)) {
    uint8_t leds = nextRandom();
    displayBinary(leds);
  }
}

static void flashRandomLights(uint16_t endPlayer, uint16_t min, uint16_t max, float growth) {
  for (uint32_t x = (uint32_t)min << 8; x < (uint32_t)max << 8; x *= growth) {
    endPlayer++;
  }
  for (uint32_t x = (uint32_t)min << 8; x < (uint32_t)max << 8; x *= growth) {
    lamp(endPlayer & 3, true);
    playNoteNonBlocking(x * 4 % 24, x >> 8);
    timeDelay(x >> 8);
    lamp(endPlayer & 3, false);
    endPlayer--;
  }
}

static int displayWinner() {
  uint32_t bestEstimate = 1UL << 29;
  int bestPlayer = 0;

  for (int i = 0; i < 4; i++) {
    uint32_t diff = abs(goal - playerTimes[i]);
    if (diff < bestEstimate) {
      bestPlayer = i;
      bestEstimate = diff;
    }
  }
  lightsOut();
  flashRandomLights(bestPlayer, 15, 200, 1.05);
  lamp(bestPlayer, true);
  char fs[10];
  dtostrf((goal - playerTimes[bestPlayer]) * 0.001, 9, 2, fs);
  displayText(fs);

  return bestPlayer;
}

static void flashPlayerLamp(int p) {
  flashTimers[p] = 0x2FF;
}

static void updateFlashes() {
  for (int i = 0; i < 4; i++) {
    if (flashTimers[i] != 0) {
      lamp(i, flashTimers[i] & (1UL << 5));
      flashTimers[i]--;
    }
  }
}

uint8_t doTimeBanditsLoop() {
  aliveIndicator();
  displayNumber(getTime() - startTime);
  updateFlashes();
  for (int i = 0; i < 4; i++) {
    if (playerTimes[i] == 0) {
      if (readPlayerButton(i)) {
        playerTimes[i] = getTime() - startTime;
        flashPlayerLamp(i);
      }
    }
  }
  if (checkDoubleClick()) {
    int winner = displayWinner();
    playPlayerSound(winner);
    playWinSound();
    while (!checkDoubleClick()) {
    }
    if(!startGame()) {
      return 1;
    }
  }

  if (isTM1638ButtonPressed(BUT_BACK)) {
    return 1;
  }

  return 0;
}
