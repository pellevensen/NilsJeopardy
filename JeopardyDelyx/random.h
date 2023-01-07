#pragma once

#include <stdint.h>

extern void permuteArray(uint32_t* rng, int arr[], int size);
extern void permuteArray(int arr[], int size);
extern uint32_t next32(uint32_t* state);
extern uint32_t nextRandom();