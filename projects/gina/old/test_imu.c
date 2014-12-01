/**
\brief This is a standalone test program to test all the IMU sensors on the GINA2.2b/c
       board
       
That is, the gyro, large range accel, magnetometer, sensitive accel, temperature.

Download the program to a GINA board, run it, and break after a few
seconds at the line indicated in the code. Use the Watch Window to see the 
value of the variables TBC.

The digital connection to the gyro, large range accel and magnetometeris is 
done through a two-wire I2C serial bus:
   - P5.2: B1_I2C_SCL
   - P5.1: B1_I2C_SDA

In addition:
   - P1.5: interrupt pin from the gyro (not used)
   - P1.7: interrupt pin from the large range accelerometer (not used)
   - P5.4: output to configure the I2C mode of the large range accelerometer (keep high)

The debug pins are:
   - P1.1: toggles at every measurement
   - P1.2: on during initial configuration and pulses at a new measurement
   - P1.3: toggles upon an USCIAB1TX_VECTOR interrupt (in ti_i2c.c)
   - P1.4: toggles upon an USCIAB1RX_VECTOR interrupt (in ti_i2c.c)

Speed:
 - one measurement every ~1.29ms

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2010
*/

#include "msp430x26x.h"
#include "stdint.h"
#include "opendefs.h"
#include "gyro.h"
#include "large_range_accel.h"
#include "magnetometer.h"
#include "sensitive_accel_temperature.h"

uint8_t  gyro_data[8];
uint8_t  large_range_accel_data[6];
uint8_t  magnetometer_data[6];
uint8_t  sensitive_accel_temperature_data[10];

void main(void)
{
   WDTCTL  = WDTPW + WDTHOLD;                    // disable watchdog timer
   BCSCTL1 = CALBC1_16MHZ;                       // MCLK at 16MHz
   DCOCTL  = CALDCO_16MHZ;

   P1DIR  |= 0x1E;                               // P1.1-4 as outputs (for debug)

   __enable_interrupt();                         // global enable interrupts

   //configuring the sensors
   if (*(&eui64+3)==0x09) {                      // this is a GINA board (not a basestation)
      i2c_init();
      gyro_init();
      large_range_accel_init();
      magnetometer_init();
      sensitive_accel_temperature_init();
   }

   //check sensor configuration is right
   gyro_get_config();
   large_range_accel_get_config();
   magnetometer_get_config();
   sensitive_accel_temperature_get_config();;

   //make continuous measurements
   while(1) {
      P1OUT ^= 0x02;                             // toggle P1.1 for debug
      P1OUT |= 0x04;P1OUT &= ~0x04;
      gyro_get_measurement(&(gyro_data[0]));
      P1OUT |= 0x04;P1OUT &= ~0x04;
      large_range_accel_get_measurement(&(large_range_accel_data[0]));
      P1OUT |= 0x04;P1OUT &= ~0x04;
      magnetometer_get_measurement(&(magnetometer_data[0]));
      P1OUT |= 0x04;P1OUT &= ~0x04;
      sensitive_accel_temperature_get_measurement(&(sensitive_accel_temperature_data[0]));
      __no_operation();                          //useless, just for breakpoint
   }
}
