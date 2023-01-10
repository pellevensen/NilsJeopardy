#include <Arduino.h>
#include <stdint.h>
#include "sounds.h"
#include "random.h"
#include "common.h"
#include "timebandits.h"

static uint32_t startTime;
static int32_t playerTimes[4];
static int32_t goal;

void countDown() {
  for (int i = 0; i < 4; i++) {
    lamp(i, true);
  }

  for (int i = 0; i < 4; i++) {
    playNote(0, 1000);
    lamp(3 - i, false);
  }
  uint32_t noteMillis = playNote(12, 1000);
  startTime = millis() - noteMillis;
}

static void startGame() {
  waitForGreenFlank();
  for(int i = 0; i < 4; i++) {
    playerTimes[i] = 0;
  }
  countDown();
  displayText("        ");
}

void initTimeBandits() {
  while (readTM1638Buttons() & 128)
    ;
  goal = getUserCursorValue("TID", 10, 7200) * 1000;
  startGame();
}

static void aliveIndicator() {
  if (0xF == (millis() & 0xF)) {
    uint8_t leds = nextRandom();
    displayBinary(leds);
  }
}

static void flashRandomLights(uint16_t endPlayer, uint16_t min, uint16_t max, float growth) {
    for(uint32_t x = (uint32_t) min << 8; x < (uint32_t) max << 8; x *= growth) {
        endPlayer++;
    }
    for(uint32_t x = (uint32_t) min << 8; x < (uint32_t) max << 8; x *= growth) {
      lamp(endPlayer & 3, true);
      delay(x >> 8);
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
  for (int d = 30; d < 150; d *= 1.2) {
    lamp(p, true);
    delay(d);
    lamp(p, false);
    delay(d);
  }
}

void doTimeBanditsLoop() {
  aliveIndicator();
  displayNumber(millis() - startTime);
  for (int i = 0; i < 4; i++) {
    if (playerTimes[i] == 0) {
      if (readPlayerButton(i)) {
        playerTimes[i] = millis() - startTime;
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
    startGame();
  }
}
