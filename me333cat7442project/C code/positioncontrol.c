#include "NU32.h"
#include <xc.h>
#include "positioncontrol.h"
#include "utilities.h"

static volatile int eint = 0, eprev = 0;
static volatile int StoringData = 1;
static volatile int r_array[5000];

void __ISR(_TIMER_2_VECTOR, IPL7SOFT) positioncontrol(void) {
  int e = 0, edot = 0;
  int c = 0;
  volatile static int r;
  static int counter = 0;
  static int y = 0, yt = 0;
  int cnew = 0, excess = 0, counter_max = 0;
  int Kp_p, Ki_p, Kd_p,

  StoringData = 1;

  switch(get_mode()){
    case 3:
    {
      r = (get_angle())*1000; //get angle
      encoder_counts();
      y = (int) ((encoder_counts()-32768)*270); //get y (output)
      e = r - y;
      edot = e - eprev;
      eint = eint + e;
      Kp_p = get_Kd_p();
      Ki_p = get_Ki_p();
      Kd_p = get_Kd_p();
      c = Kp_p*e + Kp_p*eint +Kp_p*edot;
      cnew = c/1000;
      eprev = e;
      if(cnew>100){ //cap value at 100 to aboid int saturation
        cnew = 100;
      }
      else if(c<-100){
        cnew= -100;
      }
      send_current(cnew);
      if (counter==1000){ //1000 counts
        StoringData = 0;
        sent_data((r/1000),y/1000,StoringData);
        counter = 0;
        set_mode(IDLE);
      } else {
        sent_data((r/1000),y/1000,StoringData);
        counter++;
      }
      break;
    }
    case 4:
    {
      StoringData = 1;
      counter_max = get_ref_array(r_array);
      r = r_array[counter]*1000;
      encoder_counts();
      yt = (int) ((encoder_counts()-32768)*270);
      e = r - yt;
      edot = e - eprev;
      eint = eint + e;
      Kp_p = get_Kd_p();
      Ki_p = get_Ki_p();
      Kd_p = get_Kd_p();
      c = Kp_p*e + Kp_p*eint +Kp_p*edot;
      cnew = c/1000;
      eprev = e;
      if(cnew>100){
        cnew = 100;
      }
      else if(c<-100){
        cnew= -100;
      }
      send_current(cnew);
      if (counter==counter_max){
        StoringData = 0;
        sent_data((r/1000),yt/1000,StoringData);
        counter = 0;
        set_mode(IDLE);
      } else {
        sent_data((r/1000),yt/1000,StoringData);
        counter++;
      }
      break;
    }
  }
  IFS0bits.T2IF = 0; //flag to 0
}

void pos_init(void){
  TRISBbits.TRISB1 = 0; //B1 is dig O
  T2CONbits.ON = 1;       //TMR4 on
  T2CONbits.TCKPS = 6;   //Prescaler 1:64
  PR2 = 6249;  //given 200 Hz
  TMR2 = 0;
  IPC2bits.T2IP = 7; //priority 6
  IPC2bits.T2IS = 0; // sub 0
  IFS0bits.T2IF = 0; //clr TMR2 flag
  IEC0bits.T2IE = 1; //enable int
}
