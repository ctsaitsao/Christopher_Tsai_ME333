#include <xc.h>
#include "NU32.h"
#include <stdio.h>
#define DELAYTIME 20000000

static volatile unsigned int x = 0;

void delay(void);

int main(void) {
  TRISF = 0xFFFC;        
  NU32_LED1 = 1;
  NU32_LED2 = 1;

  while(1) {
    delay();
    NU32_LED1 = 1;
    NU32_LED2 = 1;
  }
  return 0;
}

void delay(void) {
  int j;
  for (j = 0; j < 1000000; j++) {
    if (!PORTDbits.RD7) {
      char msg[128] = {};
      x++;
      NU32_LED1 = 0;
      NU32_LED2 = 0;
      _CP0_SET_COUNT(0);

      while(_CP0_GET_COUNT() < DELAYTIME) { ; }

      sprintf(msg, "Pressed %3d times!", x);
      NU32_WriteUART3(msg);
      NU32_WriteUART3("\r\n");
    }
  }
}
