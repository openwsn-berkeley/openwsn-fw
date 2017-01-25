#include "adc_sensor.h"
#include "opendefs.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototype =======================================

void start(void);
void stop(void);

//=========================== public ==========================================

/**
   \brief Stop the sensors
*/
port_INLINE void stop(void)
{
  //stop convertion immediately
  ADC12CTL0 &= ~ENC;
  
  //need to remove CONSEQ_3 if not EOS is configured
  ADC12CTL1 &= ~CONSEQ_3;

  //wait for conversion to stop
  while(ADC12CTL1 & ADC12BUSY);

  //clear any pending interrupts
  ADC12IFG = 0;
}

/**
   \brief Start the sensors
*/
port_INLINE void start(void)
{
  //setup ADC12 parameters
  //128 sampling cycles
  //reference 2.5V
  //multiple samples
  //reference generator on
  ADC12CTL0 = SHT0_6 | SHT1_6 | REF2_5V | MSC | REFON;
  
  //sampling sourced from the sampling timer
  //repeat-sequence-of-channels
  ADC12CTL1 = SHP | CONSEQ_3;

  //clear all end-of-sequences up to P6.5
  ADC12MCTL0 &= ~EOS;
  ADC12MCTL1 &= ~EOS;
  ADC12MCTL2 &= ~EOS;
  ADC12MCTL3 &= ~EOS;
  ADC12MCTL4 &= ~EOS;
  ADC12MCTL5 |= EOS;
  
  //set the first conversion register P^.4
  ADC12CTL1 |= CSTARTADD_4;
 
  //enable the conversion
  ADC12CTL0 |= ADC12ON;
  ADC12CTL0 |= ENC;
  
  //start sampling
  ADC12CTL0 |= ADC12SC;
}

/**
   \brief Initialize the sensors
*/
void adc_sensor_init(void) {
  
  //stop the sensors
  stop();
  
  //set bits P6.4 and P6.5 as peripherals
  P6SEL |= ((1 << INCH_4) | (1 << INCH_5));
  
  //set ADC config
  ADC12MCTL4 = INCH_4 | SREF_0;
  ADC12MCTL5 = INCH_5 | SREF_0;
  
  start();
}


/**
   \brief Read rough data from sensor
   \returns Rough data
*/
uint16_t adc_sens_read_total_solar(void) {
   return ADC12MEM5;
}

/**
   \brief Convert rough data to human understandable
   \param[in] temp rough data.
   \returns Converted ADC value to lux
*/
float adc_sens_convert_total_solar(uint16_t light) {
   return (float)light;
}

/**
   \brief Read rough data from sensor
   \param[out] ui16Dummy rough data.
*/
uint16_t adc_sens_read_photosynthetic(void) {
   return ADC12MEM4;
}

/**
   \brief Convert rough data to human understandable
   \param[in] temp rough data.
   \returns Converted ADC value to lux
*/
float adc_sens_convert_photosynthetic(uint16_t light) {
   return (float)light;
}

//=========================== private =========================================
