/**
\brief This is a standalone test program for the sensitive accelometer and temperature
       sensor of the GINA2.2b/c board.

Measurement are typically done simultaneously  because they both output analog voltages.

Download the program to a GINA board, run it, and break after a few seconds at the line
indicated in the code. Use the Watch Window to see the 
value of 'sensitive_accel_x', '..._y', '..._z1', '..._z2' and 'temperature'.

The analog input are:
   - P6.1: Y axis
   - P6.2: X axis
   - P6.5: Z axis, after first order filter (abbreviated Z1)
   - P6.6: Z axis, after third order filter (abbreviated Z3)
   - P6.7: temperature

The digital outputs to the accelerometer are:
   - P6.3: pull high enables the accelerometer and analog filter
   - P6.4: full scale selection (0: +/-2g full-scale; 1: +/-6g full-scale)

The debug pins are:
   - P1.1 toggles at every measurement

Speed:
   - one measurement alone takes ~35s
   - one measurement followed by the conversion to real temperature takes ~65us

\author Leo Keselman <lkeselman@berkeley.edu>, July 2010
\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2010
*/

#include "msp430x26x.h"
#include "stdint.h"
#include "sensitive_accel_temperature.h"

uint8_t sensitive_accel_temperature_data[10];
uint16_t sensitive_accel_x;
uint16_t sensitive_accel_y;
uint16_t sensitive_accel_z1;
uint16_t sensitive_accel_z3;
uint16_t temperature;

int main(void) {
   uint16_t raw_temperature;
   WDTCTL  = WDTPW + WDTHOLD;                    // disable watchdog timer
   BCSCTL1 = CALBC1_16MHZ;                       // MCLK at 16MHz
   DCOCTL  = CALDCO_16MHZ;

   P1DIR  |= 0x06;                               // P1.1,2 as output (for debug)

   __enable_interrupt();                         // global enable interrupts

   //configuring the sensitive accelometer and temperature sensor
   sensitive_accel_temperature_init();

   //check configuration is right (break and use Watch window)
   sensitive_accel_temperature_get_config();

   //make continuous measurements
   while(1) {
      P1OUT ^= 0x02;                             // toggle P1.1 for debug
      sensitive_accel_temperature_get_measurement(&(sensitive_accel_temperature_data[0]));
      
      sensitive_accel_x  = sensitive_accel_temperature_data[0]+256*sensitive_accel_temperature_data[1];
      sensitive_accel_y  = sensitive_accel_temperature_data[2]+256*sensitive_accel_temperature_data[3];
      sensitive_accel_z1 = sensitive_accel_temperature_data[4]+256*sensitive_accel_temperature_data[5];
      sensitive_accel_z3 = sensitive_accel_temperature_data[6]+256*sensitive_accel_temperature_data[7];
      raw_temperature    = sensitive_accel_temperature_data[6]+256*sensitive_accel_temperature_data[7];

      // ADC12 is 12-bit: adc value ranges from 0 (@ 0V) to 4096 (@ +3V)
      // voltage transfer function: V=(adc_temperature*3)/4096
      // temperature transfer function (-10C to +65C): V = –0.01171*T + 1.8641
      // hence, T=(76116-30*adc_temperature)/480

      temperature = (76116-30*raw_temperature)/480;
      __no_operation();                          //useless, just for breakpoint
   }
}
