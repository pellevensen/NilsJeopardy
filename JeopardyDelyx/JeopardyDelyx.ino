#include <stdint.h>
#include <TM1638plus.h>
#include "random.h"
#include "jeopardy.h"
#include "timebandits.h"
#include "psymon.h"
#include "numbum.h"
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
  const char* gameNames[getGames()];
  for (int i = 0; i < getGames(); i++) {
    gameNames[i] = getGameName(i);
  }
  uint8_t gameIdx = selectString(gameNames, getGames());
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
  uint8_t done = INIT_WAITING;
  while (done == INIT_WAITING) {
    Serial.begin(9600);
    initIO();
    startClock();
    playBootSound();
    displayText("        ");

    while (done == INIT_WAITING) {
      currentGame = waitForStart();
      Serial.print("Game chosen: ");
      Serial.println(currentGame);
      while (readTM1638Buttons() & 128)
        ;
      switch (currentGame) {
        case JEOPARDY:
          done = initJeopardy();
          break;
        case TIME_BANDITS:
          done = initTimeBandits();
          break;
        case PSYMON:
          done = initPsymon();
          break;
        case NUMBUM:
          done = initNumBum();
          break;
        default:
          Serial.println("Failed setup; no game type chosen; this should never happen.");
      }
      lightsOut();
    }

    if (done == INIT_CANCEL) {
      done = INIT_WAITING;
    }
  }
}

void loop() {
  uint8_t done = 0;
  switch (currentGame) {
    case JEOPARDY:
      done = doJeopardyLoop();
      break;
    case TIME_BANDITS:
      done = doTimeBanditsLoop();
      break;
    case PSYMON:
      done = doPsymonLoop();
      break;
    case NUMBUM:
      done = doNumBumLoop();
      break;
    default:
      Serial.println("No game type chosen; this should never happen.");
  }
  if (done) {
    setup();
  }
}
