#include "NU32.h"          // config bits, constants, funcs for startup and UART
#include "encoder.h"
#include "utilities.h"
#include "isense.h"
#define BUF_SIZE 200

static volatile duty_cycle = 0.25;
static volatile int ADCarrayM[5000];// Used to be 1000 size arrays
static volatile int REFarrayM[1000];// = {5, 5, 5, 5, 5};
static volatile int ref_array[5000];// = {5, 5, 5, 5, 5};
//static volatile float ref_array_cubic[1000];// = {5, 5, 5, 5, 5};

int main()
{
  char buffer[BUF_SIZE];
  NU32_Startup(); // cache on, min flash wait, interrupts on, LED/button init, UART init
  NU32_LED1 = 1;  // turn off the LEDs
  NU32_LED2 = 1;
  __builtin_disable_interrupts();
  ADC_init();
  encoder_init();
  set_mode(0);
  init_pwm();
  init_pos();
  __builtin_enable_interrupts();

  while(1)
  {
    NU32_ReadUART3(buffer,BUF_SIZE); // we expect the next character to be a menu command
    NU32_LED2 = 1;                   // clear the error LED
    switch (buffer[0]) {
      case 'a':
        sprintf(buffer, "%d\r\n", adc_read_3x(0));
        NU32_WriteUART3(buffer); //send encoder count to client
        break;
      case 'b':
        sprintf(buffer, "%3.2f\r\n", (float) 1.858 * adc_read_3x(0) - 858 - 33);
        NU32_WriteUART3(buffer); //send encoder count to client
        break;
      case 'c':                      // dummy command for demonstration purposes
      {
        sprintf(buffer, "%d\r\n", encoder_counts());
        NU32_WriteUART3(buffer); //send encoder count to client
        break;
      }
      case 'd':
      {
        sprintf(buffer, "%d\r\n", (encoder_counts() - 32768) * 360/(4*448));
        NU32_WriteUART3(buffer); //send encoder count to client
        break;
      }
      case 'e':
      {
        sprintf(buffer, "%d\r\n", encoder_reset());
        NU32_WriteUART3(buffer); //send encoder reset to client
        break;
      }
      case 'f': //PWM
      {
        set_mode(1);
        sprintf(buffer,"%d\r\n", get_duty_cycle());
        NU32_WriteUART3(buffer);
        break;
      }
      case 'h': //Get current gains
      {
        sprintf(buffer, "%d %d\r\n",get_current_Kp(),get_current_Ki());
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
        sprintf(buffer, "%d %d %d\r\n",get_position_Kp(),get_position_Ki(),get_position_Kd());
        NU32_WriteUART3(buffer);
        break;
      }
      case 'k': //Test current gains
      {
        set_mode(2); //ITEST
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
      case 'p': //Unpower Motor
        {
          set_mode(0);//IDLE
          break;
        }
      case 'q':
      {
        // handle q for quit. Later you may want to return to IDLE mode here.
        break;
      }
      case 'r':
      {
        sprintf(buffer, "%d\r\n", get_mode());
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
