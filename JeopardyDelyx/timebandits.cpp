#include <stdint.h>
#include "common.h"
#include "timebandits.h"

static uint32_t x = 0;

void initTimeBandits() {
  while(readTM1638Buttons() & 128) ;
  getUserValue("NILS", 0, 9999);
}

void doTimeBanditsLoop() {
  displayNumber(x++);
}
