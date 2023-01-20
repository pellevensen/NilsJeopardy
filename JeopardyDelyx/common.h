#pragma once

#include <stdint.h>

typedef enum {
  NO_GAME = -1,
  JEOPARDY = 0,
  TIME_BANDITS = 1,
  PSYMON = 2
} GameType;

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
extern uint8_t readTM1638Buttons();
extern uint8_t checkDoubleClick();
