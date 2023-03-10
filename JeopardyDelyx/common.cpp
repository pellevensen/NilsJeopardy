#include <Arduino.h>
#include <TM1638plus.h>
#include "random.h"
#include "common.h"
#include "time.h"
#include "math.h"

static const uint8_t STROBE_PIN = A1;  // strobe = GPIO connected to strobe line of module
static const uint8_t CLOCK_PIN = A2;   // clock = GPIO connected to clock line of module
static const uint8_t DIO_PIN = A0;     // data = GPIO connected to data line of module
static const uint8_t LAMP_PINS[] = { 3, 12, 10, 8 };
static const uint8_t GREEN_BUTTON_PIN = 4;
static const uint8_t RED_BUTTON_PIN = 2;
static const uint8_t PLAYER_BUTTON_PINS[] = { 9, 11, 5, 7 };
static const uint8_t AUDIO_PIN = 6;

#define STEP_DELAY 200000
#define MAX_GAME_DESCRIPTOR 9

static char GAME_NAMES[][MAX_GAME_DESCRIPTOR] = { "JPRDY", "TIMEBAND", "PSYMON", "NUMBUM" };

static TM1638plus tm(STROBE_PIN, CLOCK_PIN, DIO_PIN, false);

void waitForTM1638Flank() {
  uint8_t buttons = readTM1638Buttons();
  while (buttons == readTM1638Buttons()) {
    // Wait until some button pressed.
  }
}

const char* getGameName(uint8_t gameIdx) {
  if (gameIdx <= getGames()) {
    return GAME_NAMES[gameIdx];
  }
  return "ERROR";
}

uint8_t getGames() {
  return sizeof(GAME_NAMES) / MAX_GAME_DESCRIPTOR;
}

GameType getGame(uint8_t gameIdx) {
  switch (gameIdx) {
    case 0:
      return JEOPARDY;
    case 1:
      return TIME_BANDITS;
    case 2:
      return PSYMON;
    case 3:
      return NUMBUM;
  }
  Serial.print("Illegal game: ");
  Serial.println(gameIdx);
  return NO_GAME;
}

void initIO() {
  tm.displayBegin();

  for (uint8_t i = 0; i < sizeof(LAMP_PINS); i++) {
    pinMode(LAMP_PINS[i], OUTPUT);
  }
  pinMode(AUDIO_PIN, OUTPUT);

  for (uint8_t i = 0; i < sizeof(PLAYER_BUTTON_PINS); i++) {
    pinMode(PLAYER_BUTTON_PINS[i], INPUT);
  }

  pinMode(GREEN_BUTTON_PIN, INPUT);
  pinMode(RED_BUTTON_PIN, INPUT);
}

uint8_t readTM1638Buttons() {
  return tm.readButtons();
}

uint8_t isTM1638ButtonPressed(TM1638Button b) {
  return readTM1638Buttons() & b;
}

uint8_t toggled(TM1638Button b) {
  if (isTM1638ButtonPressed(b)) {
    while (isTM1638ButtonPressed(b)) {
      // Wait for release.
    }
    return 1;
  }
  return 0;
}

void lamp(uint8_t lampIdx, bool on) {
  digitalWrite(LAMP_PINS[lampIdx], !on);
}

uint8_t readPlayerButton(uint8_t playerIdx) {
  return digitalRead(PLAYER_BUTTON_PINS[playerIdx]);
}

uint8_t readPlayerButtons() {
  uint8_t buttons = 0;
  for (int i = 3; i >= 0; i--) {
    buttons <<= 1;
    buttons |= digitalRead(PLAYER_BUTTON_PINS[i]);
  }

  return buttons;
}

void lightsOut() {
  for (uint8_t i = 0; i < sizeof(LAMP_PINS); i++) {
    lamp(i, false);
  }
}

void flashWinner(int p, int blinks) {
  lightsOut();
  for (int i = 0; i < blinks; i++) {
    digitalWrite(LAMP_PINS[p], 0);
    delay(100);
    digitalWrite(LAMP_PINS[p], 1);
    delay(50);
  }
}

uint8_t readRedButton() {
  return digitalRead(RED_BUTTON_PIN);
}

uint8_t readGreenButton() {
  return digitalRead(GREEN_BUTTON_PIN);
}

void waitForGreenFlank() {
  uint8_t b = readGreenButton();
  while (b == readGreenButton()) {
    // Do nothing.
  }
}

#define INC_SPEED 20
#define INC_INC_SPEED 1.05

