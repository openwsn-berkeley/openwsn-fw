#ifndef __MAGNETOMETER_H
#define __MAGNETOMETER_H

/**
\addtogroup drivers
\{
\addtogroup Magnetometer
\{
*/

#include "msp430x26x.h"
#include "stdint.h"
#include "i2c.h"

//=========================== define ==========================================

//address
#define MAGNETOMETER_I2C_ADDR               0x1E //b0011000_

//register addresses
#define MAGNETOMETER_REG_CONF_A_ADDR        0x00 // r/w
#define MAGNETOMETER_REG_CONF_B_ADDR        0x01 // r/w
#define MAGNETOMETER_REG_MODE_ADDR          0x02 // r/w
#define MAGNETOMETER_REG_DATA_X_H_ADDR      0x03 // r
#define MAGNETOMETER_REG_DATA_X_L_ADDR      0x04 // r
#define MAGNETOMETER_REG_DATA_y_H_ADDR      0x05 // r
#define MAGNETOMETER_REG_DATA_Y_L_ADDR      0x06 // r
#define MAGNETOMETER_REG_DATA_Z_H_ADDR      0x07 // r
#define MAGNETOMETER_REG_DATA_Z_L_ADDR      0x08 // r
#define MAGNETOMETER_REG_STATUS_ADDR        0x09 // r
#define MAGNETOMETER_REG_ID_A_ADDR          0x0A // r
#define MAGNETOMETER_REG_ID_B_ADDR          0x0B // r
#define MAGNETOMETER_REG_ID_C_ADDR          0x0C // r

//register settings
#define MAGNETOMETER_REG_CONF_A_SETTING     0x18 // 0b000xxxxx: unused
                                                 // 0bxxx110xx: Data Output Rate = 50Hz
                                                 // 0bxxxxxx00: Normal measurement mode
                                                     
#define MAGNETOMETER_REG_CONF_B_SETTING     0x40 // 0b010xxxxx: Gain = 970 counts/Gauss (higher causes overload)
                                                 // 0bxxx00000: unused

#define MAGNETOMETER_REG_MODE_WAKEUP        0x00 // 0b000000xx: unused
                                                 // 0bxxxxxx00: continuous conversion mode

#define MAGNETOMETER_REG_MODE_SLEEP         0x03 // 0b000000xx: unused
                                                 // 0bxxxxxx11: sleep mode


//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void magnetometer_init();
void magnetometer_enable();
void magnetometer_disable();
void magnetometer_get_config();
void magnetometer_get_measurement(uint8_t* spaceToWrite);

/**
\}
\}
*/

#endif
