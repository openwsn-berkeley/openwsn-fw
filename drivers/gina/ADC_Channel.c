#include "opendefs.h"
#include "ADC_Channel.h"
#include "math.h"

//===================================== variables =============================

bool ADC_configured = FALSE;

//===================================== public ================================

void ADC_init() {
  volatile int i;
  
  // enable accelerometer and filter
  //P6DIR  |=  0x08;                              // P6.3 as output for accelerometer and filter
  //P6OUT  |=  0x08;		                 // P6.3 high to enable accelerometer and filter
  
  /* configuring the ADC12 */
  // SHT0_2: Sample-and-hold time is 16 ADC12CLK cycles
  // MSC: Multiple sample and conversion: sample-and-conversions performed automatically
  // ADC12ON: ADC12 on
  ADC12CTL0 = SHT0_2 + MSC + ADC12ON;
  // CSTARTADD_0: Conversion start address is 0
  // SHP: Sample-and-hold pulse-mode select, sourced from the sampling timer
  // CONSEQ_1: Conversion sequence mode select, Sequence-of-channels
  ADC12CTL1 = CSTARTADD_0 + SHP + CONSEQ_1;
  
  //ADC12MCTL0 = INCH_2;                          // ADC12MCTL0 <- P6.2 = X axis
  ADC12MCTL0 = INCH_0;                          // ADC12MCTL0 <- P6.2 = open ADC
  ADC12MCTL1 = INCH_1;                          // ADC12MCTL1 <- P6.1 = Y axis
  ADC12MCTL2 = INCH_5;                          // ADC12MCTL2 <- P6.5 = Z1 axis
  ADC12MCTL3 = INCH_6;                          // ADC12MCTL3 <- P6.6 = Z3 axis
  ADC12MCTL4 = INCH_7 + EOS;                    // ADC12MCTL4 <- P6.7 = temperature
  
  ADC12IE = 0x10;                               // interrupt only when ADC12MEM4 changes
  
  for (i=0; i<0x3600; i++) {}      // delay for ADC12 reference start-up
  ADC_configured = TRUE;
}

void ADC_disable() {
  P6OUT   &=  ~0x08;   //Turn off P6.3 to disable accelerometer and filter
}
void ADC_get_config() {
  if (ADC_configured==TRUE) {
  }
}

void ADC_getvoltage(uint16_t* spaceToWrite) {
  uint8_t i;
  if (ADC_configured==TRUE) {
    
    ADC12CTL0 |= ENC;                             // sampling and conversion start
    ADC12CTL0 |= ADC12SC;                         // start conversion
    __bis_SR_register(CPUOFF+GIE);                // turn off CPU, but leave on interrupts
    
    spaceToWrite[0] = ADC12MEM0;
    
  } else {
    for (i=0;i<2;i++) {
      spaceToWrite[i] = 0;
    }
  }
}
