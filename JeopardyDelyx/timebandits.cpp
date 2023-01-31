#include "math.h"
#include <Arduino.h>
#include <stdint.h>
#include "time.h"
#include "sounds.h"
#include "random.h"
#include "common.h"
#include "timebandits.h"

static uint32_t startTime;
static uint8_t players;
static int32_t playerTimes[4];
static int32_t goal;
static uint16_t flashTimers[4];
static uint8_t completed;

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

static uint8_t isControlPressed() {
  return isTM1638ButtonPressed(BUT_BACK) || readGreenButton();
}

static void updateCompletedDisplay() {
  displayBinary(completed);
}

static uint8_t startGame() {
  while (!isControlPressed()) {
    // Do nothing...
  }
  if (isTM1638ButtonPressed(BUT_BACK)) {
    return 0;
  }
  waitForGreenFlank();

  for (int i = 0; i < 4; i++) {
    playerTimes[i] = 0;
  }

  completed = ~((1 << (players * 2)) - 1);
  Serial.println(completed);
  updateCompletedDisplay();

  countDown();
  displayText("        ");

  return 1;
}

uint8_t initTimeBandits() {
  goal = getUserCursorValue("TID", 10, 5, 7200) * 1000;
  players = getUserCursorValue("Spelare", 4, 2, 4);

  return startGame();
}

static void flashRandomLights(uint16_t endPlayer, uint16_t min, uint16_t max, float growth) {
  for (uint32_t x = (uint32_t)min << 8; x < (uint32_t)max << 8; x *= growth) {
    endPlayer++;
  }
  for (uint32_t x = (uint32_t)min << 8; x < (uint32_t)max << 8; x *= growth) {
    lamp(endPlayer % players, true);
    playNoteNonBlocking(x * players % 24, x >> 8);
    timeDelay(x >> 8);
    lamp(endPlayer % players, false);
    endPlayer--;
  }
}

static void displayTime(uint8_t playerIdx, int32_t t) {
  char text[9] = "        ";
  int decimals = max(0, 3 - log10((uint16_t)(abs(t) >> 16)));
  dtostrf(t * 0.001, 9, decimals, text);
  text[0] = '1' + playerIdx;
  displayText(text);
}

static int displayWinner() {
  uint32_t bestEstimate = 1UL << 29;
  int bestPlayer = 0;

  for (int i = 0; i < players; i++) {
    uint32_t diff = abs(goal - playerTimes[i]);
    if (diff < bestEstimate) {
      bestPlayer = i;
      bestEstimate = diff;
    }
  }
  lightsOut();
  flashRandomLights(bestPlayer, 15, 200, 1.05);
  lamp(bestPlayer, true);

  return bestPlayer;
}

static void flashPlayerLamp(int p) {
  flashTimers[p] = 0x2FF;
}

static void updateFlashes() {
  for (int i = 0; i < players; i++) {
    if (flashTimers[i] != 0) {
      lamp(i, flashTimers[i] & (1UL << 5));
      flashTimers[i]--;
    }
  }
}

typedef struct {
  uint8_t playerIdx;
  int32_t score;
} PlayerScore;

static void sortPlayerScores(PlayerScore scores[]) {
  for (int i = 1; i < players; i++) {
    int j = i;
    PlayerScore toInsert;
    memcpy(&toInsert, &scores[j], sizeof(PlayerScore));
    while (j > 0 && abs(toInsert.score) < abs(scores[j - 1].score)) {
      memcpy(&scores[j], &scores[j - 1], sizeof(PlayerScore));
      j--;
    }
    memcpy(&scores[j], &toInsert, sizeof(PlayerScore));
  }
}

static void initPlayerScores(int32_t playerTimes[], PlayerScore* scores) {
  for (int i = 0; i < players; i++) {
    scores[i].playerIdx = i;
    scores[i].score = goal - playerTimes[i];
  }
  sortPlayerScores(scores);
}

uint8_t doTimeBanditsLoop() {
  displayNumber(getTime() - startTime);
  updateFlashes();
  for (int i = 0; i < players; i++) {
    if (playerTimes[i] == 0) {
      if (readPlayerButton(i)) {
        playerTimes[i] = getTime() - startTime;
        flashPlayerLamp(i);
        completed |= 3 << (2 * i);
        updateCompletedDisplay();
      }
    }
  }
  if (checkDoubleClick()) {
    int winner = displayWinner();
    playPlayerSound(winner);
    playWinSound();

    PlayerScore scores[4];
    initPlayerScores(playerTimes, scores);
    uint8_t oldPlayerIdx = 0xFF;
    uint8_t playerIdx = 0;

    while (!isControlPressed()) {
      if (toggled(BUT_DOWN)) {
        playerIdx = (playerIdx + players - 1) % players;
      } else if (toggled(BUT_UP)) {
        playerIdx = (playerIdx + 1) % players;
      }
      if (oldPlayerIdx != playerIdx) {
        displayTime(scores[playerIdx].playerIdx, scores[playerIdx].score);
        displayLEDScore(playerIdx);
        oldPlayerIdx = playerIdx;
      }
      timeDelay(20);
    }

    if (!startGame()) {
      return 1;
    }
  }

  if (isTM1638ButtonPressed(BUT_BACK)) {
    waitForTM1638Flank();
    return 1;
  }

  return 0;
}
