#pragma once

#include <stdint.h>

#define REST ((uint8_t)254)
#define END_MELODY ((uint8_t)255)

extern void playMelody(uint8_t notes[], uint8_t durations[], int speed);
extern uint16_t playNote(uint8_t note, uint16_t duration);
extern uint16_t playNoteNonBlocking(uint8_t note, uint16_t duration);
extern void noteOff();
extern void soundDelay();
extern void playPlayerSound(int player);
extern void playWinSound();
extern void playLoseSound();
extern void playBootSound();
