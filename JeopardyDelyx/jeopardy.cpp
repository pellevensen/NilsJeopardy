#include "jeopardy.h"
#include "common.h"
#include "random.h"
#include "volume2.h"

#define PLAYERS 4
#define GREEN_BUTTON_IDX 0
#define RED_BUTTON_IDX 1
#define GREEN_MASK (1 << GREEN_BUTTON_IDX)
#define RED_MASK (1 << RED_BUTTON_IDX)

static Volume vol;

static uint8_t activePlayer;
static uint8_t canPlay;
static uint8_t canReset;

static const uint8_t NO_PLAYER = 255;

static void playerSound1() {
  int frequencies[5] = { 392, 440, 349, 175, 262 };

  for (int l1 = 127; l1 < 255; l1 += 20) {
    for (int lev = 0; lev < 20; lev += 25) {
      for (uint8_t freq = 0; freq < 5; freq++) {
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

void setActivePlayer(int p) {
  lightsOut();
  activePlayer = p;
  if (p != NO_PLAYER) {
    lamp(p, true);
    playPlayerSound(p);
    Serial.print("Player initiative: ");
    Serial.println(p);
  }
}

static void resetGameCycle(const char msg[]) {
  setActivePlayer(NO_PLAYER);
  canPlay = 0;
  canReset = 0;
  Serial.println(msg);
}

void initJeopardy() {
  activePlayer = NO_PLAYER;
  canPlay = 0;
  canReset = 0;
  lightsOut();
}

void doJeopardyLoop() {
  if (checkDoubleClick()) {
    playLoseSound();
    resetGameCycle("Lose");
  }
  if (activePlayer == NO_PLAYER) {
    for (int i = 0; i < 4; i++) {
      canPlay |= !readPlayerButton(i) << i;
    }
    int shuffledPlayers[] = { 0, 1, 2, 3 };
    permuteArray(shuffledPlayers, 4);
    for (uint8_t i = 0; i < PLAYERS; i++) {
      int p = shuffledPlayers[i];
      if ((canPlay & (1 << p)) && readPlayerButton(p)) {
        setActivePlayer(p);
        break;
      }
    }
    Serial.println("\n");
  } else {
    canReset |= ((!readGreenButton() << GREEN_BUTTON_IDX) | (!readRedButton() << RED_BUTTON_IDX));
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
