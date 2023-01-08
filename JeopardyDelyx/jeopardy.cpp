#include "jeopardy.h"
#include "common.h"
#include "random.h"
#include "sounds.h"

#define PLAYERS 4
#define GREEN_BUTTON_IDX 0
#define RED_BUTTON_IDX 1
#define GREEN_MASK (1 << GREEN_BUTTON_IDX)
#define RED_MASK (1 << RED_BUTTON_IDX)

static uint8_t activePlayer;
static uint8_t canPlay;
static uint8_t canReset;

static const uint8_t NO_PLAYER = 255;

void setActivePlayer(int p) {
  lightsOut();
  activePlayer = p;
  if (p != NO_PLAYER) {
    lamp(p, true);
    playPlayerSound(p);
    Serial.print("Player initiative: ");
    Serial.println(p);
  }
}

static void resetGameCycle(const char msg[]) {
  setActivePlayer(NO_PLAYER);
  canPlay = 0;
  canReset = 0;
  Serial.println(msg);
}

void initJeopardy() {
  activePlayer = NO_PLAYER;
  canPlay = 0;
  canReset = 0;
  lightsOut();
}

void doJeopardyLoop() {
  if (checkDoubleClick()) {
    playLoseSound();
    resetGameCycle("Lose");
  }
  if (activePlayer == NO_PLAYER) {
    for (int i = 0; i < 4; i++) {
      canPlay |= !readPlayerButton(i) << i;
    }
    int shuffledPlayers[] = { 0, 1, 2, 3 };
    permuteArray(shuffledPlayers, 4);
    for (uint8_t i = 0; i < PLAYERS; i++) {
      int p = shuffledPlayers[i];
      if ((canPlay & (1 << p)) && readPlayerButton(p)) {
        setActivePlayer(p);
        break;
      }
    }
    Serial.println("\n");
  } else {
    canReset |= ((!readGreenButton() << GREEN_BUTTON_IDX) | (!readRedButton() << RED_BUTTON_IDX));
    if ((canReset & GREEN_MASK) && readGreenButton()) {
      playWinSound();
      flashWinner(activePlayer, 10);
      resetGameCycle("Win");
    } else if ((canReset & RED_MASK) && readRedButton()) {
      playLoseSound();
      resetGameCycle("Lose");
    }
    delay(50);
  }
}
