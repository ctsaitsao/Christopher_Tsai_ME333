#include "isense.h"
#include <xc.h>

void ADC_init(void) {
  AD1PCFGbits.PCFG0 = 0;                 // AN0 is an adc pin
  AD1CON3bits.ADCS = 2;                   // Tad = 75ns
  AD1CON3bits.SAMC = 2;           //        sample for 2 Tad                                    //
  AD1CON1bits.ADON = 1;                   // turn on A/D converter
}

static unsigned int adc_sample_convert(int pin) { //sample & convert the value of the given pin; pin should be configured as an analog input in AD1PCFG
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

unsigned int adc_read_3x(int pin) {

  static int adcval = 0;
  static int avg = 0;

  adcval = adc_sample_convert(0);
  avg = avg + adcval;
  adcval = adc_sample_convert(0);
  avg = avg + adcval;
  adcval = adc_sample_convert(0);
  avg = avg + adcval;
  adcval = adc_sample_convert(0);
  avg = avg + adcval;
  adcval = adc_sample_convert(0);
  avg = avg + adcval;

  avg = avg/5;

  return avg;
}
