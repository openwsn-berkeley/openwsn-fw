/******************************************************************************
*  Filename:       i2c.c
*  Revised:        $Date: 2013-03-20 14:47:53 +0100 (Wed, 20 Mar 2013) $
*  Revision:       $Revision: 9489 $
*
*  Description:    Driver for Inter-IC (I2C) bus block.
*
*  Copyright (C) 2012 Texas Instruments Incorporated - http://www.ti.com/
*
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*    Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************/

//*****************************************************************************
//
//! \addtogroup i2c_api
//! @{
//
//*****************************************************************************

#include "hw_i2cm.h"
#include "hw_i2cs.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_sys_ctrl.h"
#include "debug.h"
#include "i2c.h"
#include "interrupt.h"

//*****************************************************************************
//
//! Initializes the I2C master block
//!
//! \param ui32I2CClk is the rate of the clock supplied to the I2C module.
//! \param bFast set up for fast data transfers
//!
//! This function initializes operation of the I2C master block.  Upon
//! successful initialization of the I2C block, this functionhas set the
//! bus speed for the master, and has enabled the I2C master block.
//!
//! If the parameter \e bFast is \b true, then the master block will be set up
//! to transfer data at 400 kbps; otherwise, it will be set up to transfer data
//! at 100 kbps.
//!
//! The peripheral clock will be the same as the processor clock.  This will be
//! the value returned by SysCtrlClockGet(), or it can be explicitly hardcoded
//! if it is constant and known (to save the code/execution overhead of a call
//! to SysCtrlClockGet()).
//!
//! \return None
//
//*****************************************************************************
void
I2CMasterInitExpClk(uint32_t ui32I2CClk, bool bFast)
{
    uint32_t ui32SCLFreq;
    uint32_t ui32TPR;

    //
    // Must enable the device before doing anything else.
    //
    I2CMasterEnable();

    //
    // Get the desired SCL speed.
    //
    if(bFast == true)
    {
        ui32SCLFreq = 400000;
    }
    else
    {
        ui32SCLFreq = 100000;
    }

    //
    // Compute the clock divider that achieves the fastest speed less than or
    // equal to the desired speed.  The numerator is biased to favor a larger
    // clock divider so that the resulting clock is always less than or equal
    // to the desired clock, never greater.
    //
    ui32TPR = ((ui32I2CClk + (2 * 10 * ui32SCLFreq) - 1) / (2 * 10 * ui32SCLFreq)) - 1;
    HWREG(I2CM_TPR) = ui32TPR;
}

//*****************************************************************************
//
//! Initializes the I2C slave block
//!
//! \param ui8SlaveAddr 7-bit slave address
//!
//! This function initializes operation of the I2C slave block.  Upon
//! successful initialization of the I2C blocks, this function has set
//! the slave address has enabled the I2C slave block.
//!
//! The parameter \e ui8SlaveAddr is the value that will be compared against the
//! slave address sent by an I2C master.
//!
//! \return None
//
//*****************************************************************************
void
I2CSlaveInit(uint8_t ui8SlaveAddr)
{
    //
    // Check the arguments.
    //
    ASSERT(!(ui8SlaveAddr & 0x80));

    //
    // Must enable the device before doing anything else.
    //
    I2CSlaveEnable();

    //
    // Set up the slave address.
    //
    HWREG(I2CS_OAR) = ui8SlaveAddr;
}

//*****************************************************************************
//
//! Enables the I2C Master block
//!
//! This function will enable operation of the I2C Master block.
//!
//! \return None
//
//*****************************************************************************
void
I2CMasterEnable(void)
{
    //
    // Enable the master block.
    //
    HWREG(I2CM_CR) |= I2CM_CR_MFE;
}

//*****************************************************************************
//
//! Enables the I2C slave block
//!
//! This function enables operation of the I2C slave block.
//!
//! \return None
//
//*****************************************************************************
void
I2CSlaveEnable(void)
{
    //
    // Enable the clock to the slave block.
    //
    HWREG(I2CM_CR) |= I2CM_CR_SFE;

    //
    // Enable the slave.
    //
    HWREG(I2CS_CTRL) = I2CS_CTRL_DA;
}

