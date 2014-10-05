#include "opendefs.h"
#include "large_range_accel.h"

//=========================== variables =======================================

typedef struct {
   uint8_t reg_CTRL_REGC;    // register backup
   uint8_t reg_CTRL_REGB;    // register backup
   bool    configured;
} large_range_accel_vars_t;

large_range_accel_vars_t large_range_accel_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void large_range_accel_init() {
   uint8_t reg[]={LARGE_RANGE_ACCEL_REG_CTRL_REGC_ADDR,LARGE_RANGE_ACCEL_REG_CTRL_REGC_SETTING};
   large_range_accel_vars.configured = FALSE;
   P5OUT |=  0x10;                               // set P5.4 as output
   P5DIR |=  0x10;                               // set P5.4 high to enable I2C
   i2c_write_register(1,LARGE_RANGE_ACCEL_I2C_ADDR, sizeof(reg), reg);
   reg[0]=LARGE_RANGE_ACCEL_REG_CTRL_REGB_ADDR;
   reg[1]=LARGE_RANGE_ACCEL_REG_CTRL_REGB_SETTING;
   i2c_write_register(1,LARGE_RANGE_ACCEL_I2C_ADDR, sizeof(reg), reg);
   large_range_accel_vars.configured = TRUE;
}

void large_range_accel_disable() {
   uint8_t reg[]={LARGE_RANGE_ACCEL_REG_CTRL_REGB_ADDR,LARGE_RANGE_ACCEL_REG_CTRL_REGB_SLEEP};
   i2c_write_register(1,LARGE_RANGE_ACCEL_I2C_ADDR, sizeof(reg), reg);
}

void large_range_accel_get_config() {
   if (large_range_accel_vars.configured==TRUE) {
      i2c_read_registers(1,LARGE_RANGE_ACCEL_I2C_ADDR,
            LARGE_RANGE_ACCEL_REG_CTRL_REGC_ADDR,
            1,
            &large_range_accel_vars.reg_CTRL_REGC);
      i2c_read_registers(1,LARGE_RANGE_ACCEL_I2C_ADDR,
            LARGE_RANGE_ACCEL_REG_CTRL_REGB_ADDR,
            1,
            &large_range_accel_vars.reg_CTRL_REGB);
   }
}

void large_range_accel_get_measurement(uint8_t* spaceToWrite) {
   uint8_t i;
   if (large_range_accel_vars.configured==TRUE) {
      i2c_read_registers(1,LARGE_RANGE_ACCEL_I2C_ADDR,
            LARGE_RANGE_ACCEL_REG_XOUT_H_ADDR,
            6,
            spaceToWrite);
   } else {
      for (i=0;i<6;i++) {
         spaceToWrite[i] = 0;
      }
   }
}

//=========================== private =========================================