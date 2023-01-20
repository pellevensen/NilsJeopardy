#include <Arduino.h>
#include "random.h"

static uint32_t initRandom();
static uint32_t rngState = initRandom();

static uint32_t initRandom() {
  uint32_t x = 0;

  for (int i = 0; i < 10; i++) {
    for (int i = A0; i <= A6; i++) {
      x += analogRead(i);
      x *= 23456789;
      x += micros();
      x ^= x >> 16;
    }
  }
  rngState = x;
  return rngState;
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
  for (int i = 0; i < 10; i++) {
    r ^= r >> 20 ^ r >> 7;
    r += 0x91239713;
    r += r << 16;
  }

  return r;
}

uint32_t nextRandom() {
  if (rngState == 0) {
    rngState = initRandom();
  }
  return next32(&rngState);
}
