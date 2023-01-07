#include <stdint.h>
#include <TM1638plus.h>
#include "volume2.h"
#include "random.h"
#include "jeopardy.h"
#include "timebandits.h"
#include "common.h"

// static const
#if 0
#define TM1638_IO
#else
#define ARCADE_IO
#endif

static GameType currentGame;

static GameType waitForStart() {
  int i = 0;
  GameType done = NONE_CHOSEN;
  while (!done) {
    if (readRedButton() || readGreenButton()) {
      done = JEOPARDY;
    }
    uint8_t buttons = readTM1638Buttons();
    if (buttons != 0) {
      done = TIME_BANDITS;
      int val = getUserValue("SEK", 10, 10000);
      Serial.println(val);
    }
    int k = i % 6;
    if (k >= 4) {
      k = 6 - k;
    }
    lamp(k, true);
    delay(100);
    lamp(k, false);
    displayBinary(1 << k);
    i++;
  }
  return done;
}

void setup() {
  Serial.begin(9600);
  initIO();

  playWinSound();
  currentGame = waitForStart();
  Serial.print("Game chosen: ");
  Serial.println(currentGame);
  switch (currentGame) {
    case JEOPARDY:
      initJeopardy();
      break;
    case TIME_BANDITS:
      initTimeBandits();
      break;
    case NONE_CHOSEN:
    default:
      Serial.println("Failed setup; no game type chosen; this should never happen.");
  }
  playWinSound();
  lightsOut();
}

void loop() {
  switch (currentGame) {
    case JEOPARDY:
      doJeopardyLoop();
      break;
    case TIME_BANDITS:
      doTimeBanditsLoop();
      break;
    case NONE_CHOSEN:
    default:
      Serial.println("No game type chosen; this should never happen.");
  }
}
