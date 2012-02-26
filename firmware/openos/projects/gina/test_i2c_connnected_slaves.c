/**
\brief This is a standalone test program which tests which devices are attached to
       the I2C bus
       
It does so by 'pinging' every possible address. Every I2C slave has a unique
address, indicated in its data sheet.

Download the program to a GINA board, run it, and break the line indicated
in the code. Use the Watch Window to see the value of the variables 'connected',
a bitmap of addresses. The LEDs also count the number of discovered devices.
On a completely assembled GINA 2.2b/c board, you should see three I2C slaves
attached:
  .  24 = 0x18 is the large-scale accelerometer (by Kionix)
  .  30 = 0x1e is the the magnetometer (by Honeywell)
  . 104 = 0x68 is the gyroscope (by Invensense)
The digital connection is done through two-wire I2C serial bus:
   - P5.2: B1_I2C_SCL
   - P5.1: B1_I2C_SDA
Note: Bus Free Time Between STOP and START should be >1.3us

The debug pins are:
   - P1.1 toggle at every new address tested
   - P1.2 is on during the testing
   - P1.3 is toggled upon an USCIAB1TX_VECTOR interrupt (in ti_i2c.c)
   - P1.4 is toggled upon an USCIAB1RX_VECTOR interrupt (in ti_i2c.c)

Speed:
  - checking one address takes 40us
  - the whole test takes 6.112ms

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2010
*/

#include "msp430x26x.h"
#include "stdint.h"
#include "i2c.h"

uint8_t  connected[128];

void main(void)
{
   uint8_t current_address;
   WDTCTL  = WDTPW + WDTHOLD;                    // disable watchdog timer
   BCSCTL1 = CALBC1_16MHZ;                       // MCLK at 16MHz
   DCOCTL  = CALDCO_16MHZ;
   
   P5OUT  |=  0x10;                              // set P5.4 as output
   P5DIR  |=  0x10;                              // set P5.4 high to enable I2C on large scale accel

   P1DIR  |=  0x06;                              // P1.1-2 as outputs (for debug)
   P2DIR  |=  0x0F;                              // P2.1-4 as outputs (for LEDs)
   P2OUT  &= ~0x0F;                              // LEDs off

   __enable_interrupt();                         // global enable interrupts

   P1OUT |= 0x04;                                // P1.2 high
   for (current_address=1; current_address<128; current_address++) {
     P1OUT ^= 0x02;                              // toggle P1.1
     connected[current_address] = i2c_slave_present(1,current_address);
     if (connected[current_address]==0x01) {
       P2OUT++;
     }
   }
   P1OUT &= ~0x04;                               // P1.2 low
   P1OUT &= ~0x02;                               // P1.1 low
   
   __bis_SR_register(LPM4_bits);                 // sleep
}
