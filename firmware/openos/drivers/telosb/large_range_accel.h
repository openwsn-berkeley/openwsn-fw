#ifndef __LARGE_RANGE_ACCEL_H
#define __LARGE_RANGE_ACCEL_H

/**
\addtogroup drivers
\{
\addtogroup LargeRangeAccel
\{
*/

#include "msp430x26x.h"
#include "stdint.h"
#include "i2c.h"

//=========================== define ==========================================

//address
#define LARGE_RANGE_ACCEL_I2C_ADDR               0x18 // b0011000_

//register addresses
#define LARGE_RANGE_ACCEL_REG_XOUT_H_ADDR        0x00 // r
#define LARGE_RANGE_ACCEL_REG_XOUT_L_ADDR        0x01 // r
#define LARGE_RANGE_ACCEL_REG_YOUT_H_ADDR        0x02 // r
#define LARGE_RANGE_ACCEL_REG_YOUT_L_ADDR        0x03 // r
#define LARGE_RANGE_ACCEL_REG_ZOUT_H_ADDR        0x04 // r
#define LARGE_RANGE_ACCEL_REG_ZOUT_L_ADDR        0x05 // r
#define LARGE_RANGE_ACCEL_REG_AUXOUT_H_ADDR      0x06 // r
#define LARGE_RANGE_ACCEL_REG_AUXOUT_L_ADDR      0x07 // r
#define LARGE_RANGE_ACCEL_REG_RESET_WRITE_ADDR   0x0A //   w
#define LARGE_RANGE_ACCEL_REG_CTRL_REGC_ADDR     0x0C // r/w
#define LARGE_RANGE_ACCEL_REG_CTRL_REGB_ADDR     0x0D // r/w
#define LARGE_RANGE_ACCEL_REG_CTRL_REGA_ADDR     0x0E //   w

//register settings
#define LARGE_RANGE_ACCEL_REG_CTRL_REGC_SETTING  0xE0 // 0b111xxxxx: LP            : 50Hz bandwidth
                                                      // 0bxxx0xxxx: MOTLev level  : +/-6 g
                                                      // 0bxxxx0xxx: MOTLat        : non-latching wake up response
                                                      // 0bxxxxx0xx: unused
                                                      // 0bxxxxxx00: FS=full scale : +/-8g
                                                     
#define LARGE_RANGE_ACCEL_REG_CTRL_REGB_SETTING  0xC0 // 0b1xxxxxxx: CLKhld        : hold I2C SCL to wait for A/D conversion
                                                      // 0bx1xxxxxx: ENABLE        : enabled
                                                      // 0bxx0xxxxx: ST            : no self-test
                                                      // 0bxxx00xxx: unused
                                                      // 0bxxxxx0xx: MOTlen        : no motion wakeup feature
                                                      // 0bxxxxxx00: unused

#define LARGE_RANGE_ACCEL_REG_CTRL_REGB_SLEEP    0x80 // 0b1xxxxxxx: CLKhld        : hold I2C SCL to wait for A/D conversion
                                                      // 0bx0xxxxxx: ENABLE        : sleep
                                                      // 0bxx0xxxxx: ST            : no self-test
                                                      // 0bxxx00xxx: unused
                                                      // 0bxxxxx0xx: MOTlen        : no motion wakeup feature
                                                      // 0bxxxxxx00: unused

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void large_range_accel_init();
void large_range_accel_disable();
void large_range_accel_get_config();
void large_range_accel_get_measurement(uint8_t* spaceToWrite);

/**
\}
\}
*/

#endif
