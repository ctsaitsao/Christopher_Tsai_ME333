#include "NU32.h"
#define NUMSAMPS 1000
static volatile int Waveform[NUMSAMPS];

int main(void) {
  NU32_Startup();

  T3CONbits.TCKPS = 0;
  PR3 = 3999;
  TMR3 = 0;
  OC1CONbits.OCM = 0b110;
  OC1RS = 3000;
  OC1R = 3000;
  T3CONbits.ON = 1;
  OC1CONbits.ON = 1;

  _CP0_SET_COUNT(0);
  while(_CP0_GET_COUNT() < 4*40000000) {;}

  OC1RS = 3000;

  while(1){;}
  return 0;
}
