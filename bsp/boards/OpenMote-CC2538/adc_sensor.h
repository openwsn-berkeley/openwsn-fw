#ifndef __ADC_SENSOR_H__
#define __ADC_SENSOR_H__

void adc_sensor_init(void);
uint16_t adc_sens_read_temperature(void);
float adc_sens_convert_temperature(uint16_t cputemp);

#endif // __ADC_SENSOR_H__
