#include <Arduino.h>
#include <stdint.h>
#include "time.h"

static volatile uint32_t clock;
static volatile uint8_t clockHi;

void startClock() {
  cli();//stop interrupts

  TCCR2A = 0;                 // Reset entire TCCR1A to 0 
  TCCR2B = 0;                 // Reset entire TCCR1B to 0
  TCCR2B |= B00000100;        //Set CS20, CS21 and CS22 to 1 so we get prescalar 1024 
  TIMSK2 |= B00000100;        //Set OCIE1B to 1 so we enable compare match B
  OCR2B = 61;

  clock = 0;
  clockHi = 0;

  sei();
}

uint32_t getTime() {
  return (((uint32_t) clockHi << 30) | (clock >> 2)) * 0.99206349;
}

void timeDelay(uint32_t t) {
  uint32_t startTime = getTime();
  while(getTime() - startTime < t) ;
}

ISR(TIMER2_COMPB_vect) {
  TCNT2 = 0;
  clock++;
  if(clock == 0) {
    clockHi++;
  }
}