//*****************************************************************************
//
//! Disables the I2C master block
//!
//! This function disables operation of the I2C master block.
//!
//! \return None
//
//*****************************************************************************
void
I2CMasterDisable(void)
{
    //
    // Disable the master block.
    //
    HWREG(I2CM_CR) &= ~(I2CM_CR_MFE);
}

//*****************************************************************************
//
//! Disables the I2C slave block
//!
//! This function disables operation of the I2C slave block.
//!
//! \return None
//
//*****************************************************************************
void
I2CSlaveDisable(void)
{
    //
    // Disable the slave.
    //
    HWREG(I2CS_CTRL) = 0;

    //
    // Disable the clock to the slave block.
    //
    HWREG(I2CM_CR) &= ~(I2CM_CR_SFE);
}

//*****************************************************************************
//
//! Registers an interrupt handler for the I2C module
//!
//! \param pfnHandler is a pointer to the function to be called when the
//! I2C interrupt occurs.
//!
//! This function sets the handler to be called when an I2C interrupt occurs.
//! This function enables the global interrupt in the interrupt controller;
//! specific I2C interrupts must be enabled through I2CMasterIntEnable() and
//! I2CSlaveIntEnable().  If necessary, the interrupt handler must clear
//! the interrupt source through I2CMasterIntClear() and I2CSlaveIntClear().
//!
//! \sa See IntRegister() for important information about registering interrupt
//! handlers.
//!
//! \return None
//
//*****************************************************************************
void
I2CIntRegister(void (*pfnHandler)(void))
{
    //
    // Register the interrupt handler, returning an error if an error occurs.
    //
    IntRegister(INT_I2C0, pfnHandler);

    //
    // Enable the I2C interrupt.
    //
    IntEnable(INT_I2C0);
}

//*****************************************************************************
//
//! Unregisters an interrupt handler for the I2C module
//!
//! This function clears the handler to be called when an I2C interrupt
//! occurs.  The function also masks off the interrupt in the interrupt
//! controller so that the interrupt handler no longer is called.
//!
//! \sa See IntRegister() for important information about registering interrupt
//! handlers.
//!
//! \return None
//
//*****************************************************************************
void
I2CIntUnregister(void)
{
    //
    // Disable the interrupt.
    //
    IntDisable(INT_I2C0);

    //
    // Unregister the interrupt handler.
    //
    IntUnregister(INT_I2C0);
}

//*****************************************************************************
//
//! Enables the I2C Master interrupt
//!
//! This function enables the I2C Master interrupt source.
//!
//! \return None
//
//*****************************************************************************
void
I2CMasterIntEnable(void)
{
    //
    // Enable the master interrupt.
    //
    HWREG(I2CM_IMR) = I2CM_IMR_IM;
}

//*****************************************************************************
//
//! Enables the I2C Slave interrupt
//!
//! This function enables the I2C Slave interrupt source.
//!
//! \return None
//
//*****************************************************************************
void
I2CSlaveIntEnable(void)
{
    //
    // Enable the slave interrupt.
    //
    HWREG(I2CS_IMR) |= I2C_SLAVE_INT_DATA;
}

//*****************************************************************************
//
//! Enables individual I2C slave interrupt sources
//!
//! \param ui32IntFlags is the bit mask of the interrupt sources to be enabled.
//!
//! This function enables the indicated I2C slave interrupt sources.  Only the
//! sources that are enabled can be reflected to the processor interrupt;
//! disabled sources have no effect on the processor.
//!
//! The \e ui32IntFlags parameter is the logical OR of any of the following:
//!
//! - \b I2C_SLAVE_INT_STOP   Stop condition detected interrupt
//! - \b I2C_SLAVE_INT_START  Start condition detected interrupt
//! - \b I2C_SLAVE_INT_DATA   Data interrupt
//!
//! \return None
//
//*****************************************************************************
void
I2CSlaveIntEnableEx(uint32_t ui32IntFlags)
{
    //
    // Enable the slave interrupt.
    //
    HWREG(I2CS_IMR) |= ui32IntFlags;
}

