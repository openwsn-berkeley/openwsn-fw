#ifndef __GYRO_H
#define __GYRO_H

/**
\addtogroup drivers
\{
\addtogroup Gyro
\{
*/

#include "msp430x26x.h"
#include "stdint.h"
#include "i2c.h"

//=========================== define ==========================================

//address
#define GYRO_I2C_ADDR                  0x68 //b1101000_

//register addresses
#define GYRO_REG_WHO_AM_I_ADDR         0x00
#define GYRO_REG_SMPLRT_DIV_ADDR       0x15
#define GYRO_REG_DLPF_FS_ADDR          0x16
#define GYRO_REG_INT_CFG_ADDR          0x17
#define GYRO_REG_INT_STATUS_ADDR       0x1A
#define GYRO_REG_TEMP_OUT_H_ADDR       0x1B
#define GYRO_REG_TEMP_OUT_L_ADDR       0x1C
#define GYRO_REG_GYRO_XOUT_H_ADDR      0x1D
#define GYRO_REG_GYRO_XOUT_L_ADDR      0x1E
#define GYRO_REG_GYRO_YOUT_H_ADDR      0x1F
#define GYRO_REG_GYRO_YOUT_L_ADDR      0x20
#define GYRO_REG_GYRO_ZOUT_H_ADDR      0x21
#define GYRO_REG_GYRO_ZOUT_L_ADDR      0x22
#define GYRO_REG_PWR_MGM_ADDR          0x3E

//register settings
#define GYRO_REG_SMPLRT_DIV_SETTING    0x01 //Sample Rate Divider set to 1
#define GYRO_REG_DLPF_FS_SETTING       0x1E //full-scale range: +/-2000°/sec; digital low pass filter: 6
#define GYRO_REG_INT_CFG_SETTING       0x00 //no interrupts used
#define GYRO_REG_PWR_MGM_SETTING       0x00 //no low-power modes used
#define GYRO_REG_PWR_MGM_SLEEP         0x40 //sleep mode

//=========================== variables =======================================

//=========================== prototypes ======================================

void gyro_init();
void gyro_disable();
void gyro_get_config();
void gyro_get_measurement(uint8_t* spaceToWrite);

/**
\}
\}
*/

#endif
