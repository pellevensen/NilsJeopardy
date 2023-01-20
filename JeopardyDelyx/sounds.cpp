#include <Arduino.h>
#include "sounds.h"
#include "Volume.h"
#include "random.h"

#define AUDIO_PIN 6

static Volume vol;
static boolean initialized = 0;

static void playTone(uint16_t frequency, uint8_t volume, uint16_t duration, bool turnOff) {
  if (!initialized) {
    vol.alternatePin(1);
    vol.begin();
    initialized = true;
  }
  vol.tone(frequency, volume);
  vol.fadeOut(duration);
  vol.delay(duration);
  if (turnOff) {
    vol.noTone();
  }
}

void soundDelay() {
  vol.delay(10);
}

static void soundOff() {
  if (initialized) {
    vol.delay(50);
    vol.end();
    digitalWrite(AUDIO_PIN, 0);
  }
  initialized = false;
}

uint16_t playNote(uint8_t note, uint16_t duration) {
  int freq = 440 * pow(2, note / 12.0);
  playTone(freq, 255, duration, true);
  soundOff();
  return duration + 51;
}

uint16_t playNoteNonBlocking(uint8_t note, uint16_t duration) {
  int freq = 440 * pow(2, note / 12.0);
  if (!initialized) {
    vol.alternatePin(1);
    vol.begin();
    initialized = true;
  }
  vol.tone(freq, 255);
  vol.fadeOut(duration);
  vol.delay(59);
  return duration + 51;
}

void noteOff() {
  vol.noTone();
}

void playMelody(uint8_t notes[], uint8_t durations[], int tempo) {
  int i = 0;

  while (notes[i] != END_MELODY) {
    if (notes[i] == REST) {
      delay(durations[i] * tempo);
    } else {
      playNote(notes[i], durations[i] * tempo);
    }
    i++;
  }
}

static void playerSound1() {
  const uint16_t frequencies[5] = { 392, 440, 349, 175, 262 };

  for (int l1 = 127; l1 < 255; l1 += 20) {
    for (int lev = 0; lev < 20; lev += 25) {
      for (uint8_t freq = 0; freq < 5; freq++) {
        playTone(frequencies[freq] * 2 - lev, lev + l1, 10, false);
      }
    }
    vol.delay(10);
  }
  soundOff();
}

static void playerSound2() {
  const uint16_t frequencies[5] = { 220, 440, 660 };
  uint32_t rng = 123871;

  for (int dur = 35; dur > 25; dur *= 0.95) {
    for (byte freq = 0; freq < 3; freq++) {
      playTone(frequencies[freq] + (next32(&rng) & 127) - 63, 255, dur, false);
    }
    vol.noTone();
    delay(100);
  }
  int dur = 15;
  for (int i = 0; i < 15; i++) {
    for (byte freq = 0; freq < 3; freq++) {
      playTone(frequencies[freq] + (next32(&rng) & 3) - 1 - i * 2, 255, dur, false);
    }
  }
  soundOff();
}

static void playerSound3() {
  uint32_t rs = 65431;

  for (int dur = 20; dur < 26; dur *= 1.1) {
    int bFreq = (next32(&rs) & 255) + 400;
    for (byte freq = 0; freq < 15; freq++) {
      int f = (next32(&rs) & 127) + bFreq;
      playTone(f, 255, dur, false);
    }
    vol.noTone();
    delay((next32(&rs) & 7) + 20);
  }
  soundOff();
}

static void playerSound4() {
  uint32_t rs = 65432;
  const uint16_t freqs[] = { 262, 330, 392, 494 };
  for (int notes = 0; notes < 15; notes++) {
    int f = (next32(&rs) & 3);
    playTone(freqs[f] * 2 + (next32(&rs) & 15), 255, 50, false);
  }
  soundOff();
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
      playTone(f, 255 - octave * 30, 100, false);
    }
  }
  soundOff();
}

void playLoseSound() {
  const uint16_t freqs[] = { 700, 500 };
  uint32_t rs = 125;
  for (int octave = 0; octave < 5; octave++) {
    for (uint8_t notes = 0; notes < sizeof(freqs) / sizeof(int); notes++) {
      playTone(freqs[notes] >> (next32(&rs) & 3), 255 - octave * 40, 120, true);
    }
  }
  soundOff();
}

void playBootSound() {
  for (uint32_t bFreq = 200; bFreq < 1000; bFreq *= 1.4142) {
    uint8_t v = 255;
    for (uint32_t oFreq = (bFreq << 9); oFreq > (bFreq << 7); oFreq *= 0.99) {
      if (oFreq >> 8 < 2000) {
        playTone(oFreq >> 8, v--, 2, false);
      }
    }
  }
  soundOff();
}
