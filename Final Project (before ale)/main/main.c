#include "NU32.h"          // config bits, constants, funcs for startup and UART
#include "encoder.h"
#include "utilities.h"
#include "isense.h"
#define BUF_SIZE 200

int main()
{
  char buffer[BUF_SIZE];
  NU32_Startup(); // cache on, min flash wait, interrupts on, LED/button init, UART init
  NU32_LED1 = 1;  // turn off the LEDs
  NU32_LED2 = 1;
  __builtin_disable_interrupts();
  ADC_init();
  encoder_init();
  set_mode(IDLE);
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
        set_mode(PWM);
        sprintf(buffer,"%d\r\n", get_duty_cycle());
        NU32_WriteUART3(buffer);
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