//*****************************************************************************
//
//! Disables the I2C master interrupt
//!
//! This function disables the I2C master interrupt source.
//!
//! \return None
//
//*****************************************************************************
void
I2CMasterIntDisable(void)
{
    //
    // Disable the master interrupt.
    //
    HWREG(I2CM_IMR) = 0;
}

//*****************************************************************************
//
//! Disables the I2C Slave interrupt
//!
//! This function disables the I2C Slave interrupt source
//!
//! \return None
//
//*****************************************************************************
void
I2CSlaveIntDisable(void)
{
    //
    // Disable the slave interrupt.
    //
    HWREG(I2CS_IMR) &= ~I2C_SLAVE_INT_DATA;
}

//*****************************************************************************
//
//! Disables individual I2C slave interrupt sources
//!
//! \param ui32IntFlags is the bit mask of the interrupt sources to be disabled.
//!
//! This function disables the indicated I2C slave interrupt sources.
//! Only the sources that are enabled can be reflected to the processor
//! interrupt; disabled sources have no effect on the processor.
//!
//! The \e ui32IntFlags parameter has the same definition as the \e ui32IntFlags
//! parameter to I2CSlaveIntEnableEx().
//!
//! \return None
//
//*****************************************************************************
void
I2CSlaveIntDisableEx(uint32_t ui32IntFlags)
{
    //
    // Disable the slave interrupt.
    //
    HWREG(I2CS_IMR) &= ~ui32IntFlags;
}

//*****************************************************************************
//
//! Gets the current I2C master interrupt status
//!
//! \param bMasked is false if the raw interrupt status is requested and
//! true if the masked interrupt status is requested.
//!
//! This function returns the interrupt status for the I2C master module.
//! Either the raw interrupt status or the status of interrupts that are allowed
//! to reflect to the processor can be returned.
//!
//! \return Returns the current interrupt status, returned as \b true if active
//! or \b false if not active.
//
//*****************************************************************************
bool
I2CMasterIntStatus(bool bMasked)
{
    //
    // Return either the interrupt status or the raw interrupt status as
    // requested.
    //
    if(bMasked)
    {
        return((HWREG(I2CM_MIS)) ? true : false);
    }
    else
    {
        return((HWREG(I2CM_RIS)) ? true : false);
    }
}

//*****************************************************************************
//
//! Gets the current I2C slave interrupt status
//!
//! \param bMasked is false if the raw interrupt status is requested and
//! true if the masked interrupt status is requested.
//!
//! This function returns the interrupt status for the I2C slave module.
//! Either the raw interrupt status or the status of interrupts that are
//! allowed to reflect to the processor can be returned.
//!
//! \return Returns the current interrupt status, returned as \b true if active
//! or \b false if not active.
//
//*****************************************************************************
bool
I2CSlaveIntStatus(bool bMasked)
{
    //
    // Return either the interrupt status or the raw interrupt status as
    // requested.
    //
    if(bMasked)
    {
        return((HWREG(I2CS_MIS)) ? true : false);
    }
    else
    {
        return((HWREG(I2CS_RIS)) ? true : false);
    }
}

//*****************************************************************************
//
//! Gets the current I2C slave interrupt status
//!
//! \param bMasked is false if the raw interrupt status is requested and
//! true if the masked interrupt status is requested.
//!
//! This function returns the interrupt status for the I2C slave module.  Either
//! the raw interrupt status or the status of interrupts that are allowed to
//! reflect to the processor can be returned.
//!
//! \return Returns the current interrupt status, enumerated as a bit field of
//! values described in I2CSlaveIntEnableEx().
//
//*****************************************************************************
uint32_t
I2CSlaveIntStatusEx(bool bMasked)
{
    //
    // Return either the interrupt status or the raw interrupt status as
    // requested.
    //
    if(bMasked)
    {
        return(HWREG(I2CS_MIS));
    }
    else
    {
        return(HWREG(I2CS_RIS));
    }
}

