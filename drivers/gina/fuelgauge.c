/**
\brief Driver for the fuelgauge of the GINA daughter card

\author Ankur Mehta <mehtank@eecs.berkeley.edu>, August 2010
*/

//!!!!!!!!!!THIS CODE IS STILL UNDER DVELOPMENT. DO NOT USE.

#include "fuelgauge.h"

void fuelgauge_init(void) { 
   i2c_init(FUELGAUGE_I2C_ADDR);
}

int fuelgauge_test(void) {
   static char fuelgauge_volt[] = {FUELGAUGE_REG_VOLT};
   static char fuelgauge_result[2];

   I2C_ADDR(FUELGAUGE_I2C_ADDR);                         // Set slave address
   while(i2c_send(fuelgauge_volt,1));

   I2C_WAIT {
      if (UCB0STAT & UCNACKIFG)
         return 1;
   };

   while(i2c_recv(fuelgauge_result, 2));

   I2C_WAIT {
      if (UCB0STAT & UCNACKIFG)
         return 1;
   };
   P2OUT = (fuelgauge_result[1] << 4);

   return 0;
}

int fuelgauge_read(char value) {
   static char fuelgauge_volt[1];
   fuelgauge_volt[0] = value;

   static char* fuelgauge_result;
   static int fuelgauge_value = 0;
   fuelgauge_result = (char*)&fuelgauge_value;

   I2C_ADDR(FUELGAUGE_I2C_ADDR);                         // Set slave address
   while(i2c_send(fuelgauge_volt,1));
   while(i2c_recv(fuelgauge_result, 2));

   return fuelgauge_value;
}
