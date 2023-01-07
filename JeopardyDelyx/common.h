#pragma once

#include <stdint.h>

typedef enum {
  NONE_CHOSEN = 0,
  JEOPARDY = 1,
  TIME_BANDITS = 2
} GameType;

extern const char* getGameName(int gameIdx);

extern void initIO();

extern void lightsOut();
extern void lamp(uint8_t lampIdx, bool on);
extern void flashWinner(int p, int blinks);
extern uint8_t readRedButton();
extern uint8_t readGreenButton();
extern uint8_t readPlayerButton(uint8_t playerIdx);
extern uint16_t getUserValue(const char* text, uint16_t min, uint16_t max);
extern void displayNumber(uint32_t v);
extern void displayBinary(uint8_t value);
extern uint8_t readTM1638Buttons();
extern uint8_t checkDoubleClick();
extern void playWinSound();
extern void playLoseSound();