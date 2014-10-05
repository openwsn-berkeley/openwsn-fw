/**
\brief This is a standalone test program for the gyroscope of the GINA2.2b/c
       board.
       
Download the program to a GINA board, run it, and break after a few
seconds at the line indicated in the code. Use the Watch Window to see the 
value of the variables 'temperature', 'gyro_x', 'gyro_y' and 'gyro_z'.

The digital connection is done through two-wire I2C serial bus:
  - P5.2: B1_I2C_SCL
  - P5.1: B1_I2C_SDA
  
The digital input is:
  - P1.5: interrupt (not used)

The debug pins are:
   - P1.1: toggles at every measurement
   - P1.2: on during initial configuration
   - P1.3: toggles upon an USCIAB1TX_VECTOR interrupt (in ti_i2c.c)
   - P1.4: toggles upon an USCIAB1RX_VECTOR interrupt (in ti_i2c.c)

Speed: one measurement takes ~293us

\author Leo Keselman <lkeselman@berkeley.edu>, July 2010
\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2010
*/

#include "msp430x26x.h"
#include "stdint.h"
#include "leds.h"
#include "opendefs.h"
#include "gyro.h"

uint8_t  gyro_data[8];
uint16_t temperature;
uint16_t gyro_x;
uint16_t gyro_y;
uint16_t gyro_z;

void main(void)
{
   WDTCTL  = WDTPW + WDTHOLD;                    // disable watchdog timer
   BCSCTL1 = CALBC1_16MHZ;                       // MCLK at 16MHz
   DCOCTL  = CALDCO_16MHZ;
   
   P1DIR  |= 0x1E;                               // P1.1-4 as outputs (for debug)

   __enable_interrupt();                         // global enable interrupts
   
   leds_init();

   //configuring the gyro
   if (*(&eui64+3)==0x09) {                      // this is a GINA board (not a basestation)
      i2c_init():
      gyro_init();
   }

   //check configuration is right (break and use Watch window)
   gyro_get_config();

   //make continuous measurements
   while(1) {
      P1OUT ^= 0x02;                             // toggle P1.1 for debug
      gyro_get_measurement(&(gyro_data[0]));
      temperature = 256*gyro_data[0] + gyro_data[1];
      gyro_x      = 256*gyro_data[2] + gyro_data[3];
      gyro_y      = 256*gyro_data[4] + gyro_data[5];
      gyro_z      = 256*gyro_data[6] + gyro_data[7];
      leds_circular_shift();
      __no_operation();                          //useless, just for breakpoint
   }
}