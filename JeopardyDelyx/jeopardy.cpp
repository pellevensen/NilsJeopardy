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
static uint8_t cursorPos;
static uint8_t canReset;
static uint8_t playerScores[4];
static const uint8_t NO_PLAYER = 255;

static void displayScores();

static void setActivePlayer(int p) {
  lightsOut();
  activePlayer = p;
  if (p != NO_PLAYER) {
    lamp(p, true);
    displayBinary(1 << (2 * p + 1));
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

uint8_t initJeopardy() {
  activePlayer = NO_PLAYER;
  canPlay = 0;
  canReset = 0;
  for (int i = 0; i < 4; i++) {
    playerScores[i] = 0;
  }
  lightsOut();
  playWinSound();
  displayScores();
  cursorPos = 1;

  return 1;
}

static void insertNumber(char* textScores, uint8_t playerScore, int position) {
  uint8_t tens = playerScore / 10;
  if (playerScore >= 10) {
    textScores[position] = '0' + tens;
  }
  textScores[position + 1] = '0' + (playerScore - 10 * tens);
}

static void displayScores() {
  char textScores[] = "        ";
  for (int i = 0; i < 4; i++) {
    insertNumber(textScores, playerScores[i], 2 * i);
  }
  displayText(textScores);
}

uint8_t doJeopardyLoop() {
  if (checkDoubleClick()) {
    playLoseSound();
    resetGameCycle("Lose");
  }
  if (activePlayer == NO_PLAYER) {
    if (toggled(BUT_LEFT)) {
      cursorPos = (cursorPos + 6) & 7;
    } else if (toggled(BUT_RIGHT)) {
      cursorPos = (cursorPos + 2) & 7;
    } else if (toggled(BUT_DOWN)) {
      playerScores[cursorPos >> 1] = max(playerScores[cursorPos >> 1] - 1, 0);
      displayScores();
    } else if (toggled(BUT_UP)) {
      playerScores[cursorPos >> 1]++;
      displayScores();
    }
    displayBinary(1UL << cursorPos);

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
  } else {
    canReset |= ((!readGreenButton() << GREEN_BUTTON_IDX) | (!readRedButton() << RED_BUTTON_IDX));
    if ((canReset & GREEN_MASK) && readGreenButton()) {
      playWinSound();
      flashWinner(activePlayer, 10);
      playerScores[activePlayer]++;
      displayScores();
      resetGameCycle("Win");
    } else if ((canReset & RED_MASK) && readRedButton()) {
      playLoseSound();
      resetGameCycle("Lose");
    }
    delay(50);
  }

  if (isTM1638ButtonPressed(BUT_BACK)) {
    waitForTM1638Flank();
    return 1;
  }

  return 0;
}
