/**
\brief This is a standalone test program for the large range accelerometer of the GINA2.2b/c
board. Download the program to a GINA board, run it, and break after a few
seconds at the line indicated in the code. Use the Watch Window to see the 
value of the variables 'large_range_accel_x', 'large_range_accel_y' and 'large_range_accel_z'.

The digital connection is done through two-wire I2C serial bus:
   - P5.2: B1_I2C_SCL
   - P5.1: B1_I2C_SDA

The digital input is:
   - P1.7: interrupt (not used)

The digital ouput is:
   - P5.4: keep high to configure the I2C mode of the large range accelerometer

The debug pins are:
   - P1.1: toggles at every measurement
   - P1.2: on during initial configuration
   - P1.3: toggles upon an USCIAB1TX_VECTOR interrupt (in ti_i2c.c)
   - P1.4: toggles upon an USCIAB1RX_VECTOR interrupt (in ti_i2c.c)

Speed: one measurement takes ~738us

\author Leo Keselman <lkeselman@berkeley.edu>, July 2010
\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2010
*/

#include "msp430x26x.h"
#include "stdint.h"
#include "leds.h"
#include "opendefs.h"
#include "large_range_accel.h"

uint8_t  large_range_accel_data[6];
uint16_t large_range_accel_x;
uint16_t large_range_accel_y;
uint16_t large_range_accel_z;

void main(void)
{
   WDTCTL  = WDTPW + WDTHOLD;                    // disable watchdog timer
   BCSCTL1 = CALBC1_16MHZ;                       // MCLK at 16MHz
   DCOCTL  = CALDCO_16MHZ;
   
   P1DIR  |= 0x1E;                               // P1.1-4 as outputs (for debug)

   __enable_interrupt();                         // global enable interrupts
   
   leds_init();

   //configuring the large scale accelerometer
   //configuring the gyro
   if (*(&eui64+3)==0x09) {                      // this is a GINA board (not a basestation)
      i2c_init();
      large_range_accel_init();
   }

   //check configuration is right (break and use Watch window)
   large_range_accel_get_config();

   //make continuous measurements
   while(1) {
      P1OUT ^= 0x02;                             // toggle P1.1 for debug
      large_range_accel_get_measurement(&(large_range_accel_data[0]));
      large_range_accel_x = (256*large_range_accel_data[0]+large_range_accel_data[1])>>3;
      large_range_accel_y = (256*large_range_accel_data[2]+large_range_accel_data[3])>>3;
      large_range_accel_z = (256*large_range_accel_data[4]+large_range_accel_data[5])>>3;
      leds_circular_shift();
      __no_operation();                          //useless, just for breakpoint
   }
}
