#include <stdint.h>
#include <TM1638plus.h>
#include "random.h"
#include "jeopardy.h"
#include "timebandits.h"
#include "psymon.h"
#include "common.h"
#include "sounds.h"
#include "time.h"

// static const
#if 0
#define TM1638_IO
#else
#define ARCADE_IO
#endif

static GameType currentGame;

static GameType selectGame() {
  uint8_t gameIdx = 0;
  uint8_t buttons = 0;
  uint8_t games = getGames();
  while (!(readTM1638Buttons() & 128)) {
    displayText(getGameName(gameIdx));
    while (buttons == readTM1638Buttons()) {
      // Wait until some button pressed.
    }
    buttons = readTM1638Buttons();
    if (buttons & 1) {
      gameIdx = (gameIdx + games - 1) % games;
    } else if (buttons & 2) {
      gameIdx = (gameIdx + 1) % games;
    }
  }
  return getGame(gameIdx);
}

static uint8_t triangle(uint16_t v0, uint8_t cycle) {
  uint8_t rCycle = cycle * 2 - 2;
  uint8_t v = v0 % rCycle;
  if (v >= cycle) {
    v = rCycle - v;
  }
  return v;
}

static GameType waitForStart() {
  int i = 0;
  GameType game = NO_GAME;
  while (game == NO_GAME) {
    if (readRedButton() || readGreenButton()) {
      lightsOut();
      game = JEOPARDY;
    }
    uint8_t buttons = readTM1638Buttons();
    if (buttons != 0) {
      while (readTM1638Buttons() & 128) {
        // Do nothing.
      }
      delay(50);
      lightsOut();
      game = selectGame();
    }
    lamp(triangle(i, 4), true);
    displayBinary(1 << triangle(i, 8));
    delay(100);
    lamp(triangle(i, 4), false);
    i++;
  }
  return game;
}

void setup() {
  Serial.begin(9600);
  initIO();
  startClock();

  playBootSound();

#if 0
  delay(10000);
  uint32_t ic = getTime();
  Serial.print("getTime: ");
  Serial.println(ic);
#endif

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
    case PSYMON:
      initPsymon();
      break;
    default:
      Serial.println("Failed setup; no game type chosen; this should never happen.");
  }
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
    case PSYMON:
      doPsymonLoop();
      break;
    default:
      Serial.println("No game type chosen; this should never happen.");
  }
}
