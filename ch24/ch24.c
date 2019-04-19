#include "NU32.h"
#include <stdio.h>
#define NUMSAMPS 1000 //number of points in waveform
#define PLOTPTS 200 //number of data points to plot
#define DECIMATION 10 //plot every 10th point
#define SAMPLE_TIME 10 //10 core timer ticks = 250 ns

static volatile int Waveform[NUMSAMPS]; //waveform
static volatile int ADCarray[PLOTPTS]; //measured values to plot
static volatile int REFarray[PLOTPTS]; //reference values to plot
static volatile int StoringData = 0; //if this flag equals 1, currently storing
static volatile float Kp = 0, Ki = 0; //control gains
static float Eint = 0;

unsigned int adc_sample_convert(int pin) { //sample & convert the value of the given pin; pin should be configured as an analog input in AD1PCFG
    unsigned int elapsed = 0, finish_time = 0;
    AD1CHSbits.CH0SA = pin; //connect chosen pin to MUXA for sampling
    AD1CON1bits.SAMP = 1; //start sampling
    elapsed = _CP0_GET_COUNT();
    finish_time = elapsed + SAMPLE_TIME;
    while (_CP0_GET_COUNT() < finish_time) {
      ; //sample for more than 250 ns
    }
    AD1CON1bits.SAMP = 0; //stop sampling and start converting
    while (!AD1CON1bits.DONE) {
      ; //wait for the conversion process to finish
    }
    return ADC1BUF0; //read the buffer with the result
}

void makeWaveform() {
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

void __ISR(_TIMER_2_VECTOR, IPL5SOFT) Controller(void) {
  static int counter = 0; //initialize counter once
  static int plotind = 0; //index for data arrrays; counts up to PLOTPTS - 1
  static int decctr = 0; // counts to store data once every DECIMATION
  static int adcval = 0; //adc value
  float e, u=0, unew;
  static float Eint = 0;

  adcval = adc_sample_convert(0);

  e = Waveform[counter]-adcval;
  Eint = Eint+e;
  u = e*Kp + Ki*Eint;
  unew = u + 50.0; //since PWM duty cycle can only be b/w 0 & PR3, unew is a percentage
  if (unew > 100.0) { //with limits of 100%
    unew = 100.0;
  }
  else if (unew < 0.0) { //and 0%
    unew = 0.0;
  }

  //OC1RS = Waveform[counter]; //sets OC1RS to open-loop input variable Waveform
  OC1RS = (unsigned int) ((unew/100.0) * PR3);

  if (StoringData) {
    decctr++;
    if (decctr == DECIMATION) { //after DECIMATION control loops
      decctr = 0; //reset DECIMATION counter
      ADCarray[plotind] = adcval; //store data in global arrrays
      REFarray[plotind] = Waveform[counter];
      plotind++; //increment plot data index
    }
    if (plotind == PLOTPTS) { //if max number of plot points plot is reached
      plotind = 0; //reset plot index
      StoringData = 0; //tell main() that data is ready to be sent to MATLAB
    }
  }

  counter++; //add one to counter every time ISR is entered
  if (counter == NUMSAMPS) {
    counter = 0; //rollover counter over when end of waveform is reached
  }
  IFS0bits.T2IF = 0;
}

int main(void) {
  NU32_Startup();
  makeWaveform();

  char message[100]; //message to and from MATLAB
  float kptemp = 0, kitemp = 0; //temporary local gains
  int i = 0; //plot data loop counter

  PR3 = 3999;
  TMR3 = 0;
  T3CONbits.TCKPS = 0;
  T3CONbits.ON = 1;
  OC1CONbits.OCM = 0b110;
  OC1RS = 3000;
  OC1R = 3000;
  OC1CONbits.OCTSEL = 1;
  OC1CONbits.ON = 1;

  AD1PCFGbits.PCFG0 = 0; //configures AN0 as adc pin
  AD1CON3bits.ADCS = 2; //Tad = 75 ns
  AD1CON1bits.ADON = 1; //turn on A/D converter
  AD1CON1bits.SSRC = 0b111; //auto converstion
  AD1CON1bits.ASAM = 0; //manual sampling

  PR2 = 2499;
  TMR2 = 0;
  T2CONbits.TCKPS = 5;
  T2CONbits.ON = 1;
  __builtin_disable_interrupts();
  IPC2bits.T2IP = 5;
  IPC2bits.T2IS = 0;
  IFS0bits.T2IF = 0;
  IEC0bits.T2IE = 1;
  __builtin_enable_interrupts();

  while (1) {
    NU32_ReadUART3(message, sizeof(message)); //wait for a message from MATLAB that tells us which kptemp and kitemp to use
    sscanf(message, "%f %f", &kptemp, &kitemp);
    __builtin_disable_interrupts();
    Kp = kptemp; //copy local variables to globals used by ISR, Kp & Ki are still zero zero at this point but kptemp and kitemp aren't as sscanf modifies them
    Ki = kitemp;
    __builtin_enable_interrupts(); //only 2 simple C commands while ISR is disabled
    StoringData = 1; //message to ISR to start storing data (with new gains in PIC)
    while (StoringData) { //wait until ISR says data storing is done
      ; //do nothing
    }
    for (i = 0; i < PLOTPTS; i++) { //send plot data to MATLAB; when first number sent = 1, MATLAB knows we're done
      sprintf(message, "%d %d %d\r\n", PLOTPTS - i, ADCarray[i], REFarray[i]);
      NU32_WriteUART3(message);
    }
    Eint = 0;
  }
}
