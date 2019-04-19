#include "utilities.h"
#include <xc.h>
#include <stdio.h>

#define BUF_SIZE 200

static volatile int mode_num = 0; duty_cycle = 0, dir = 0; Kp_c = 0; Ki_c = 0;
static volatile int  Kp_p = 0; Ki_p = 0, Kd_p = 0; desired_angle = -1000; desired_current = 0;
static volatile int ADCarrayU[5000];
static volatile int REFarrayU[1000];
static volatile int StoringData = 1;
static volatile int s_ref = 0;
static volatile int ref_position[5000];

void set_mode(enum mode_t new_mode) {
  char buffer[BUF_SIZE];

  switch(new_mode){
    case IDLE:
    {
      mode_num = 0;
      duty_cycle = 0;
      break;
    }
    case PWM:
    {
      mode_num = 1;
      NU32_ReadUART3(buffer,BUF_SIZE);
      sscanf(buffer, "%d" , &duty_cycle);
      if(duty_cycle < 0){
        duty_cycle = -1*duty_cycle;
        dir = 1; //Go Revere (CW) dir
      }
      else{
        dir = 0; //Go Foward (CCW) dir
      }
      break;
    }
    case ITEST:
    {
      mode_num = 2;
      StoringData = 1;
      break;
    }
    case HOLD:
    {
      StoringData = 1;
      NU32_ReadUART3(buffer,BUF_SIZE);
      sscanf(buffer, "%d" , &desired_angle);
      while(desired_angle==-1000){
        //wait for user input
      }
      mode_num = 3; //Trigger ISRs
      break;
    }
    case TRACK:
    {
      StoringData = 1;
      mode_num = 4; //Trigger ISRs
      break;
    }
    default: {break;}
  }
}

void sent_data(int r_data, int adc_data, int Str_data){
  static int i = 0; //used to be static
  ADCarrayU[i] = adc_data;
  REFarrayU[i] = r_data;
  StoringData =Str_data;
  i++;
  if(Str_data==0){
    i = 0;
    desired_angle = -1000;
    Str_data = 1;
  }
}

void send_current(int u){
  desired_current = u;
}

void send_ref_array(int* ref_array,int size_ref){
  s_ref = size_ref;
  int j; //used to not be static
  for(j=0; j<size_ref;j++){
    *(ref_position + j) = ref_array[j];
  }
}

int get_desired_current(void){
  return desired_current;
}

int get_Str_data(void){
  return StoringData;
}
void get_data(int* r, int* adc){
  int i;
  for(i=0; i<1000; i++){
    *(adc + i) = ADCarrayU[i];
    *(r + i) = REFarrayU[i];
  }
}

void get_data_2(int* r, int* adc){
  int i;
  for(i=0; i<s_ref; i++){
    *(adc + i) = ADCarrayU[i];
    *(r + i) = ref_position[i];
  }
}

int get_ref_array(int* array){
  int i;
  for(i=0; i<s_ref; i++){
    *(array + i) = ref_position[i];
  }
  return s_ref;
}

void set_current_gains(void){
  char buffer[BUF_SIZE];
  NU32_ReadUART3(buffer,BUF_SIZE);
  sscanf(buffer, "%d" , &Kp_c);
  NU32_ReadUART3(buffer,BUF_SIZE);
  sscanf(buffer, "%d" , &Ki_c);
}

void set_positions_gains(void){
  char buffer[BUF_SIZE];
  NU32_ReadUART3(buffer,BUF_SIZE);
  sscanf(buffer, "%d" , &Kp_p);
  NU32_ReadUART3(buffer,BUF_SIZE);
  sscanf(buffer, "%d" , &Ki_p);
  NU32_ReadUART3(buffer,BUF_SIZE);
  sscanf(buffer, "%d" , &Kd_p);
}

enum mode_t get_mode(void){
  return mode_num;
}

int get_duty_cycle(void){
  return (duty_cycle*40);
}

int get_dir(void){
  return dir;
}

int get_Kp_c(void){
  return Kp_c;
}

int get_Ki_c(void){
  return Ki_c;
}

int get_Kp_p(void){
  return Kp_p;
}

int get_Ki_p(void){
  return Ki_p;
}

int get_Kd_p(void){
  return Kd_p;
}

int get_angle(void){
  return desired_angle;
}
