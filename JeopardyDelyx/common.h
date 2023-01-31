#pragma once

#include <stdint.h>

typedef enum {
  NO_GAME = -1,
  JEOPARDY = 0,
  TIME_BANDITS = 1,
  PSYMON = 2,
  NUMBUM = 3
} GameType;

typedef enum {
  BUT_OK = 128,
  BUT_BACK = 64,
  BUT_HELP = 32,
  BUT_DOWN = 1,
  BUT_UP = 2,
  BUT_LEFT = 4,
  BUT_RIGHT = 8
} TM1638Button;

extern const char* getGameName(uint8_t gameIdx);
extern uint8_t getGames();
extern GameType getGame(uint8_t gameIdx);

extern void initIO();

extern void lightsOut();
extern void lamp(uint8_t lampIdx, bool on);
extern void flashWinner(int p, int blinks);
extern uint8_t readRedButton();
extern uint8_t readGreenButton();
extern void waitForGreenFlank();
extern uint8_t readPlayerButton(uint8_t playerIdx);
extern uint8_t readPlayerButtons();

// extern uint16_t getUserValue(const char* text, uint16_t min, uint16_t max);
extern uint16_t getUserCursorValue(const char* text, uint16_t dflt, uint16_t min, uint16_t max);
extern void displayNumber(uint32_t v);
extern void displayText(const char* text);
extern void displayBinary(uint8_t value);
void displayLEDScore(uint8_t playerIdx);
extern uint8_t readTM1638Buttons();
extern void waitForTM1638Flank();
extern uint8_t isTM1638ButtonPressed(TM1638Button b);
extern uint8_t toggled(TM1638Button b);
extern uint8_t checkDoubleClick();

uint8_t selectString(const char* strings[], int size);