//*****************************************************************************
//
//! Clears I2C master interrupt sources
//!
//! This function clears the I2C master interrupt source, so that it no longer
//! asserts. This must be done in the interrupt handler to keep it from being
//! called again immediately upon exit.
//!
//! \note Because there is a write buffer in the Cortex-M3 processor, it may
//! take several clock cycles before the interrupt source is actually cleared.
//! Therefore, it is recommended that the interrupt source be cleared early in
//! the interrupt handler (as opposed to the very last action) to avoid
//! returning from the interrupt handler before the interrupt source is
//! actually cleared.  Failure to do so may result in the interrupt handler
//! being immediately reentered (because the interrupt controller still sees
//! the interrupt source asserted).
//!
//! \return None
//
//*****************************************************************************
void
I2CMasterIntClear(void)
{
    //
    // Clear the I2C master interrupt source.
    //
    HWREG(I2CM_ICR) = I2CM_ICR_IC;

    // This might not be needed. It was used on previous revisions of the IP
    HWREG(I2CM_MIS) = I2CM_ICR_IC;
}

//*****************************************************************************
//
//! Clears I2C slave interrupt sources
//!
//! This function clears the I2C slave interrupt source, so that it no longer
//! asserts. This must be done in the interrupt handler to keep it from being
//! recalled immediately upon exit.
//!
//! \note Because there is a write buffer in the Cortex-M3 processor, it may
//! take several clock cycles before the interrupt source is actually cleared.
//! Therefore, it is recommended that the interrupt source be cleared early in
//! the interrupt handler (as opposed to the very last action) to avoid
//! returning from the interrupt handler before the interrupt source is
//! actually cleared.  Failure to do so may result in the interrupt handler
//! being immediately reentered (because the interrupt controller still sees
//! the interrupt source asserted).
//!
//! \return None
//
//*****************************************************************************
void
I2CSlaveIntClear(void)
{
    //
    // Clear the I2C slave interrupt source.
    //
    HWREG(I2CS_ICR) = I2CS_ICR_DATAIC;
}

//*****************************************************************************
//
//! Clears the I2C slave interrupt sources
//!
//! \param ui32IntFlags is a bit mask of the interrupt sources to be cleared.
//!
//! This function clears the specified I2C Slave interrupt sources, so that they
//! no longer assert.  This must be done in the interrupt handler to keep it from
//! being called again immediately upon exit.
//!
//! The \e ui32IntFlags parameter has the same definition as the \e ui32IntFlags
//! parameter to I2CSlaveIntEnableEx().
//!
//! \note Because there is a write buffer in the Cortex-M3 processor, it may
//! take several clock cycles before the interrupt source is actually cleared.
//! Therefore, it is recommended that the interrupt source be cleared early in
//! the interrupt handler (as opposed to the very last action) to avoid
//! returning from the interrupt handler before the interrupt source is
//! actually cleared.  Failure to do so may result in the interrupt handler
//! being immediately reentered (because the interrupt controller still sees
//! the interrupt source asserted).
//!
//! \return None
//
//*****************************************************************************
void
I2CSlaveIntClearEx(uint32_t ui32IntFlags)
{
    //
    // Clear the I2C slave interrupt source.
    //
    HWREG(I2CS_ICR) = ui32IntFlags;
}

//*****************************************************************************
//
//! Sets the address that the I2C master places on the bus
//!
//! \param ui8SlaveAddr 7-bit slave address
//! \param bReceive flag indicating the type of communication with the slave
//!
//! This function sets the address that the I2C master places on the bus
//! when initiating a transaction. When the \e bReceive parameter is set
//! to \b true, the address indicates that the I2C master is initiating a
//! read from the slave; otherwise, the address indicates that the I2C
//! master is initiating a write to the slave.
//!
//! \return None
//
//*****************************************************************************
void
I2CMasterSlaveAddrSet(uint8_t ui8SlaveAddr, bool bReceive)
{
    //
    // Check the arguments.
    //
    ASSERT(!(ui8SlaveAddr & 0x80));

    //
    // Set the address of the slave with which the master will communicate.
    //
    HWREG(I2CM_SA) = (ui8SlaveAddr << 1) | bReceive;
}

