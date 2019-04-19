#include "NU32.h"          // constants, functions for startup and UART
#include <stdio.h>
#define NUMSAMPS 1000
#define PLOTPTS 200
#define DECIMATION 10
#define SAMPLE_TIME 10       // 10 core timer ticks = 250 ns


unsigned int adc_sample_convert(int pin) { // sample & convert the value on the given
                                           // adc pin the pin should be configured as an
                                           // analog input in AD1PCFG
    unsigned int elapsed = 0, finish_time = 0;
    AD1CHSbits.CH0SA = pin;                // connect chosen pin to MUXA for sampling
    AD1CON1bits.SAMP = 1;                  // start sampling
    elapsed = _CP0_GET_COUNT();
    finish_time = elapsed + SAMPLE_TIME;
    while (_CP0_GET_COUNT() < finish_time) {
      ;                                   // sample for more than 250 ns
    }
    AD1CON1bits.SAMP = 0;                 // stop sampling and start converting
    while (!AD1CON1bits.DONE) {
      ;                                   // wait for the conversion process to finish
    }
    return ADC1BUF0;                      // read the buffer with the result
}


static volatile int Waveform[NUMSAMPS];
static volatile int ADCarray[PLOTPTS];
static volatile int REFarray[PLOTPTS];
static volatile int StoringData = 0;

static volatile float Kp=0, Ki=0;

void __ISR(_TIMER_2_VECTOR, IPL5SOFT) Controller(void) {  // INT step 1: the ISR

  static int counter = 0;
  static int plotind = 0;
  static int decctr = 0;
  static int adcval = 0;
  static float eint = 0;

  adcval = adc_sample_convert(0);

  float u=0, unew;
  float e = Waveform[counter]-adcval;
  eint = eint+e;
  u = e*Kp + Ki*eint;
  unew = u + 50.0;
  if (unew > 100.0) {
    unew = 100.0;
  }
  else if (unew < 0.0) {
    unew = 0.0;
  }

  OC1RS = (unsigned int) ((unew/100.0)*PR3);  // duty cycle = OC1RS/(PR2+1) = 75%

  if (StoringData) {
   decctr++;
   if (decctr == DECIMATION) {
     decctr = 0;
     ADCarray[plotind] = adcval;
     REFarray[plotind] = Waveform[counter];
     plotind++;
   }
   if (plotind == PLOTPTS) {
     plotind = 0;
     StoringData = 0;
   }
 }


  counter++;
  if (counter == NUMSAMPS) {
    counter = 0;
  }
  IFS0bits.T2IF = 0;              // clear interrupt flag
}

void makeWaveform() {

//int i = 0, center = 2000, A = 1000;
int i = 0, center = 500, A = 300;
for (i = 0; i < NUMSAMPS; i++) {
  if (i < NUMSAMPS/2) {
    Waveform[i] = center + A;
  }
  else {
    Waveform[i] = center - A;
  }
}
}

int main(void) {
  makeWaveform();

  NU32_Startup();          // cache on, interrupts on, LED/button init, UART init

  T3CONbits.TCKPS = 0;     // Timer2 prescaler N=4 (1:4)
  PR3 = 3999;              // period = (PR2+1) * N * 12.5 ns = 100 us, 10 kHz
  TMR3 = 0;                // initial TMR2 count is 0
  OC1CONbits.OCM = 0b110;  // PWM mode without fault pin; other OC1CON bits are defaults
  OC1RS = 3000;             // duty cycle = OC1RS/(PR2+1) = 75%
  OC1R = 3000;              // initialize before turning OC1 on; afterward it is read-only
  T3CONbits.ON = 1;        // turn on Timer3
  OC1CONbits.OCTSEL = 1;
  OC1CONbits.ON = 1;       // turn on OC1


  AD1PCFGbits.PCFG0 = 0;                  // AN0 is an adc pin
  AD1CON3bits.ADCS = 2;                   // ADC clock period is Tad = 2*(ADCS+1)*Tpb =
                                          //                           2*3*12.5ns = 75ns
  AD1CON1bits.ADON = 1;                   // turn on A/D converter


  __builtin_disable_interrupts(); // INT step 2: disable interrupts at CPU
                                  // INT step 3: setup peripheral
  PR2 = 1249;                     //             set period register
  TMR2 = 0;                       //             initialize count to 0
  T2CONbits.TCKPS = 0b110;            //             set prescaler to 64
  T2CONbits.TGATE = 0;            //             not gated input (the default)
  T2CONbits.ON = 1;               //             turn on Timer2
  IPC2bits.T2IP = 5;              // INT step 4: priority
  IPC2bits.T2IS = 1;              //             subpriority
  IFS0bits.T2IF = 0;              // INT step 5: clear interrupt flag
  IEC0bits.T2IE = 1;              // INT step 6: enable interrupt
  __builtin_enable_interrupts();  // INT step 7: enable interrupts at CPU



  char message[100];
  float kptemp=0, kitemp=0;
  int i=0;

  while(1) {
    NU32_ReadUART3(message, sizeof(message));
         sscanf(message, "%f %f", &kptemp, &kitemp);
         __builtin_disable_interrupts();
         Kp = kptemp;
         Ki = kitemp;
         __builtin_enable_interrupts();
         StoringData = 1;
         while(StoringData) {
           ;
         }
         for (i = 0; i<PLOTPTS; i++) {
           sprintf(message, "%d %d %d \r\n", PLOTPTS-i, ADCarray[i],REFarray[i]);
           NU32_WriteUART3(message);
         }                   // infinite loop
  }
  return 0;
}
