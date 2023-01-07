#include <Arduino.h>
#include "random.h"

static uint32_t initRandom();
static uint32_t rngState = initRandom();

static uint32_t initRandom() {
  uint32_t x = 0;
  for(int i = 0; i < 20; i++) {
    x += analogRead(A3);
    x *= 23456789;
    x ^= x >> 16;
  }
  return x;
}

// Standard Fisher-Yates shuffle.
void permuteArray(uint32_t* rng, int players[], int size) {
  *rng += 23456789;
  for (int i = size; i > 1; i--) {
    int r = next32(rng) % i;
    int tmp = players[i - 1];
    players[i - 1] = players[r];
    players[r] = tmp;
  }
}

void permuteArray(int players[], int size) {
  for (int i = size; i > 1; i--) {
    uint32_t r = nextRandom() % i;
    int tmp = players[i - 1];
    players[i - 1] = players[r];
    players[r] = tmp;
  }
}

uint32_t next32(uint32_t* state) {
  *state = *state + 23456789;
  uint32_t r = *state;
  r ^= r >> 16;
  r += 0x91239713;
  r += r << 16;
  r ^= r >> 16;
  r += 0x91239765;
  r ^= r >> 16;

  return r;
}

uint32_t nextRandom() {
  return next32(&rngState);
}