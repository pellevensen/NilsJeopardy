#include "random.h"

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
