#include "NU32.h"
#include <xc.h>
#include "currentcontrol.h"
#include "utilities.h"

static volatile int eint = 0;
static volatile int ADCarray[100];
static volatile int REFarray[100];
static volatile int StoringData = 1;

void __ISR(_TIMER_4_VECTOR, IPL6SOFT) currentcontrol(void) {
  int e = 0;
  int u = 0;
  volatile static int r = 200;
  static int i = 0;
  static int adcval = 0;
  int unew = 0;
  int Kp, Ki;

  switch(get_mode()){
    case 0: //IDLE
    {
      OC1RS = 0; //brake
      break;
    }
    case 1: //PWM
    {
      LATDbits.LATD1 = get_dir(); //0 = forward +ve  1 = reverse -ve
      OC1RS = get_duty_cycle();
      break;
    }
    case 2: //ITEST
    {
      LATDbits.LATD1 = 0;
      if(i == 25 || i == 50 || i == 75){
        r = -1*r;
      }
      adcval = (int) (0.5171 * adc_read_3x(0) + 461.7);
      e = r - adcval;
      eint = eint + e;
      Kp = get_Kp_c();
      Ki = get_Ki_c();
      u = Kp*e + Ki*eint;
      if(u < 0){
         LATDSET = 0b10;; // enable direction bit 0 that makes H bridge go in reverse dir
         unew = u*-1;
      }
      else if(u >= 0) {
         unew = u;
      }
      if (unew > 100) {
        unew = 100;
      }
      OC1RS = (unsigned int) (unew/100)*(PR3+1); // duty cycle = OC1RS/(PR2+1) = 75%
      if (i == 100){
        StoringData = 0;
        sent_data(r, adcval , StoringData);
        i = 0;
        set_mode(IDLE);
      } else {
        sent_data(r, adcval, StoringData);
        i++;
      }
      break;
    }
    case 3:
    {
      LATDbits.LATD1 = 0;
      r = get_desired_current();
      adcval = (int) (0.5171 * adc_read_3x(0) + 461.7);
      e = r - adcval;
      eint = eint + e;
      Kp = get_Kp_c();
      Ki = get_Ki_c();
      u = Kp*e + Ki*eint;
      if(u < 0) {
         LATDSET = 0b10;;
         unew = u*-1;
      }
      else if(u >= 0) {

         unew = u;
      }
      if (unew > 100) {
        unew = 100;
      }
      OC1RS = (unsigned int) (unew/100)*(PR3+1); // duty cycle = OC1RS/(PR2+1) = 75%
      break;
    }
    case 4:
    {
      LATDbits.LATD1 = 0;
      r = get_desired_current();
      adcval = (int) (0.5171 * adc_read_3x(0) + 461.7);
      e = r - adcval;
      eint = eint + e;
      Kp = get_Kp_c();
      Ki = get_Ki_c();
      u = Kp*e + Ki*eint;
      if(u < 0){
         LATDSET = 0b10;;
         unew = u*-1;
      }
      else if(u >= 0) {
         unew = u;
      }
      if (unew > 100) {
        unew = 100;
      }
      OC1RS = (unsigned int) (unew/100)*(PR3+1); // duty cycle = OC1RS/(PR2+1) = 75%
      break;
    }
  }
  IFS0bits.T4IF = 0; //Clear flag
}

void pwm_init(void){
  TRISDbits.TRISD1 = 0;   //D1 dig O

  T3CONbits.ON = 1;       //TMR 3 on
  T3CONbits.TCKPS = 0;   //Prescaler 0
  PR3 = 3999;       //Given 20 kHz
  TMR3 = 0;        //Reset TMR3

  OC1CONbits.OCTSEL = 1;  //OC1 uses TMR3
  OC1CONbits.OCM = 0b110; //PWM mode, no fault pin
  OC1CONbits.ON = 1;     //OC1 on
  OC1RS = 1000;  //For a duty cycle of 25%

  T4CONbits.ON = 1;       //TMR 3 on
  T4CONbits.TCKPS = 1;   //Prescaler 1
  PR4 = 7999;  //Given 5 kHz
  TMR4 = 0;
  IPC4bits.T4IP = 6; //priority 6
  IPC4bits.T4IS = 0; //sub 0
  IFS0bits.T4IF = 0; // clear flag
  IEC0bits.T4IE = 1; //enable TMR4
}
