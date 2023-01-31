#pragma once

#include <Arduino.h>
#include <stdint.h>
#include "common.h"

extern InitStatus initPsymon();
extern LoopStatus doPsymonLoop();
