#include "NU32.h"
#include <stdio.h>
#define VOLTS_PER_COUNT (3.3/1024)
#define CORE_TICK_TIME 25
#define SAMPLE_TIME 10
#define CORE_TICKS 4000000

unsigned int adc_sample_convert(int pin) {
  unsigned int elapsed = 0, finish_time = 0;
  AD1CHSbits.CH0SA = pin;
  AD1CON1bits.SAMP = 1;
  elapsed = _CP0_GET_COUNT();
  finish_time = elapsed + SAMPLE_TIME;
  while (_CP0_GET_COUNT() < finish_time) { ; }
  AD1CON1bits.SAMP = 0;
  while (!AD1CON1bits.DONE) { ; }
  return ADC1BUF0;
}

void __ISR(_CORE_TIMER_VECTOR, IPL6SRS) CoreTimerISR(void) {
  unsigned int sample0 = 0, elapsed = 0;
  char msg[100] = {};

  _CP0_SET_COUNT(0);
  sample0 = adc_sample_convert(0);

  sprintf(msg, "%5.3f volts\r\n", sample0 * VOLTS_PER_COUNT);
  NU32_WriteUART3(msg);
  _CP0_SET_COUNT(0);

  _CP0_SET_COUNT(0);
  _CP0_SET_COMPARE(CORE_TICKS);
  IFS0bits.CTIF = 0;
}

int main(void) {
  NU32_Startup();

  AD1PCFGbits.PCFG0 = 0;
  AD1CON3bits.ADCS = 2;
  AD1CON1bits.ON = 1;

  __builtin_disable_interrupts();
  _CP0_SET_COMPARE(CORE_TICKS);
  IPC0bits.CTIP = 6;
  IPC0bits.CTIS = 0;
  IFS0bits.CTIF = 0;
  IEC0bits.CTIE = 1;
  __builtin_enable_interrupts();

  _CP0_SET_COUNT(0);

  while(1) { ; }
  return 0;
}
