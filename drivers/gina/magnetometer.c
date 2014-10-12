#include "opendefs.h"
#include "magnetometer.h"

//=========================== variables =======================================

typedef struct {
   uint8_t reg_CONF_A;       // register backup
   uint8_t reg_CONF_B;       // register backup
   uint8_t reg_MODE;         // register backup
   uint8_t reg_STATUS;       // register backup
   uint8_t reg_ID_A;         // register backup
   uint8_t reg_ID_B;         // register backup
   uint8_t reg_ID_C;         // register backup
   bool    configured;
} magnetometer_vars_t;

magnetometer_vars_t magnetometer_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void magnetometer_init() {
   uint8_t reg[]={MAGNETOMETER_REG_CONF_A_ADDR,MAGNETOMETER_REG_CONF_A_SETTING};
   
   magnetometer_vars.configured = FALSE;
   i2c_write_register(1,MAGNETOMETER_I2C_ADDR, sizeof(reg), reg);
   reg[0]=MAGNETOMETER_REG_CONF_B_ADDR;
   reg[1]=MAGNETOMETER_REG_CONF_B_SETTING;
   i2c_write_register(1,MAGNETOMETER_I2C_ADDR, sizeof(reg), reg);
 
   magnetometer_enable();
   magnetometer_vars.configured = TRUE;
}

void magnetometer_enable() {
   i2c_write_register(1,MAGNETOMETER_I2C_ADDR,
         MAGNETOMETER_REG_MODE_ADDR,
         MAGNETOMETER_REG_MODE_WAKEUP);
}

void magnetometer_disable() {
   uint8_t reg[]={MAGNETOMETER_REG_MODE_ADDR,MAGNETOMETER_REG_MODE_SLEEP};
   i2c_write_register(1,MAGNETOMETER_I2C_ADDR, sizeof(reg), reg);
}

void magnetometer_get_config() {
   if (magnetometer_vars.configured==TRUE) {
      i2c_read_registers(1,MAGNETOMETER_I2C_ADDR,MAGNETOMETER_REG_CONF_A_ADDR  ,1,&magnetometer_vars.reg_CONF_A);
      i2c_read_registers(1,MAGNETOMETER_I2C_ADDR,MAGNETOMETER_REG_CONF_B_ADDR  ,1,&magnetometer_vars.reg_CONF_B);
      i2c_read_registers(1,MAGNETOMETER_I2C_ADDR,MAGNETOMETER_REG_MODE_ADDR    ,1,&magnetometer_vars.reg_MODE);
      i2c_read_registers(1,MAGNETOMETER_I2C_ADDR,MAGNETOMETER_REG_STATUS_ADDR  ,1,&magnetometer_vars.reg_STATUS);
      i2c_read_registers(1,MAGNETOMETER_I2C_ADDR,MAGNETOMETER_REG_ID_A_ADDR    ,1,&magnetometer_vars.reg_ID_A);
      i2c_read_registers(1,MAGNETOMETER_I2C_ADDR,MAGNETOMETER_REG_ID_B_ADDR    ,1,&magnetometer_vars.reg_ID_B);
      i2c_read_registers(1,MAGNETOMETER_I2C_ADDR,MAGNETOMETER_REG_ID_C_ADDR    ,1,&magnetometer_vars.reg_ID_C);
   }
}

void magnetometer_get_measurement(uint8_t* spaceToWrite) {
   uint8_t i;
   if (magnetometer_vars.configured==TRUE) {
      i2c_read_registers(1,MAGNETOMETER_I2C_ADDR,MAGNETOMETER_REG_DATA_X_H_ADDR,6,spaceToWrite);
   } else {
      for (i=0;i<6;i++) {
         spaceToWrite[i] = 0;
      }
   }
}

//=========================== private ========================================