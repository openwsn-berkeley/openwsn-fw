#include "opendefs.h"
#include "gyro.h"

//=========================== variables =======================================

typedef struct {
   uint8_t reg_WHO_AM_I;     // register backup
   uint8_t reg_SMPLRT_DIV;   // register backup
   uint8_t reg_DLPF_FS;      // register backup
   uint8_t reg_INT_CFG;      // register backup
   uint8_t reg_INT_STATUS;   // register backup
   uint8_t reg_PWR_MGM;      // register backup
   bool    configured;
} gyro_vars_t;

gyro_vars_t gyro_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void gyro_init() {
   uint8_t reg[]={GYRO_REG_SMPLRT_DIV_ADDR,GYRO_REG_SMPLRT_DIV_SETTING};
   
   gyro_vars.configured = FALSE;
   
   i2c_write_register(1,GYRO_I2C_ADDR, sizeof(reg), reg);
   reg[0]=GYRO_REG_DLPF_FS_ADDR;
   reg[1]=GYRO_REG_DLPF_FS_SETTING;
   i2c_write_register(1,GYRO_I2C_ADDR, sizeof(reg), reg);
   reg[0]=GYRO_REG_INT_CFG_ADDR;
   reg[1]=GYRO_REG_INT_CFG_SETTING;
   i2c_write_register(1,GYRO_I2C_ADDR, sizeof(reg), reg);
   reg[0]=GYRO_REG_PWR_MGM_ADDR;
   reg[1]=GYRO_REG_PWR_MGM_SETTING;
   i2c_write_register(1,GYRO_I2C_ADDR, sizeof(reg), reg);
   gyro_vars.configured = TRUE;
}

void gyro_disable() {
   uint8_t reg[]={GYRO_REG_PWR_MGM_ADDR,GYRO_REG_PWR_MGM_SLEEP};
   i2c_write_register(1,GYRO_I2C_ADDR, sizeof(reg), reg);
}

void gyro_get_config() {
   if (gyro_vars.configured==TRUE) {
      i2c_read_registers(1,GYRO_I2C_ADDR, GYRO_REG_WHO_AM_I_ADDR   ,1,&gyro_vars.reg_WHO_AM_I);
      i2c_read_registers(1,GYRO_I2C_ADDR, GYRO_REG_SMPLRT_DIV_ADDR ,1,&gyro_vars.reg_SMPLRT_DIV);
      i2c_read_registers(1,GYRO_I2C_ADDR, GYRO_REG_DLPF_FS_ADDR    ,1,&gyro_vars.reg_DLPF_FS);
      i2c_read_registers(1,GYRO_I2C_ADDR, GYRO_REG_INT_CFG_ADDR    ,1,&gyro_vars.reg_INT_CFG);
      i2c_read_registers(1,GYRO_I2C_ADDR, GYRO_REG_INT_STATUS_ADDR ,1,&gyro_vars.reg_INT_STATUS);
      i2c_read_registers(1,GYRO_I2C_ADDR, GYRO_REG_PWR_MGM_ADDR    ,1,&gyro_vars.reg_PWR_MGM);
   }
}

void gyro_get_measurement(uint8_t* spaceToWrite) {
   uint8_t i;
   if (gyro_vars.configured==TRUE) {
      i2c_read_registers(1,GYRO_I2C_ADDR, GYRO_REG_TEMP_OUT_H_ADDR,8,spaceToWrite);
   } else {
      for (i=0;i<8;i++) {
         spaceToWrite[i] = 0;
      }
   }
}

//=========================== private =========================================