#pragma once

#include <stdint.h>

extern uint16_t playNote(uint8_t note, uint16_t duration);
extern void playPlayerSound(int player);
extern void playWinSound();
extern void playLoseSound();
extern void playBootSound();
