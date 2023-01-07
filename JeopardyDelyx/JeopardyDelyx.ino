#include <stdint.h>
#include <TM1638plus.h>
#include "volume2.h"
#include "random.h"

static const uint8_t LAMP_PINS[] = { 3, 12, 10, 8 };
static const uint8_t PLAYER_BUTTON_PINS[] = { 9, 11, 5, 7 };
static const uint8_t AUDIO_PIN = 6;
static const uint8_t GREEN_BUTTON_PIN = 4;
static const uint8_t RED_BUTTON_PIN = 2;
static const uint8_t GREEN_MASK = 1 << GREEN_BUTTON_PIN;
static const uint8_t RED_MASK = 1 << RED_BUTTON_PIN;
static const uint8_t NO_PLAYER = 255;
static const uint8_t STROBE_PIN = A1;  // strobe = GPIO connected to strobe line of module
static const uint8_t CLOCK_PIN = A2;   // clock = GPIO connected to clock line of module
static const uint8_t DIO_PIN = A0;     // data = GPIO connected to data line of module

#define CONFIRM_BUTTON (1 << 7)
#define DECREASE_BUTTON (1 << 0)
#define INCREASE_BUTTON (1 << 1)

// static const
#if 0
#define TM1638_IO
#else
#define ARCADE_IO
#endif

typedef enum {
  NONE_CHOSEN = 0,
  JEOPARDY = 1,
  TIME_ESTIMATE = 2
} GameType;

static char GAME_NAMES[][10] = { "JEOP", "TMEST" };

static uint32_t lampCtr;
static uint32_t rs;
static uint8_t activePlayer;
static uint8_t canPlay;
static uint8_t canReset;
static GameType currentGame;

static Volume vol;
static TM1638plus tm(STROBE_PIN, CLOCK_PIN, DIO_PIN, false);

static void displayBinary(uint8_t value);

static void lightsOut() {
  for (uint8_t i = 0; i < sizeof(LAMP_PINS); i++) {
    digitalWrite(LAMP_PINS[i], 1);
  }
}

static void flashWinner(int p, int blinks) {
  lightsOut();
  for (int i = 0; i < blinks; i++) {
    digitalWrite(LAMP_PINS[p], 0);
    delay(100);
    digitalWrite(LAMP_PINS[p], 1);
    delay(50);
  }
}

static int readRedButton() {
  return digitalRead(RED_BUTTON_PIN);
}

static int readGreenButton() {
  return digitalRead(GREEN_BUTTON_PIN);
}

#define STEP_DELAY 100000

static int getUserValue(uint16_t min, uint16_t max) {
  uint16_t incSpeed = 20;
  uint16_t val = min;
  uint8_t lastButtons = 255;
  uint32_t lastActionTimeStamp = 0;

  while (1) {
    uint8_t buttons = tm.readButtons();
    if (lastButtons != buttons || micros() - lastActionTimeStamp >= STEP_DELAY) {
      if (lastButtons == buttons) {
        incSpeed *= 1.05;
      } else {
        incSpeed = 20;
      }
      lastButtons = buttons;
      lastActionTimeStamp = micros();
      if (buttons & CONFIRM_BUTTON) {
        break;
      }
      if (buttons & INCREASE_BUTTON) {
        val += incSpeed / 20;
        val = val > max ? max : val;
      }
      if (buttons & DECREASE_BUTTON) {
        val -= incSpeed / 20;
        val = val < min ? min : val;
      }
      tm.displayIntNum(val, false, TMAlignTextRight);
    }
  }
  return val;
}

