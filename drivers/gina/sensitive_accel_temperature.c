#include "opendefs.h"
#include "sensitive_accel_temperature.h"
#include "msp430x26x.h"

//=========================== variables =======================================

typedef struct {
   bool configured;
} sensitive_accel_temperature_vars_t;

sensitive_accel_temperature_vars_t sensitive_accel_temperature_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void sensitive_accel_temperature_init() {
   volatile int i;
   if (sensitive_accel_temperature_vars.configured==FALSE) {
      sensitive_accel_temperature_vars.configured = FALSE;
      
      // enable accelerometer and filter
      P6DIR  |=  0x08;                              // P6.3 as output for accelerometer and filter
      P6OUT  |=  0x08;		                 // P6.3 high to enable accelerometer and filter
   
      /* configuring the ADC12 */
      // SHT0_2: Sample-and-hold time is 16 ADC12CLK cycles
      // MSC: Multiple sample and conversion: sample-and-conversions performed automatically
      // ADC12ON: ADC12 on
      ADC12CTL0 = SHT0_2 + MSC + ADC12ON;
      // CSTARTADD_0: Conversion start address is 0
      // SHP: Sample-and-hold pulse-mode select, sourced from the sampling timer
      // CONSEQ_1: Conversion sequence mode select, Sequence-of-channels
      ADC12CTL1 = CSTARTADD_0 + SHP + CONSEQ_1;
   
      ADC12MCTL0 = INCH_2;                          // ADC12MCTL0 <- P6.2 = X axis
      ADC12MCTL1 = INCH_1;                          // ADC12MCTL1 <- P6.1 = Y axis
      ADC12MCTL2 = INCH_5;                          // ADC12MCTL2 <- P6.5 = Z1 axis
      ADC12MCTL3 = INCH_6;                          // ADC12MCTL3 <- P6.6 = Z3 axis
      ADC12MCTL4 = INCH_7 + EOS;                    // ADC12MCTL4 <- P6.7 = temperature
   
      ADC12IE = 0x10;                               // interrupt only when ADC12MEM4 changes
   
      for (i=0; i<0x3600; i++) {}      // delay for ADC12 reference start-up
      sensitive_accel_temperature_vars.configured = TRUE;
   }
}

void sensitive_accel_temperature_disable() {
   P6OUT   &=  ~0x08;   //Turn off P6.3 to disable accelerometer and filter
}
void sensitive_accel_temperature_get_config() {
   if (sensitive_accel_temperature_vars.configured==TRUE) {
   }
}

/**
writes 10 bytes of data to spaceToWrite.
- [0,1] acceleration in X axis
- [2,3] acceleration in Y axis
- [4,5] acceleration in Z axis
- [6,7] acceleration in Z axis, filtered
- [8,9] temperature
*/
void sensitive_accel_temperature_get_measurement(uint8_t* spaceToWrite) {
   uint8_t i;
   if (sensitive_accel_temperature_vars.configured==TRUE) {
      ADC12CTL0 |= ENC;                             // sampling and conversion start
      ADC12CTL0 |= ADC12SC;                         // start conversion
      __bis_SR_register(CPUOFF+GIE);                // turn off CPU, but leave on interrupts
      spaceToWrite[0] = (uint8_t)((uint16_t)(ADC12MEM0&0xFF00)>>8);
      spaceToWrite[1] = (uint8_t)((uint16_t)(ADC12MEM0&0x00FF));
      spaceToWrite[2] = (uint8_t)((uint16_t)(ADC12MEM1&0xFF00)>>8);
      spaceToWrite[3] = (uint8_t)((uint16_t)(ADC12MEM1&0x00FF));
      spaceToWrite[4] = (uint8_t)((uint16_t)(ADC12MEM2&0xFF00)>>8);
      spaceToWrite[5] = (uint8_t)((uint16_t)(ADC12MEM2&0x00FF));
      spaceToWrite[6] = (uint8_t)((uint16_t)(ADC12MEM3&0xFF00)>>8);
      spaceToWrite[7] = (uint8_t)((uint16_t)(ADC12MEM3&0x00FF));
      spaceToWrite[8] = (uint8_t)((uint16_t)(ADC12MEM4&0xFF00)>>8);
      spaceToWrite[9] = (uint8_t)((uint16_t)(ADC12MEM4&0x00FF));
   } else {
      for (i=0;i<10;i++) {
         spaceToWrite[i] = 0;
      }
   }
}

//=========================== private =========================================