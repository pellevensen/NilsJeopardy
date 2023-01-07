#include <stdint.h>
#include "common.h"
#include "timebandits.h"

static uint32_t x = 0;

void initTimeBandits() {
  displayNumber(10);
}

void doTimeBanditsLoop() {
  displayNumber(x++);
}
