/**
 * Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Date:   Jan 2016
 * Description:EZR32WG-specific definition of the "i2c" bsp module.
 */

#include "toolchain_defs.h"
#include "board_info.h"


//=========================== define ==========================================


//=========================== variables =======================================


//=========================== prototypes ======================================

//=========================== public ==========================================

void i2c_init(void) {
}

bool i2c_read_byte(uint8_t address, uint8_t* byte) {
    return TRUE;
}

uint32_t i2c_read_bytes(uint8_t address, uint8_t* buffer, uint32_t length) {
        return 0;
}

bool i2c_write_byte(uint8_t address, uint8_t byte) {   

    return TRUE;
}

uint32_t i2c_write_bytes(uint8_t address, uint8_t* buffer, uint32_t length) {

    // Return bytes written
    return 0;
}

//=========================== private =========================================

