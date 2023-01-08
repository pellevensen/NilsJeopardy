#include "sounds.h"
#include "volume2.h"
#include "random.h"

static Volume vol;

static void playerSound1() {
  const uint16_t frequencies[5] = { 392, 440, 349, 175, 262 };

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
  const uint16_t frequencies[5] = { 220, 440, 660 };
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
  const uint16_t freqs[] = { 262, 330, 392, 494 };
  for (int notes = 0; notes < 15; notes++) {
    int f = (next32(&rs) & 3);
    vol.tone(freqs[f] * 2 + (next32(&rs) & 15), SQUARE, 255);
    vol.delay(50);
  }
  vol.noTone();
  delay(100);
}

void playPlayerSound(int p) {
  switch (p) {
    case 0: playerSound1(); break;
    case 1: playerSound2(); break;
    case 2: playerSound3(); break;
    case 3: playerSound4(); break;
  }
}

void playWinSound() {
  const uint16_t freqs[] = { 262, 330, 392, 494 };

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

void playLoseSound() {
  const uint16_t freqs[] = { 700, 500 };
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

void playBootSound() {
  uint8_t volume = 255;
  uint32_t rs = 128;
  uint16_t freqs[] = {440, (uint16_t) (440 * 1.2599), (uint16_t) (440 * 1.33484), (uint16_t) (440 * 1.68179)};

  uint8_t octave = 1;
  for (uint16_t fIdx = 0; volume > 100; fIdx++) {
    octave = next32(&rs) % 3;
    uint16_t fo = freqs[fIdx & 3] << octave;
    if (fo < 2000) {
      uint8_t bv = volume * (fIdx % 5 == 3 ? 1.8 : 1.0);
      if(bv * volume > 255) {
        bv = 255;
      }      
      for(int vOff = 256; vOff > 30; vOff *= 0.8) {
        uint8_t av = (bv * vOff) >> 8;
        vol.tone(fo, PWM_25, av);
        vol.delay(12);
      }
      volume *= 0.9;
    }
  }
  vol.noTone();
  delay(100);
}