static GameType waitForStart() {
  int i = 0;
  GameType done = NONE_CHOSEN;
  while (!done) {
    if (readRedButton() || readGreenButton()) {
      done = JEOPARDY;
    }
    uint8_t buttons = tm.readButtons();
    if (buttons != 0) {
      done = TIME_ESTIMATE;
      int val = getUserValue(10, 10000);
      Serial.println(val);
    }
    int k = i % 6;
    if (k >= 4) {
      k = 6 - k;
    }
    digitalWrite(LAMP_PINS[k], 0);
    //      tm.setLED(k, 1);
    delay(100);
    digitalWrite(LAMP_PINS[k], 1);
    //      tm.setLED(k, 0);
    displayBinary(1 << k);
    // tm.displayIntNum(k, false, TMAlignTextRight);
    //    tm.displayText("        ");
    //    tm.displayText(GAME_NAMES[i & 1]);
    i++;
  }
  return done;
}

void setup() {
  Serial.begin(9600);
  tm.displayBegin();
  vol.noTone();

  for (uint8_t i = 0; i < sizeof(LAMP_PINS); i++) {
    pinMode(LAMP_PINS[i], OUTPUT);
  }
  pinMode(AUDIO_PIN, OUTPUT);

  for (uint8_t i = 0; i < sizeof(PLAYER_BUTTON_PINS); i++) {
    pinMode(PLAYER_BUTTON_PINS[i], INPUT);
  }
  pinMode(GREEN_BUTTON_PIN, INPUT);
  pinMode(RED_BUTTON_PIN, INPUT);

  activePlayer = NO_PLAYER;
  canPlay = 0;
  canReset = 0;
  lampCtr = 0;

  playWinSound();
  currentGame = waitForStart();
  Serial.print("Game chosen: ");
  Serial.println(currentGame);
  playWinSound();
  lightsOut();
}

static void playerSound1() {
  int frequencies[5] = { 392, 440, 349, 175, 262 };

  for (int l1 = 127; l1 < 255; l1 += 20) {
    for (int lev = 0; lev < 20; lev += 25) {
      for (byte freq = 0; freq < 5; freq++) {
        vol.tone(frequencies[freq] * 2 - lev, SQUARE, lev + l1);
        vol.delay(10);
      }
    }
    vol.noTone();
    delay(10);
  }
}

static void playerSound2() {
  int frequencies[5] = { 220, 440, 660 };
  uint32_t rng = 123871;

  for (int dur = 25; dur > 20; dur *= 0.95) {
    for (byte freq = 0; freq < 3; freq++) {
      vol.tone(frequencies[freq] + (next32(&rng) & 127) - 63, SQUARE, 255);
      vol.delay(dur);
    }
    vol.noTone();
    delay(100);
  }
  int dur = 10;
  for (int i = 0; i < 15; i++) {
    for (byte freq = 0; freq < 3; freq++) {
      vol.tone(frequencies[freq] + (next32(&rng) & 3) - 1 - i * 2, SAWTOOTH, 255);
      vol.delay(dur);
    }
  }
  vol.noTone();
  delay(100);
}

static void playerSound3() {
  uint32_t rs = 65431;

  for (int dur = 20; dur < 26; dur *= 1.1) {
    int bFreq = (next32(&rs) & 255) + 400; 
    for (byte freq = 0; freq < 15; freq++) {
      int f = (next32(&rs) & 127) + bFreq;
      vol.tone(f, SAWTOOTH, 255);
      vol.delay(dur);
    }
    vol.noTone();
    delay((next32(&rs) & 7) + 20);
  }
}

static void playerSound4() {
  uint32_t rs = 65432;
  int freqs[] = { 262, 330, 392, 494 };
  for (int notes = 0; notes < 15; notes++) {
    int f = (next32(&rs) & 3);
    vol.tone(freqs[f] * 2 + (next32(&rs) & 15), SQUARE, 255);
    vol.delay(50);
  }
  vol.noTone();
  delay(100);
}

static void playPlayerSound(int p) {
  switch (p) {
    case 0: playerSound1(); break;
    case 1: playerSound2(); break;
    case 2: playerSound3(); break;
    case 3: playerSound4(); break;
  }
}

static void playWinSound() {
  int freqs[] = { 262, 330, 392, 494 };

  for (int octave = 0; octave < 3; octave++) {
    for (uint8_t notes = 0; notes < sizeof(freqs) / sizeof(int); notes++) {
      int f = freqs[notes] << octave;
      if (f < 4000) {}
      vol.tone(f, TRIANGLE, 255 - octave * 30);
      vol.delay(100);
    }
  }
  vol.noTone();
  delay(100);
}