//*****************************************************************************
//
//! Indicates whether or not the I2C master is busy
//!
//! This function returns an indication of whether or not the I2C master is
//! busy transmitting or receiving data.
//!
//! \return Returns \b true if the I2C master is busy; otherwise, returns
//! \b false
//
//*****************************************************************************
bool
I2CMasterBusy(void)
{
    //
    // Return the busy status.
    //
    if(HWREG(I2CM_STAT) & I2CM_STAT_BUSY)
    {
        return(true);
    }
    else
    {
        return(false);
    }
}

//*****************************************************************************
//
//! Indicates whether or not the I2C bus is busy
//!
//! This function returns an indication of whether or not the I2C bus is busy.
//! This function can be used in a multimaster environment to determine if
//! another master is currently using the bus.
//!
//! \return Returns \b true if the I2C bus is busy; otherwise, returns
//! \b false
//
//*****************************************************************************
bool
I2CMasterBusBusy(void)
{
    //
    // Return the bus busy status.
    //
    if(HWREG(I2CM_STAT) & I2CM_STAT_BUSBSY)
    {
        return(true);
    }
    else
    {
        return(false);
    }
}

//*****************************************************************************
//
//! Controls the state of the I2C master module
//!
//! \param ui32Cmd command to be issued to the I2C master module
//!
//! This function is used to control the state of the master module send and
//! receive operations.  The \e ui32Cmd parameter can be one of the following
//! values:
//!
//! - \b I2C_MASTER_CMD_SINGLE_SEND
//! - \b I2C_MASTER_CMD_SINGLE_RECEIVE
//! - \b I2C_MASTER_CMD_BURST_SEND_START
//! - \b I2C_MASTER_CMD_BURST_SEND_CONT
//! - \b I2C_MASTER_CMD_BURST_SEND_FINISH
//! - \b I2C_MASTER_CMD_BURST_SEND_ERROR_STOP
//! - \b I2C_MASTER_CMD_BURST_RECEIVE_START
//! - \b I2C_MASTER_CMD_BURST_RECEIVE_CONT
//! - \b I2C_MASTER_CMD_BURST_RECEIVE_FINISH
//! - \b I2C_MASTER_CMD_BURST_RECEIVE_ERROR_STOP
//!
//! \return None
//
//*****************************************************************************
void
I2CMasterControl(uint32_t ui32Cmd)
{
    //
    // Check the arguments.
    //
    ASSERT((ui32Cmd == I2C_MASTER_CMD_SINGLE_SEND) ||
           (ui32Cmd == I2C_MASTER_CMD_SINGLE_RECEIVE) ||
           (ui32Cmd == I2C_MASTER_CMD_BURST_SEND_START) ||
           (ui32Cmd == I2C_MASTER_CMD_BURST_SEND_CONT) ||
           (ui32Cmd == I2C_MASTER_CMD_BURST_SEND_FINISH) ||
           (ui32Cmd == I2C_MASTER_CMD_BURST_SEND_ERROR_STOP) ||
           (ui32Cmd == I2C_MASTER_CMD_BURST_RECEIVE_START) ||
           (ui32Cmd == I2C_MASTER_CMD_BURST_RECEIVE_CONT) ||
           (ui32Cmd == I2C_MASTER_CMD_BURST_RECEIVE_FINISH) ||
           (ui32Cmd == I2C_MASTER_CMD_BURST_RECEIVE_ERROR_STOP));

    //
    // Send the command.
    //
    HWREG(I2CM_CTRL) = ui32Cmd;
}

