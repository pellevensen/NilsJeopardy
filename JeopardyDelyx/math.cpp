#include "math.h"

uint8_t log10(uint16_t n) {
  uint8_t l = 0;
  while(n != 0) {
    l++;
    n /= 10;
  }

  return l;
}
