#include <xc.h>
#include <stdio.h>
#include "NU32.h"
#include "utilities.h"
#include "encoder.h"
#include "currentcontrol.h"
#include "positioncontrol.h"
#include "isense.h"

#define BUF_SIZE 200
#define PLOTPTS 100

static volatile duty_cycle = 0.25;
static volatile int ADCarrayM[5000];
static volatile int REFarrayM[1000];
static volatile int ref_array[5000];

int main()
{
  char buffer[BUF_SIZE];
  int i;
  int adcval = 0;
  int size_ref;
  NU32_Startup();
  NU32_LED1 = 1;
  NU32_LED2 = 1;
  set_mode(IDLE);

  __builtin_disable_interrupts();
  encoder_init();
  ADC_init();
  pwm_init();
  pos_init();
  __builtin_enable_interrupts();

  while(1)
  {

    NU32_ReadUART3(buffer,BUF_SIZE); // we expect the next character to be a menu command
    NU32_LED2 = 1;
    switch (buffer[0]) {
      case 'a': //Read current sensor (ADC)
      {
        sprintf(buffer,"%d\r\n", adc_read_3x(0));
        NU32_WriteUART3(buffer);
        break;
      }
      case 'b': //Read current sensor (mA)
      {
        NU32_LED2 = 0;
        sprintf(buffer,"%3.2f\r\n",(float) (0.5171 * adc_read_3x(0) + 461.7));
        NU32_WriteUART3(buffer);
        break;
      }
      case 'c': //Read encoder (counts)
      {
        encoder_counts();
        sprintf(buffer,"%d\r\n", encoder_counts()); // return the encoder count in degrees
        NU32_WriteUART3(buffer);
        break;
      }
      case 'd': //Read encoder (degrees)
      {
        encoder_counts();
        sprintf(buffer,"%3.2f\r\n",(float) ((encoder_counts() - 32768) * 360/(4*334))); // return the encoder count in degrees
        NU32_WriteUART3(buffer);
        break;
      }
      case 'e': //Reset encoder
      {
        sprintf(buffer, "%d\r\n", encoder_reset());
        NU32_WriteUART3(buffer); //send encoder reset to client
        break;
        break;
      }
      case 'f': //Set PWM (-100 to 100)
      {
        set_mode(PWM);
        sprintf(buffer,"%d\r\n", get_duty_cycle());
        NU32_WriteUART3(buffer);
        break;
      }
      case 'g': //Set current gains
      {
        set_current_gains();
        break;
      }
      case 'h': //Get current gains
      {
        sprintf(buffer, "%d %d\r\n",get_Kp_c(),get_Ki_c());
        NU32_WriteUART3(buffer);
        break;
      }
      case 'i': //Set position gains
      {
        set_positions_gains();
        break;
      }
      case 'j':
      {
        sprintf(buffer, "%d %d %d\r\n",get_Kp_p(),get_Kp_p(),get_Kd_p());
        NU32_WriteUART3(buffer);
        break;
      }
      case 'k': //ITEST
      {
        set_mode(ITEST);
        while(get_Str_data()){
          //wait for ITEST to be done
        }
        get_data(REFarrayM,ADCarrayM);
        sprintf(buffer, "%d\r\n",100);
        NU32_WriteUART3(buffer);
        for (i=0; i<100; i++){
          sprintf(buffer, "%d %d\r\n", REFarrayM[i], ADCarrayM[i]);
          NU32_WriteUART3(buffer);
        }
        break;
      }
      case 'l': //Go to angle (deg)
      {
        SPI4BUF = 0;  //reset counter
        encoder_counts();
        set_mode(HOLD);

        while(get_Str_data()){
          //wait for HOLD to be done
        }
        get_data(REFarrayM,ADCarrayM);
        sprintf(buffer, "%d\r\n",1000);
        NU32_WriteUART3(buffer);
        for (i=0; i<1000; i++){
          sprintf(buffer, "%d %d\r\n", REFarrayM[i], ADCarrayM[i]);
          NU32_WriteUART3(buffer);
        }
        break;
      }
      case 'm': //Load step trajectory
      {
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer, "%d" , &size_ref);
        //int ref_array[size_ref];
        for(i=0;i<size_ref;i++){
          NU32_ReadUART3(buffer,BUF_SIZE);
          sscanf(buffer, "%d" , ref_array+i);
        }
        break;
      }
      case 'n': //Load cubic trajectory
      {
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer, "%d" , &size_ref);
        //int ref_array[size_ref];
        for(i=0;i<size_ref;i++){
          NU32_ReadUART3(buffer,BUF_SIZE);
          sscanf(buffer, "%d" , (int) (ref_array+i));
        }
        break;
      }
      case 'o': //Execute trajectory
      {
        send_ref_array(ref_array,size_ref);
        set_mode(TRACK);
        while(get_Str_data()){
          //wait for TRACK to be done
        }
        get_data_2(REFarrayM,ADCarrayM);
        sprintf(buffer, "%d\r\n",size_ref);
        NU32_WriteUART3(buffer);
        for (i=0; i<size_ref; i++){
          sprintf(buffer, "%d %d\r\n", REFarrayM[i], ADCarrayM[i]);
          NU32_WriteUART3(buffer);
        }
      }
      case 'p': //Unpower Motor
      {
        set_mode(IDLE);
        break;
      }
      case 'q': //Quit
      {
        set_mode(IDLE);
        break;
      }
      case 'r': //Get Mode
      {
         sprintf(buffer,"%d\r\n",get_mode()); // returns current mode
         NU32_WriteUART3(buffer);
         break;
      }
      default:
      {
        NU32_LED2 = 0;  // turn on LED2 to indicate an error
        break;
      }
    }
  }
  return 0;
}
