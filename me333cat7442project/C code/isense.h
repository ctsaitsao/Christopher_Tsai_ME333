#ifndef ISENSE__H__
#define ISENSE__H__
#define SAMPLE_TIME 10

void ADC_init(void);
unsigned int adc_read_3x(int pin);

#endif
