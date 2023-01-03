#include "volume2.h"

static const int LAMP_PINS[] = { 3, 12, 10, 8 };
static const int PLAYER_BUTTON_PINS[] = { 9, 11, 5, 7 };
static const int AUDIO_PIN = 6;
static const int GREEN_BUTTON_PIN = 4;
static const int RED_BUTTON_PIN = 2;
static const int NO_PLAYER = -1;

static uint32_t lampCtr;
static uint32_t rs;
static int activePlayer;
static int canPlay;
static Volume vol;

static void lightsOut() {
  for (int i = 0; i < sizeof(LAMP_PINS) / sizeof(int); i++) {
    digitalWrite(LAMP_PINS[i], 1);
  }
}

static void flashWinner(int p, int blinks) {
  lightsOut();
  for(int i = 0; i < blinks; i++) {
    digitalWrite(LAMP_PINS[p], 0);
    delay(100);
    digitalWrite(LAMP_PINS[p], 1);
    delay(50);
  }
}

void setup() {
  // declare the ledPin as an OUTPUT:
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
  lampCtr = 0;
  while (!digitalRead(GREEN_BUTTON_PIN) && !digitalRead(RED_BUTTON_PIN)) {
    for (int i = 0; i < sizeof(LAMP_PINS) / sizeof(int); i++) {
      digitalWrite(LAMP_PINS[i], 0);
      delay(50);
      digitalWrite(LAMP_PINS[i], 1);
    }
  }
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
  int frequencies[5] = { 392, 440, 349, 175, 262 };  // Notes: G4, A4, F4, F3, C4

  for (int lev = 255; lev > 0; lev -= 55) {  // Reduce volume by ~20% each time we loop
    Serial.print("Volume: ");
    Serial.print((float(lev) / float(255)) * 100);
    Serial.println("%");
    for (byte freq = 0; freq < 5; freq++) {               // Play through the notes
      vol.tone(frequencies[freq] * 2, type + freq, lev);  // Start the voice at [freq] Hz, waveform [type], at volume [lev]
      vol.delay(200);                                     // use Volume delay during sound // Seems to hang!
    }
    //vol.delay(100);
    vol.noTone();  // end voice
    delay(100);
  }
  Serial.println();
}

static void playerSound1() {
  int frequencies[5] = { 392, 440, 349, 175, 262 };  // Notes: G4, A4, F4, F3, C4

  for (int lev = 255; lev > 0; lev -= 55) {  // Reduce volume by ~20% each time we loop
    Serial.print("Volume: ");
    Serial.print((float(lev) / float(255)) * 100);
    Serial.println("%");
    for (byte freq = 0; freq < 5; freq++) {          // Play through the notes
      vol.tone(frequencies[freq] * 2, SQUARE, lev);  // Start the voice at [freq] Hz, waveform [type], at volume [lev]
      vol.delay(20);                                 // use Volume delay during sound // Seems to hang!
    }
    //vol.delay(100);
    vol.noTone();  // end voice
    delay(100);
  }
  Serial.println();
}

static void playerSound2() {
  int frequencies[5] = { 220, 440, 660 };  // Notes: G4, A4, F4, F3, C4

  for (int dur = 30; dur > 6; dur *= 0.9) {  // Reduce volume by ~20% each time we loop
    Serial.print("Volume: ");
    Serial.print((float(255) / float(255)) * 100);
    Serial.println("%");
    for (byte freq = 0; freq < 3; freq++) {            // Play through the notes
      vol.tone(frequencies[freq] * 2, SAWTOOTH, 255);  // Start the voice at [freq] Hz, waveform [type], at volume [lev]
      vol.delay(dur);                                  // use Volume delay during sound // Seems to hang!
    }
    //vol.delay(100);
    vol.noTone();  // end voice
    delay(100);
  }
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
  uint32_t rs = 65432;

  for (int dur = 20; dur < 25; dur *= 1.1) {  // Reduce volume by ~20% each time we loop
    Serial.print("Volume: ");
    Serial.print((float(255) / float(255)) * 100);
    Serial.println("%");
    for (byte freq = 0; freq < 10; freq++) {  // Play through the notes
      int f = (next(&rs) & 511) + 250;
      Serial.println(f);

      vol.tone(f, SAWTOOTH, 255);  // Start the voice at [freq] Hz, waveform [type], at volume [lev]
      vol.delay(dur);              // use Volume delay during sound // Seems to hang!
    }
    //vol.delay(100);
    vol.noTone();  // end voice
    delay(100);
  }
  Serial.println();
}

static void playerSound4() {
  uint32_t rs = 65432;
  int freqs[] = { 262, 330, 392, 494 };
  for (int notes = 0; notes < 15; notes++) {
    int f = (next(&rs) & 3);
    Serial.println(f);

    vol.tone(freqs[f] * 2, SQUARE, 255);
    vol.delay(50);
  }
  //vol.delay(100);
  vol.noTone();  // end voice
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
    for (int notes = 0; notes < sizeof(freqs) / sizeof(int); notes++) {  // Reduce volume by ~20% each time we loop
      int f = freqs[notes] << octave;
      if (f < 4000) {}
      vol.tone(f, TRIANGLE, 255 - octave * 30);  // Start the voice at [freq] Hz, waveform [type], at volume [lev]
      vol.delay(100);
    }
  }
  vol.noTone();  // end voice
  delay(100);
}

static void playLoseSound() {
  int freqs[] = { 700, 500 };
  uint32_t rs = 125;
  for (int octave = 0; octave < 5; octave++) {
    for (int notes = 0; notes < 2; notes++) {                                  // Reduce volume by ~20% each time we loop
      vol.tone(freqs[notes] >> (next(&rs) & 3), SAWTOOTH, 255 - octave * 40);  // Start the voice at [freq] Hz, waveform [type], at volume [lev]
      vol.delay(120);                                                          // use Volume delay during sound // Seems to hang!
      vol.noTone();                                                            // end voice
    }
  }
  vol.noTone();  // end voice
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
    if (digitalRead(GREEN_BUTTON_PIN)) {
      playWinSound();
      flashWinner(activePlayer, 20);      
      resetGameCycle("Win");
    } else if (digitalRead(RED_BUTTON_PIN)) {
      playLoseSound();
      resetGameCycle("Lose");
    }
    delay(50);
  }
}
