#include "NU32.h"
#include <stdio.h>
#define DELAYTIME 20000000

static volatile unsigned int x = 0;

void __ISR(_EXTERNAL_0_VECTOR, IPL2SOFT) Ext0ISR(void) {
  char msg[128] = {};
  x++;
  NU32_LED1 = 0;
  NU32_LED2 = 0;
  _CP0_SET_COUNT(0);

  while(_CP0_GET_COUNT() < DELAYTIME) { ; }

  sprintf(msg, "Pressed %3d times!", x);
  NU32_WriteUART3(msg);
  NU32_WriteUART3("\r\n");

  NU32_LED1 = 1;
  NU32_LED2 = 1;
  IFS0bits.INT0IF = 0;
}

int main(void) {
  NU32_Startup();
  __builtin_disable_interrupts();
  INTCONbits.INT0EP = 0;
  IPC0bits.INT0IP = 2;
  IPC0bits.INT0IS = 1;
  IFS0bits.INT0IF = 0;
  IEC0bits.INT0IE = 1;
  __builtin_enable_interrupts();

  while(1) {
    ;
  }
  return 0;
}
