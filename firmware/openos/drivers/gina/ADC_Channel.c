#include "openwsn.h"
#include "ADC_Channel.h"
#include "math.h"
#include "openwsn.h"

//===================================== variables =============================

bool ADC_configured = FALSE;  

//===================================== public ================================

void ADC_init() {
  // enable accelerometer and filter
  P6DIR  &=  ~0x01;                              // P6.3 as output for accelerometer and filter
  //P6OUT  |=  0x08;		                 // P6.3 high to enable accelerometer and filter
  
  P6SEL = 0x01;
  
  /* configuring the ADC12 */
  // SHT0_0 and SHT1_0: Sample-and-hold time is 4 ADC12CLK cycles
  // MSC: Multiple sample and conversion: sample-and-conversions performed automatically
  // ADC12ON: ADC12 on
  ADC12CTL0 = SHT0_0 + SHT1_0 + MSC + ADC12ON;
  // CSTARTADD_0: Conversion start address is 0
  // SHP: Sample-and-hold pulse-mode select, sourced from the sampling timer
  // CONSEQ_1: Conversion sequence mode select, Sequence-of-channels
  // ADC12SSEL_1: ADC Clock Source (32kHz crystal, 31.25us)
  // ADC12DIV_1: ADC Clock Divider is 2
  ADC12CTL1 = CSTARTADD_0 + SHP + CONSEQ_1 + ADC12SSEL_1 + ADC12DIV_1;
  
  ADC12MCTL0 = INCH_0;                  // ADC12MCTL0 <- P6.0 = open ADC
  ADC12MCTL1 = INCH_0;                          
  ADC12MCTL2 = INCH_0;
  ADC12MCTL3 = INCH_0;
  ADC12MCTL4 = INCH_0;
  ADC12MCTL5 = INCH_0;
  ADC12MCTL6 = INCH_0;
  ADC12MCTL7 = INCH_0;
  ADC12MCTL8 = INCH_0;
  ADC12MCTL9 = INCH_0;
  ADC12MCTL10 = INCH_0;
  ADC12MCTL11 = INCH_0;
  ADC12MCTL12 = INCH_0;
  ADC12MCTL13 = INCH_0;
  ADC12MCTL14 = INCH_0;
  ADC12MCTL15 = INCH_0 + EOS;
  
  ADC12IE = 0x0000;                               // no interrupts
  
  for (volatile int i=0; i<0x3600; i++) {}      // delay for ADC12 reference start-up
  ADC_configured = TRUE;
}

void ADC_disable() {
//  P6OUT   &=  ~0x08;   //Turn off P6.3 to disable accelerometer and filter
  ADC12CTL0 &= ~ADC12ON;
}

void ADC_enable() {
  ADC12CTL0 |= ADC12ON;  
}

void ADC_get_config() {
  if (ADC_configured==TRUE) {
  }
}

void ADC_getvoltage(uint16_t* spaceToWrite) {
  uint8_t i;
  if (ADC_configured==TRUE) {
    
    ADC12CTL0 |= ENC + ADC12SC;                             // sampling and conversion start
    
    spaceToWrite[0] = ADC12MEM0;
    spaceToWrite[1] = ADC12MEM1;
    spaceToWrite[2] = ADC12MEM2;
    spaceToWrite[3] = ADC12MEM3;
    spaceToWrite[4] = ADC12MEM4;
    spaceToWrite[5] = ADC12MEM5;
    spaceToWrite[6] = ADC12MEM6;
    spaceToWrite[7] = ADC12MEM7;
    spaceToWrite[8] = ADC12MEM8;
    spaceToWrite[9] = ADC12MEM9;
    spaceToWrite[10] = ADC12MEM10;
    spaceToWrite[11] = ADC12MEM11;
    spaceToWrite[12] = ADC12MEM12;
    spaceToWrite[13] = ADC12MEM13;
    spaceToWrite[14] = ADC12MEM14;
    spaceToWrite[15] = ADC12MEM15;
    
  } else {
    for (i=0;i<2;i++) {
      spaceToWrite[i] = 0;
    }
  }
}
