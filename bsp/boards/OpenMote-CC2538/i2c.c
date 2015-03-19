/**
 * Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Date:   July 2013
 * Description:CC2538-specific definition of the "i2c" bsp module.
 */

#include "gpio.h"
#include "source/i2c.h"
#include "ioc.h"
#include "sys_ctrl.h"

#include "headers/hw_gpio.h"
#include "headers/hw_i2cm.h"
#include "headers/hw_ioc.h"
#include "headers/hw_memmap.h"
#include "headers/hw_sys_ctrl.h"
#include "headers/hw_types.h"

//=========================== define ==========================================

#define I2C_PERIPHERAL          ( SYS_CTRL_PERIPH_I2C )
#define I2C_BASE                ( GPIO_B_BASE )
#define I2C_SCL                 ( GPIO_PIN_3 )
#define I2C_SDA                 ( GPIO_PIN_4 )
#define I2C_BAUDRATE            ( 100000 )
#define I2C_MAX_DELAY_US        ( 100000 )

//=========================== variables =======================================


//=========================== prototypes ======================================

extern uint32_t board_timer_get(void);
extern bool board_timer_expired(uint32_t future);

//=========================== public ==========================================

void i2c_init(void) {
    bool status;
    
    // Enable peripheral except in deep sleep modes (e.g. LPM1, LPM2, LPM3)
    SysCtrlPeripheralEnable(I2C_PERIPHERAL);
    SysCtrlPeripheralSleepEnable(I2C_PERIPHERAL);
    SysCtrlPeripheralDeepSleepDisable(I2C_PERIPHERAL);

    // Reset peripheral previous to configuring it
    SysCtrlPeripheralReset(I2C_PERIPHERAL);

    // Configure the SCL pin
    GPIOPinTypeI2C(I2C_BASE, I2C_SCL);
    IOCPinConfigPeriphInput(I2C_BASE, I2C_SCL, IOC_I2CMSSCL);
    IOCPinConfigPeriphOutput(I2C_BASE, I2C_SCL, IOC_MUX_OUT_SEL_I2C_CMSSCL);

    // Configure the SDA pin
    GPIOPinTypeI2C(I2C_BASE, I2C_SDA);
    IOCPinConfigPeriphInput(I2C_BASE, I2C_SDA, IOC_I2CMSSDA);
    IOCPinConfigPeriphOutput(I2C_BASE, I2C_SDA, IOC_MUX_OUT_SEL_I2C_CMSSDA);

    // Configure the I2C clock
    status = (I2C_BAUDRATE == 400000 ? true : false);
    I2CMasterInitExpClk(SysCtrlClockGet(), status);

    // Enable the I2C module as master
    I2CMasterEnable();
}

bool i2c_read_byte(uint8_t address, uint8_t* byte) {
    uint32_t future = I2C_MAX_DELAY_US;
    
    // Receive operation
    I2CMasterSlaveAddrSet(address, true);

    // Single receive operation
    I2CMasterControl(I2C_MASTER_CMD_SINGLE_RECEIVE);

    // Calculate timeout
    future += board_timer_get();

    // Wait until complete or timeout
    while (I2CMasterBusy()) {
        // Update timeout status and return if expired
        if (board_timer_expired(future)) return false;
    }

    // Read data from I2C
    *byte = I2CMasterDataGet();
    
    // Return status
    return true;
}

uint32_t i2c_read_bytes(uint8_t address, uint8_t* buffer, uint32_t length) {
    uint32_t future = I2C_MAX_DELAY_US;
    
    // Receive operation
    I2CMasterSlaveAddrSet(address, true);

    // Multiple receive operation
    I2CMasterControl(I2C_MASTER_CMD_BURST_RECEIVE_START);

    // Calculate timeout
    future += board_timer_get();

    // Iterate overall all bytes
    while (length) {
        // Wait until complete or timeout
        while (I2CMasterBusy()) {
            // Update timeout status and return if expired
            if (board_timer_expired(future)) return length;
        }
        
        // Read data from I2C
        *buffer++ = I2CMasterDataGet();
        length--;

        // Check if it's the last byte
        if (length == 1) I2CMasterControl(I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
        else             I2CMasterControl(I2C_MASTER_CMD_BURST_RECEIVE_CONT);
    }
    
    // Return bytes read
    return length;
}

bool i2c_write_byte(uint8_t address, uint8_t byte) {   
    uint32_t future = I2C_MAX_DELAY_US;
    
    // Transmit operation
    I2CMasterSlaveAddrSet(address, false);

    // Write byte to I2C buffer
    I2CMasterDataPut(byte);

    // Single transmit operation
    I2CMasterControl(I2C_MASTER_CMD_SINGLE_SEND);
    
    // Calculate timeout
    future += board_timer_get();

    // Wait until complete or timeout
    while (I2CMasterBusy()) {
        // Check timeout status and return if expired
        if (board_timer_expired(future)) return false;
    }
    
    return true;
}

uint32_t i2c_write_bytes(uint8_t address, uint8_t* buffer, uint32_t length) {
    uint32_t future = I2C_MAX_DELAY_US;
    
    // Transmit operation
    I2CMasterSlaveAddrSet(address, false);

    // Write byte to I2C buffer
    I2CMasterDataPut(*buffer++);
    length--;

    // Multiple transmit operation
    I2CMasterControl(I2C_MASTER_CMD_BURST_SEND_START);

    // Calculate timeout
    future += board_timer_get();

    // Wait until complete or timeout
    while (I2CMasterBusy()) {
        // Check timeout status and return if expired
        if (board_timer_expired(future)) return length;
    }

    // Iterate overall all bytes
    while (length) {
        // Write byte to I2C buffer
        I2CMasterDataPut(*buffer++);

        // Check if it's the last byte
        if (length == 1) I2CMasterControl(I2C_MASTER_CMD_BURST_SEND_FINISH);
        else             I2CMasterControl(I2C_MASTER_CMD_BURST_SEND_CONT);

        // Wait until complete or timeout
        while (I2CMasterBusy()) {
            // Check timeout status and return if expired
            if (board_timer_expired(future)) return length;
        }

        // Update the length
        length--;
    }
    
    // Return bytes written
    return length;
}

//=========================== private =========================================

