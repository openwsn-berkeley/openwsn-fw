/**
\brief CC2538-specific definition of the I2C bsp module.

\author Pere Tuset <peretuset@openmote.com>, September 2014.
*/

#include "i2c.h"
#include "ioc.h"
#include "sys_ctrl.h"

#include "hw_gpio.h"
#include "hw_i2cm.h"
#include "hw_ioc.h"
#include "hw_memmap.h"
#include "hw_sys_ctrl.h"
#include "hw_types.h"

#define I2C_PERIPHERAL          ( SYS_CTRL_PERIPH_I2C )
#define I2C_BASE                ( GPIO_B_BASE )
#define I2C_SCL                 ( GPIO_PIN_3 )
#define I2C_SDA                 ( GPIO_PIN_4 )
#define I2C_BAUDRATE            ( 100000 )

//=========================== variables =======================================


//=========================== prototypes ======================================


//=========================== public ==========================================

void i2c_init(void)
{
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

void i2c_read_byte(uint8_t address, uint8_t* buffer)
{
    I2CMasterSlaveAddrSet(address, true); // read

    I2CMasterControl(I2C_MASTER_CMD_SINGLE_RECEIVE);

    while (I2CMasterBusy())
        ;

    *buffer = I2CMasterDataGet();
}

void i2c_read_bytes(uint8_t address, uint8_t* buffer, uint32_t length)
{
    I2CMasterSlaveAddrSet(address, true); // read

    I2CMasterControl(I2C_MASTER_CMD_BURST_RECEIVE_START);

    while (length) {
        while (I2CMasterBusy())
            ;

        *buffer++ = I2CMasterDataGet();
        size--;

        if (size == 1)
        {
            I2CMasterControl(I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
        } else
        {
            I2CMasterControl(I2C_MASTER_CMD_BURST_RECEIVE_CONT);
        }
    }
}

void i2c_write_byte(uint8_t address, uint8_t register_)
{
    I2CMasterSlaveAddrSet(address, false); // write

    I2CMasterDataPut(register_);

    I2CMasterControl(I2C_MASTER_CMD_SINGLE_SEND);

    while (I2CMasterBusy())
        ;
}

void i2c_write_bytes(uint8_t address, uint8_t* buffer, uint32_t length)
{
    I2CMasterSlaveAddrSet(address, false); // write

    I2CMasterDataPut(*buffer++);
    length--;

    I2CMasterControl(I2C_MASTER_CMD_BURST_SEND_START);

    while (I2CMasterBusy())
        ;

    while (length) {
        I2CMasterDataPut(*buffer++);
        length--;

        if (size == 0)
        {
            I2CMasterControl(I2C_MASTER_CMD_BURST_SEND_FINISH);
        }
        else
        {
            I2CMasterControl(I2C_MASTER_CMD_BURST_SEND_CONT);
        }

        while (I2CMasterBusy())
            ;
    }
}

//=========================== private =========================================