static void playLoseSound() {
  int freqs[] = { 700, 500 };
  uint32_t rs = 125;
  for (int octave = 0; octave < 5; octave++) {
    for (uint8_t notes = 0; notes < sizeof(freqs) / sizeof(int); notes++) {
      vol.tone(freqs[notes] >> (next32(&rs) & 3), SAWTOOTH, 255 - octave * 40);
      vol.delay(120);
      vol.noTone();
    }
  }
  vol.noTone();
  delay(100);
}

static void turnOffLamps() {
  for (uint8_t i = 0; i < sizeof(LAMP_PINS); i++) {
    digitalWrite(LAMP_PINS[i], 1);
  }
}

void setActivePlayer(int p) {
  turnOffLamps();
  digitalWrite(LAMP_PINS[p], 0);
  activePlayer = p;
  playPlayerSound(p);
  Serial.print("Player initiative: ");
  Serial.println(p);
}

static void resetGameCycle(const char msg[]) {
  activePlayer = NO_PLAYER;
  canPlay = 0;
  lightsOut();
  canReset = 0;
  Serial.println(msg);
}

static void displayBinary(uint8_t value) {
  for (uint8_t LEDposition = 0; LEDposition < 8; LEDposition++) {
    tm.setLED(LEDposition, value & 1);
    value = value >> 1;
  }
}

static void readTM1638Buttons() {
  uint8_t buttons = tm.readButtons();
  tm.displayIntNum(buttons, true, TMAlignTextLeft);
  displayBinary(buttons);
}

static int checkDoubleClick() {
  static uint32_t lastClick = 0;
  static uint8_t lastState = 0;
  static uint32_t lastHandled = 0;

  if (millis() - lastHandled > 50) {
    lastHandled = millis();
    uint8_t redButton = readRedButton();
    if (lastState != redButton) {
      lastState = redButton;
      if (redButton == 0) {
        if (millis() - lastClick < 1000) {
          lastClick = 0;
          return 1;
        }
        lastClick = millis();
      }
    }
  }
  return 0;
}

static void doJeopardyLoop() {
  if (checkDoubleClick()) {
      playLoseSound();
      resetGameCycle("Lose");
  }
  if (activePlayer == NO_PLAYER) {
    for (int i = 0; i < 4; i++) {
      canPlay |= (!digitalRead(PLAYER_BUTTON_PINS[i])) << i;
    }
    int shuffledPlayers[] = { 0, 1, 2, 3 };
    permuteArray(&rs, shuffledPlayers, 4);
    for (uint8_t i = 0; i < sizeof(PLAYER_BUTTON_PINS); i++) {
      int p = shuffledPlayers[i];
      if ((canPlay & (1 << p)) && digitalRead(PLAYER_BUTTON_PINS[p])) {
        setActivePlayer(p);
        break;
      }
    }
  } else {
    canReset |= ((!readGreenButton() << GREEN_BUTTON_PIN) | (!readRedButton() << RED_BUTTON_PIN));
    if ((canReset & GREEN_MASK) && readGreenButton()) {
      playWinSound();
      flashWinner(activePlayer, 10);
      resetGameCycle("Win");
    } else if ((canReset & RED_MASK) && readRedButton()) {
      playLoseSound();
      resetGameCycle("Lose");
    }
    delay(50);
  }
}

static void doTimeEstimateLoop() {
  tm.displayIntNum(1000000000 - millis(), true, TMAlignTextLeft);
  delay(100);
  //
  //readTM1638Buttons();
}

void loop() {
  switch (currentGame) {
    case JEOPARDY:
      doJeopardyLoop();
      break;
    case TIME_ESTIMATE:
      doTimeEstimateLoop();
      break;
    case NONE_CHOSEN:
    default:
      Serial.println("No game type chosen; this should never happen.");
  }
}
