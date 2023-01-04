#include "volume2.h"

static const int LAMP_PINS[] = { 3, 12, 10, 8 };
static const int PLAYER_BUTTON_PINS[] = { 9, 11, 5, 7 };
static const int AUDIO_PIN = 6;
static const int GREEN_BUTTON_PIN = 4;
static const int RED_BUTTON_PIN = 2;
static const int GREEN_MASK = 1 << GREEN_BUTTON_PIN;
static const int RED_MASK = 1 << RED_BUTTON_PIN;
static const int NO_PLAYER = -1;

static uint32_t lampCtr;
static uint32_t rs;
static int activePlayer;
static int canPlay;
static int canReset;
static Volume vol;

static void lightsOut() {
  for (int i = 0; i < sizeof(LAMP_PINS) / sizeof(int); i++) {
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

static void waitForReset() {
  int i = 0;
  while (!readRedButton() && !readGreenButton()) {
    int k = i % 6;
    if (k >= 4) {
      k = 6 - k;
    }
    digitalWrite(LAMP_PINS[k], 0);
    delay(100);
    digitalWrite(LAMP_PINS[k], 1);
    i++;
  }
}

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < sizeof(LAMP_PINS) / sizeof(int); i++) {
    pinMode(LAMP_PINS[i], OUTPUT);
  }
  pinMode(AUDIO_PIN, OUTPUT);

  for (int i = 0; i < sizeof(PLAYER_BUTTON_PINS) / sizeof(int); i++) {
    pinMode(PLAYER_BUTTON_PINS[i], INPUT);
  }
  pinMode(GREEN_BUTTON_PIN, INPUT);
  pinMode(RED_BUTTON_PIN, INPUT);

  activePlayer = NO_PLAYER;
  canPlay = 0;
  canReset = 0;
  lampCtr = 0;

  waitForReset();
  playWinSound();
  lightsOut();
}

static inline void fadeLamps() {
  lampCtr++;
  uint16_t ctr = lampCtr >> 8;
  uint16_t r = (rs += 987654321) >> 16;
  r ^= r >> 8;
  r += r << 8;
  r ^= r >> 8;
  r &= 0xFF;

  int intensity;
  if ((ctr & 255) > 128) {
    intensity = 255 - (ctr & 255);
  } else {
    intensity = (ctr & 255);
  }

  int lampOn = (r >= intensity * 2);

  for (int i = 0; i < sizeof(LAMP_PINS) / sizeof(int); i++) {
    digitalWrite(LAMP_PINS[i], lampOn);
  }
}

void closeEncounter(byte type) {
  int frequencies[5] = { 392, 440, 349, 175, 262 };

  for (int lev = 255; lev > 0; lev -= 55) {
    Serial.print("Volume: ");
    Serial.print((float(lev) / float(255)) * 100);
    Serial.println("%");
    for (byte freq = 0; freq < 5; freq++) {
      vol.tone(frequencies[freq] * 2, type + freq, lev);
      vol.delay(200);
    }
    vol.noTone();
    delay(100);
  }
  Serial.println();
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
    Serial.print("Volume: ");
    Serial.print((float(255) / float(255)) * 100);
    Serial.println("%");
    for (byte freq = 0; freq < 3; freq++) {
      vol.tone(frequencies[freq] + (next(&rng) & 127) - 63, SQUARE, 255);
      vol.delay(dur);
    }
    vol.noTone();
    delay(100);
  }
  int dur = 10;
  for (int i = 0; i < 15; i++) {
    for (byte freq = 0; freq < 3; freq++) {
      vol.tone(frequencies[freq] + (next(&rng) & 3) - 1 - i * 2, SAWTOOTH, 255);
      vol.delay(dur);
    }
  }
  vol.noTone();
  delay(100);

  Serial.println();
}

static int next(uint32_t* rs) {
  *rs = *rs + 23456789;
  int r = *rs;
  r ^= r >> 16;
  r += 0x91239713;
  r += r << 16;
  r ^= r >> 16;
  r += 0x91239765;
  r ^= r >> 16;
  return r;
}

static void playerSound3() {
  uint32_t rs = 65431;

  for (int dur = 20; dur < 25; dur *= 1.1) {
    Serial.print("Volume: ");
    Serial.print((float(255) / float(255)) * 100);
    Serial.println("%");
    for (byte freq = 0; freq < 10; freq++) {
      int f = (next(&rs) & 511) + 450;
      Serial.println(f);

      vol.tone(f, SAWTOOTH, 255);
      vol.delay(dur);
    }
    vol.noTone();
    delay((next(&rs) & 127) + 20);
  }
  Serial.println();
}

static void playerSound4() {
  uint32_t rs = 65432;
  int freqs[] = { 262, 330, 392, 494 };
  for (int notes = 0; notes < 15; notes++) {
    int f = (next(&rs) & 3);
    Serial.println(f);

    vol.tone(freqs[f] * 2 + (next(&rs) & 15), SQUARE, 255);
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
  Serial.println("Played sound");
}

static void playWinSound() {
  int freqs[] = { 262, 330, 392, 494 };
  int rs = 125;
  for (int octave = 0; octave < 3; octave++) {
    for (int notes = 0; notes < sizeof(freqs) / sizeof(int); notes++) {
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
    for (int notes = 0; notes < 2; notes++) {
      vol.tone(freqs[notes] >> (next(&rs) & 3), SAWTOOTH, 255 - octave * 40);
      vol.delay(120);
      vol.noTone();
    }
  }
  vol.noTone();
  delay(100);
}

static void turnOffLamps() {
  for (int i = 0; i < sizeof(LAMP_PINS) / sizeof(int); i++) {
    digitalWrite(LAMP_PINS[i], 1);
  }
}

void setActivePlayer(int p) {
  turnOffLamps();
  digitalWrite(LAMP_PINS[p], 0);
  activePlayer = p;
  playPlayerSound(p);
  Serial.print("Player ");
  Serial.println(p);
}

// Standard Fisher-Yates shuffle.
void permutePlayers(uint32_t* rng, int players[], int size) {
  *rng += 23456789;
  for (int i = size; i > 1; i--) {
    int r = next(rng) % i;
    int tmp = players[i - 1];
    players[i - 1] = players[r];
    players[r] = tmp;
  }
}

static void resetGameCycle(char* msg) {
  activePlayer = NO_PLAYER;
  canPlay = 0;
  lightsOut();
  canReset = 0;
  Serial.println(msg);
}

void loop() {
  if (activePlayer == NO_PLAYER) {
    for (int i = 0; i < 4; i++) {
      canPlay |= (!digitalRead(PLAYER_BUTTON_PINS[i])) << i;
    }
    int shuffledPlayers[] = { 0, 1, 2, 3 };
    permutePlayers(&rs, shuffledPlayers, 4);
    for (int i = 0; i < sizeof(PLAYER_BUTTON_PINS) / sizeof(int); i++) {
      int p = shuffledPlayers[i];
      if ((canPlay & (1 << p)) && digitalRead(PLAYER_BUTTON_PINS[p])) {
        setActivePlayer(p);
        break;
      }
    }
  } else {
    canReset |= ((!readGreenButton() << GREEN_BUTTON_PIN) | (!readRedButton() << RED_BUTTON_PIN));
    Serial.println(canReset);
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
