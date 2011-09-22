/**
\brief Driver for the fuelgauge of the GINA daughter card

\author Ankur Mehta <mehtank@eecs.berkeley.edu>, August 2010
*/

//!!!!!!!!!!THIS CODE IS STILL UNDER DVELOPMENT. DO NOT USE.

#ifndef __FUELGAUGE_H
#define __FUELGAUGE_H

#include "msp430x26x.h"
#include "i2c.h"

//=========================== define ==========================================

// #define IR_PREP while(i2c_send(ir_str6, 1))
// #define IR_READ(var) while(i2c_recv(var, 13))

#define isr_fuelgauge_rx() isr_i2c_rx()

#define FUELGAUGE_REG_CNTL          0x00
#  define FUELGAUGE_CNTL_STATUS    {0x00, 0x00}
#  define FUELGAUGE_CNTL_TYPE      {0x00, 0x01}
#  define FUELGAUGE_CNTL_FWVER     {0x00, 0x02}
#  define FUELGAUGE_CNTL_SWVER     {0x00, 0x03}
#  define FUELGAUGE_CNTL_CHKSUM    {0x00, 0x04}
#  define FUELGAUGE_CNTL_RSTDATA   {0x00, 0x05}
#  define FUELGAUGE_CNTL_RSVD      {0x00, 0x06}

#define FUELGAUGE_REG_AR            0x02
#define FUELGAUGE_REG_ARTTE         0x04
#define FUELGAUGE_REG_TEMP          0x06
#define FUELGAUGE_REG_VOLT          0x08
#define FUELGAUGE_REG_FLAGS         0x0a
#define FUELGAUGE_REG_NAC           0x0c
#define FUELGAUGE_REG_FAC           0x0e
#define FUELGAUGE_REG_RM            0x10
#define FUELGAUGE_REG_FCC           0x12
#define FUELGAUGE_REG_AI            0x14
#define FUELGAUGE_REG_TTE           0x16
#define FUELGAUGE_REG_TTF           0x18
#define FUELGAUGE_REG_SI            0x1a
#define FUELGAUGE_REG_STTE          0x1c
#define FUELGAUGE_REG_MLI           0x1e
#define FUELGAUGE_REG_MLTTE         0x20
#define FUELGAUGE_REG_AE            0x22
#define FUELGAUGE_REG_AP            0x24
#define FUELGAUGE_REG_TTECP         0x26
#define FUELGAUGE_REG_RSVD          0x28
#define FUELGAUGE_REG_CC            0x2a
#define FUELGAUGE_REG_SOC           0x2c

//address
#define FUELGAUGE_I2C_ADDR          0x55

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void fuelgauge_init(void);
int  fuelgauge_test(void);
void fuelgauge_config(void);
int  fuelgauge_read(char);

#endif