//*****************************************************************************
//
//! Gets the error status of the I2C master module
//!
//! This function is obtains the error status of the master module send
//! and receive operations.
//!
//! \return Returns the error status as one of the following values:
//!
//! - \b I2C_MASTER_ERR_NONE
//! - \b I2C_MASTER_ERR_ADDR_ACK
//! - \b I2C_MASTER_ERR_DATA_ACK
//! - \b I2C_MASTER_ERR_ARB_LOST
//
//*****************************************************************************
uint32_t
I2CMasterErr(void)
{
    uint32_t ui32Err;

    //
    // Get the raw error state
    //
    ui32Err = HWREG(I2CM_STAT);

    //
    // If the I2C master is busy, then all the other bit are invalid, and
    // don't have an error to report.
    //
    if(ui32Err & I2CM_STAT_BUSY)
    {
        return(I2C_MASTER_ERR_NONE);
    }

    //
    // Check for errors.
    //
    if(ui32Err & (I2CM_STAT_ERROR | I2CM_STAT_ARBLST))
    {
        return(ui32Err &
               (I2CM_STAT_ARBLST | I2CM_STAT_DATACK | I2CM_STAT_ADRACK));
    }
    else
    {
        return(I2C_MASTER_ERR_NONE);
    }
}

//*****************************************************************************
//
//! Transmits a byte from the I2C master
//!
//! \param ui8Data data to be transmitted from the I2C master
//!
//! This function places the supplied data into I2C master data register.
//!
//! \return None
//
//*****************************************************************************
void
I2CMasterDataPut(uint8_t ui8Data)
{
    //
    // Write the byte.
    //
    HWREG(I2CM_DR) = ui8Data;
}

//*****************************************************************************
//
//! Receives a byte that has been sent to the I2C master
//!
//! This function reads a byte of data from the I2C master data register.
//!
//! \return Returns the byte received from by the I2C master, cast as an
//! uint32_t
//
//*****************************************************************************
uint32_t
I2CMasterDataGet(void)
{
    //
    // Read a byte.
    //
    return(HWREG(I2CM_DR));
}

//*****************************************************************************
//
//! Gets the I2C slave module status
//!
//! This function returns the action requested from a master, if any.
//! Possible values are:
//!
//! - \b I2C_SLAVE_ACT_NONE
//! - \b I2C_SLAVE_ACT_RREQ
//! - \b I2C_SLAVE_ACT_TREQ
//! - \b I2C_SLAVE_ACT_RREQ_FBR
//!
//! \return Returns \b I2C_SLAVE_ACT_NONE to indicate that no action has been
//! requested of the I2C slave module, \b I2C_SLAVE_ACT_RREQ to indicate that
//! an I2C master has sent data to the I2C slave module, \b I2C_SLAVE_ACT_TREQ
//! to indicate that an I2C master has requested that the I2C slave module send
//! data, and \b I2C_SLAVE_ACT_RREQ_FBR to indicate that an I2C master has sent
//! data to the I2C slave and the first byte following the address of the slave
//! has been received.
//
//*****************************************************************************
uint32_t
I2CSlaveStatus(void)
{
    //
    // Return the slave status.
    //
    return(HWREG(I2CS_STAT));
}

//*****************************************************************************
//
//! Transmits a byte from the I2C slave
//!
//! \param ui8Data data to be transmitted from the I2C slave
//!
//! This function places the supplied data into I2C slave data register.
//!
//! \return None
//
//*****************************************************************************
void
I2CSlaveDataPut(uint8_t ui8Data)
{
    //
    // Write the byte.
    //
    HWREG(I2CS_DR) = ui8Data;
}

//*****************************************************************************
//
//! Receives a byte that has been sent to the I2C slave
//!
//! This function reads a byte of data from the I2C slave data register.
//!
//! \return Returns the byte received from by the I2C slave, cast as an
//! uint32_t.
//
//*****************************************************************************
uint32_t
I2CSlaveDataGet(void)
{
    //
    // Read a byte.
    //
    return(HWREG(I2CS_DR));
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
