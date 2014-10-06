/**
\brief This is a standalone test program for the magnetometer of the GINA2.2b/c
board.

Download the program to a GINA board, run it, and break after a few
seconds at the line indicated in the code. Use the Watch Window to see the 
value of the variables 'magnetometer_x', 'magnetometer_y' and 'magnetometer_z'.

The digital connection is done through two-wire I2C serial bus:
   - P5.2: B1_I2C_SCL
   - P5.1: B1_I2C_SDA
 
The debug pins are:
   - P1.1: toggles at every measurement
   - P1.2: on during initial configuration, and toggles upon a new measurement
   - P1.3: toggles upon an USCIAB1TX_VECTOR interrupt (in ti_i2c.c)
   - P1.4: toggles upon an USCIAB1RX_VECTOR interrupt (in ti_i2c.c)

Speed:
   - one measurement every ~242us (no data crunching, just the measurement)
   - one *new* measurement at 32Hz on average

\author Leo Keselman <lkeselman@berkeley.edu>, July 2010
\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, September 2010
*/

#include "msp430x26x.h"
#include "stdint.h"
#include "leds.h"
#include "opendefs.h"
#include "magnetometer.h"

uint8_t  magnetometer_data[6];
uint16_t poipoi;
uint16_t magnetometer_x, old_magnetometer_x;
uint16_t magnetometer_y, old_magnetometer_y;
uint16_t magnetometer_z, old_magnetometer_z;

void main(void)
{
   WDTCTL  = WDTPW + WDTHOLD;                    // disable watchdog timer
   BCSCTL1 = CALBC1_16MHZ;                       // MCLK at 16MHz
   DCOCTL  = CALDCO_16MHZ;
   
   P1DIR  |= 0x1E;                               // P1.1-4 as outputs (for debug)
   P4DIR  |= 0x20;                               // P4.5 as output (for debug)

   __enable_interrupt();                         // global enable interrupts
   
   leds_init();

   //configuring the magnetometer
   if (*(&eui64+3)==0x09) {                      // this is a GINA board (not a basestation)
     i2c_init(): 
     magnetometer_init();
   }

   //check configuration is right (break and use Watch window)
   magnetometer_get_config();

   //make continuous measurements
   while(1) {
      P1OUT ^= 0x02;                             // toggle P1.1 for debug
      P4OUT ^= 0x20;                             // toggle P4.5 for debug
      //magnetometer_enable();
      //magnetometer_get_measurement(&(magnetometer_data[0]));
      //magnetometer_disable();
      for (poipoi=0;poipoi<0xffff;poipoi++) {
         __no_operation();
      }
      /*old_magnetometer_x = magnetometer_x;
      old_magnetometer_y = magnetometer_y;
      old_magnetometer_z = magnetometer_z;
      magnetometer_x = 256*magnetometer_data[0]+magnetometer_data[1];
      magnetometer_y = 256*magnetometer_data[2]+magnetometer_data[3];
      magnetometer_z = 256*magnetometer_data[4]+magnetometer_data[5];
      if ((old_magnetometer_x != magnetometer_x) ||
          (old_magnetometer_y != magnetometer_y) ||
          (old_magnetometer_z != magnetometer_z)) {
         P1OUT ^= 0x04;
      }*/
      leds_circular_shift();
      __no_operation();                          //useless, just for breakpoint
   }
}