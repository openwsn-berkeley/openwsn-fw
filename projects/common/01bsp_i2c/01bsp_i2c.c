/**
\brief This is a program shows how to read data from i2c slave device.

\note: Since the bsp modules for different platforms have the same declaration,
       you can use this project with any platform.

This project reads the chipid through i2c interface with the device.

\author Tengfei Chang <tengfeichang@hkust-gz.edu.cn>, July 2023.
*/

#include "stdint.h"
#include "stdio.h"
// bsp modules required
#include "board.h"
#include "i2c.h"

//=========================== defines =========================================

#define I2C_DEV_ADDR                0x68
#define I2C_DEV_REG_ADDR_CHIPID     0x00

//=========================== variables =======================================

typedef struct {
   uint8_t    who_am_i;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {
   
    uint32_t  tmp;
    uint32_t  i;

    memset(&app_vars,0,sizeof(app_vars));
   
    // initialize
   
    board_init();

    // alway set address first
    i2c_set_addr(I2C_DEV_ADDR);
   
    // retrieve radio manufacturer ID over SPI
    while(1) {

        tmp = i2c_read_bytes(I2C_DEV_REG_ADDR_CHIPID, &app_vars.who_am_i, 1);
        for (i=0;i<0xfffff;i++);
   }
}

//=========================== callbacks =======================================