#if 0
uint16_t getUserValue(const char* text, uint16_t dflt, uint16_t min, uint16_t max) {
  uint16_t incSpeed = INC_SPEED;
  uint16_t val = dflt;
  uint8_t lastButtons = 255;
  uint32_t lastActionTimeStamp = 0;

  while (1) {
    uint8_t buttons = tm.readButtons();
    if (lastButtons != buttons || micros() - lastActionTimeStamp >= STEP_DELAY) {
      if (lastButtons == buttons) {
        incSpeed *= INC_INC_SPEED;
      } else {
        incSpeed = INC_SPEED;
      }
      lastButtons = buttons;
      lastActionTimeStamp = micros();
      if (buttons & CONFIRM_BUTTON) {
        break;
      }
      if (buttons & UP_BUTTON) {
        val += incSpeed / INC_SPEED;
        if (val >= max) {
          val = max;
          incSpeed = INC_SPEED;
        }
      }
      if (buttons & DOWN_BUTTON) {
        uint16_t newVal = val - incSpeed / INC_SPEED;
        if (newVal > val || newVal < min) {
          val = min;
          incSpeed = INC_SPEED;
        } else {
          val = newVal;
        }
      }
      tm.displayIntNum(val, false, TMAlignTextRight);
      tm.displayText(text);
    }
  }

  return val;
}
#endif

int16_t getUserCursorValue(const char* text, uint16_t dflt, uint16_t min, uint16_t max) {
  uint16_t val = dflt;
  uint8_t lastButtons = 255;
  uint32_t lastActionTimeStamp = 0;
  uint8_t cursorPos = 0;
  uint8_t maxCursorPos = log10(max);
  static const uint16_t positionDigits[] = { 1, 10, 100, 1000, 10000 };

  while (1) {
    uint8_t buttons = tm.readButtons();
    if (lastButtons != buttons || micros() - lastActionTimeStamp >= STEP_DELAY) {
      lastButtons = buttons;
      lastActionTimeStamp = micros();
      if (buttons & BUT_OK) {
        break;
      }
      if (buttons & BUT_UP) {
        val += positionDigits[cursorPos];
        if (val >= max) {
          val = max;
        }
      } else if (buttons & BUT_DOWN) {
        uint16_t newVal = val - positionDigits[cursorPos];
        if (newVal > val || newVal < min) {
          val = min;
        } else {
          val = newVal;
        }
      } else if (buttons & BUT_LEFT) {
        cursorPos = (cursorPos + 1) % maxCursorPos;
      } else if (buttons & BUT_RIGHT) {
        cursorPos = (cursorPos - 1 + maxCursorPos) % maxCursorPos;
      } else if (buttons & BUT_BACK) {
        waitForTM1638Flank();
        Serial.print("getUserCursorValue CANCEL");
        Serial.println(val);

        return -1;
      }

      tm.displayIntNum(val, false, TMAlignTextRight);
      tm.displayText(text);
      if ((getTime() & 0x100) > 0x80) {
        tm.display7Seg(7 - cursorPos, 1 << 7);
      }
    }
  }
  waitForTM1638Flank();

  tm.displayIntNum(val, false, TMAlignTextRight);
  tm.displayText(text);

  Serial.print("getUserCursorValue");
  Serial.println(val);

  return val;
}

void displayBinary(uint8_t value) {
  for (uint8_t LEDposition = 0; LEDposition < 8; LEDposition++) {
    tm.setLED(LEDposition, value & 1);
    value = value >> 1;
  }
}

void displayLEDScore(uint8_t playerIdx) {
  const uint8_t LED_SCORES[4] = { 0xFF, 0x7E, 0x3C, 0x18 };
  displayBinary(LED_SCORES[playerIdx]);
}

uint8_t checkDoubleClick() {
  static uint32_t lastClick = 0;
  static uint8_t lastState = 0;
  static uint32_t lastHandled = 0;

  if (getTime() - lastHandled > 50) {
    lastHandled = getTime();
    uint8_t redButton = readRedButton();
    if (lastState != redButton) {
      lastState = redButton;
      if (redButton == 0) {
        if (getTime() - lastClick < 1000) {
          lastClick = 0;
          return 1;
        }
        lastClick = getTime();
      }
    }
  }
  return 0;
}

void displayNumber(uint32_t v) {
  tm.displayIntNum(v, true, TMAlignTextLeft);
}

void displayText(const char* text) {
  tm.displayText("        ");
  tm.displayText(text);
}

int8_t selectString(const char* strings[], int size) {
  uint8_t idx = 0;
  uint8_t buttons = 0;
  uint8_t games = getGames();
  while (!(readTM1638Buttons() & BUT_OK)) {
    displayText(strings[idx]);
    waitForTM1638Flank();
    buttons = readTM1638Buttons();
    if (buttons & BUT_DOWN) {
      idx = (idx + size - 1) % size;
    } else if (buttons & BUT_UP) {
      idx = (idx + 1) % size;
    }
    if (buttons & BUT_BACK) {
      waitForTM1638Flank();
      return -1;
    }
  }
  return idx;
}
