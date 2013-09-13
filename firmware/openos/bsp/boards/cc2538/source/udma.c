/******************************************************************************
*  Filename:       udma.c
*  Revised:        $Date: 2013-03-20 14:47:53 +0100 (Wed, 20 Mar 2013) $
*  Revision:       $Revision: 9489 $
*
*  Description:    Driver for the micro-DMA controller.
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
//! \addtogroup udma_api
//! @{
//
//*****************************************************************************

#include "hw_sys_ctrl.h"
#include "hw_udma.h"
#include "hw_udmachctl.h"
#include "debug.h"
#include "interrupt.h"
#include "udma.h"

//*****************************************************************************
//
//! Enables the uDMA controller for use
//!
//! This function enables the uDMA controller.  The uDMA controller must be
//! enabled before it can be configured and used.
//!
//! \return None
//
//*****************************************************************************
void
uDMAEnable(void)
{
    //
    // Set the master enable bit in the config register.
    //
    HWREG(UDMA_CFG) = UDMA_CFG_MASTEN;
}

//*****************************************************************************
//
//! Disables the uDMA controller for use
//!
//! This function disables the uDMA controller.  Once disabled, the uDMA
//! controller will not operate until re-enabled with uDMAEnable().
//!
//! \return None
//
//*****************************************************************************
void
uDMADisable(void)
{
    //
    // Clear the master enable bit in the config register.
    //
    HWREG(UDMA_CFG) = 0;
}

//*****************************************************************************
//
//! Gets the uDMA error status
//!
//! This function returns the uDMA error status.  It should be called from
//! within the uDMA error interrupt handler to determine if a uDMA error
//! occurred.
//!
//! \return Returns non-zero if a uDMA error is pending.
//
//*****************************************************************************
uint32_t
uDMAErrorStatusGet(void)
{
    //
    // Return the uDMA error status.
    //
    return(HWREG(UDMA_ERRCLR));
}

//*****************************************************************************
//
//! Clears the uDMA error interrupt
//!
//! This function clears a pending uDMA error interrupt.  It should be called
//! from within the uDMA error interrupt handler to clear the interrupt.
//!
//! \return None
//
//*****************************************************************************
void
uDMAErrorStatusClear(void)
{
    //
    // Clear the uDMA error interrupt.
    //
    HWREG(UDMA_ERRCLR) = 1;
}

//*****************************************************************************
//
//! Enables a uDMA channel for operation
//!
//! \param ui32ChannelNum is the channel number to enable.
//!
//! This function enables a specific uDMA channel for use.  This function must
//! be used to enable a channel before it can be used to perform a uDMA
//! transfer.
//!
//! When a uDMA transfer is completed, the channel will be automatically
//! disabled by the uDMA controller.  Therefore, this function should be called
//! prior to starting up any new transfer.
//!
//! \return None
//
//*****************************************************************************
void
uDMAChannelEnable(uint32_t ui32ChannelNum)
{
    //
    // Check the arguments.
    //
    ASSERT((ui32ChannelNum & 0xffff) < 32);

    //
    // Set the bit for this channel in the enable set register.
    //
    HWREG(UDMA_ENASET) = 1 << (ui32ChannelNum & 0x1f);
}

//*****************************************************************************
//
//! Disables a uDMA channel for operation
//!
//! \param ui32ChannelNum is the channel number to disable.
//!
//! This function disables a specific uDMA channel.  Once disabled, a channel
//! will not respond to uDMA transfer requests until re-enabled via
//! uDMAChannelEnable().
//!
//! \return None
//
//*****************************************************************************
void
uDMAChannelDisable(uint32_t ui32ChannelNum)
{
    //
    // Check the arguments.
    //
    ASSERT((ui32ChannelNum & 0xffff) < 32);

    //
    // Set the bit for this channel in the enable clear register.
    //
    HWREG(UDMA_ENACLR) = 1 << (ui32ChannelNum & 0x1f);
}

//*****************************************************************************
//
//! Checks if a uDMA channel is enabled for operation
//!
//! \param ui32ChannelNum is the channel number to check.
//!
//! This function checks to see if a specific uDMA channel is enabled.  This
//! can be used to check the status of a transfer, since the channel will
//! be automatically disabled at the end of a transfer.
//!
//! \return Returns \b true if the channel is enabled, \b false if disabled.
//
//*****************************************************************************
bool
uDMAChannelIsEnabled(uint32_t ui32ChannelNum)
{
    //
    // Check the arguments.
    //
    ASSERT((ui32ChannelNum & 0xffff) < 32);

    //
    // AND the specified channel bit with the enable register, and return the
    // result.
    //

    return((HWREG(UDMA_ENASET) & (1 << (ui32ChannelNum & 0x1f))) ? true : false);
}

//*****************************************************************************
//
//! Sets the base address for the channel control table
//!
//! \param pControlTable is a pointer to the 1024 byte aligned base address
//! of the uDMA channel control table.
//!
//! This function sets the base address of the channel control table.  This
//! table resides in system memory and holds control information for each uDMA
//! channel.  The table must be aligned on a 1024 byte boundary.  The base
//! address must be set before any of the channel functions can be used.
//!
//! The size of the channel control table depends on the number of uDMA
//! channels, and which transfer modes are used.  Refer to the introductory
//! text and the microcontroller datasheet for more information about the
//! channel control table.
//!
//! \return None
//
//*****************************************************************************
void
uDMAControlBaseSet(void *pControlTable)
{
    //
    // Check the arguments.
    //
    ASSERT(((uint32_t)pControlTable & ~0x3FF) ==
           (uint32_t)pControlTable);
    ASSERT((uint32_t)pControlTable >= 0x20000000);

    //
    // Program the base address into the register.
    //
    HWREG(UDMA_CTLBASE) = (uint32_t)pControlTable;
}

//*****************************************************************************
//
//! Gets the base address for the channel control table
//!
//! This function gets the base address of the channel control table.  This
//! table resides in system memory and holds control information for each uDMA
//! channel.
//!
//! \return Returns a pointer to the base address of the channel control table.
//
//*****************************************************************************
void *
uDMAControlBaseGet(void)
{
    //
    // Read the current value of the control base register, and return it to
    // the caller.
    //
    return((void *)HWREG(UDMA_CTLBASE));
}

//*****************************************************************************
//
//! Gets the base address for the channel control table alternate structures
//!
//! This function gets the base address of the second half of the channel
//! control table that holds the alternate control structures for each channel.
//!
//! \return Returns a pointer to the base address of the second half of the
//! channel control table.
//
//*****************************************************************************
void *
uDMAControlAlternateBaseGet(void)
{
    //
    // Read the current value of the control base register, and return it to
    // the caller.
    //
    return((void *)HWREG(UDMA_ALTBASE));
}

//*****************************************************************************
//
//! Requests a uDMA channel to start a transfer
//!
//! \param ui32ChannelNum is the channel number on which to request a uDMA
//! transfer.
//!
//! This function allows software to request a uDMA channel to begin a
//! transfer.  This could be used for performing a memory to memory transfer,
//! or if for some reason a transfer needs to be initiated by software instead
//! of the peripheral associated with that channel.
//!
//! \note If the channel is \b UDMA_CH30_SW and interrupts
//! are used, then the completion will be signaled on the uDMA dedicated
//! interrupt.
//! If a peripheral channel is used, then the completion will be signaled on the
//! peripheral's interrupt.
//!
//! \return None
//
//*****************************************************************************
void
uDMAChannelRequest(uint32_t ui32ChannelNum)
{
    //
    // Check the arguments.
    //
    ASSERT((ui32ChannelNum & 0xffff) < 32);

    //
    // Set the bit for this channel in the software uDMA request register.
    //
    HWREG(UDMA_SWREQ) = 1 << (ui32ChannelNum & 0x1f);
}

//*****************************************************************************
//
//! Enables attributes of a uDMA channel
//!
//! \param ui32ChannelNum is the channel to configure.
//! \param ui32Attr is a combination of attributes for the channel.
//!
//! This function is used to enable attributes of a uDMA channel.
//!
//! The \e ui32Attr parameter is the logical OR of any of the following:
//!
//! - \b UDMA_ATTR_USEBURST is used to restrict transfers to use only a burst
//!   mode.
//! - \b UDMA_ATTR_ALTSELECT is used to select the alternate control structure
//!   for this channel (it is very unlikely that this flag should be used).
//! - \b UDMA_ATTR_HIGH_PRIORITY is used to set this channel to high priority.
//! - \b UDMA_ATTR_REQMASK is used to mask the hardware request signal from the
//!   peripheral for this channel.
//!
//! \return None
//
//*****************************************************************************
void
uDMAChannelAttributeEnable(uint32_t ui32ChannelNum, uint32_t ui32Attr)
{
    //
    // Check the arguments.
    //
    ASSERT((ui32ChannelNum & 0xffff) < 32);
    ASSERT((ui32Attr & ~(UDMA_ATTR_USEBURST | UDMA_ATTR_ALTSELECT |
                         UDMA_ATTR_HIGH_PRIORITY | UDMA_ATTR_REQMASK)) == 0);

    //
    // In case a channel selector macro (like UDMA_CH0_USB0EP1RX) was
    // passed as the ui32ChannelNum parameter, extract just the channel number
    // from this parameter.
    //
    ui32ChannelNum &= 0x1f;

    //
    // Set the useburst bit for this channel if set in ui32Config.
    //
    if(ui32Attr & UDMA_ATTR_USEBURST)
    {
        HWREG(UDMA_USEBURSTSET) = 1 << ui32ChannelNum;
    }

    //
    // Set the alternate control select bit for this channel,
    // if set in ui32Config.
    //
    if(ui32Attr & UDMA_ATTR_ALTSELECT)
    {
        HWREG(UDMA_ALTSET) = 1 << ui32ChannelNum;
    }

    //
    // Set the high priority bit for this channel, if set in ui32Config.
    //
    if(ui32Attr & UDMA_ATTR_HIGH_PRIORITY)
    {
        HWREG(UDMA_PRIOSET) = 1 << ui32ChannelNum;
    }

    //
    // Set the request mask bit for this channel, if set in ui32Config.
    //
    if(ui32Attr & UDMA_ATTR_REQMASK)
    {
        HWREG(UDMA_REQMASKSET) = 1 << ui32ChannelNum;
    }
}

//*****************************************************************************
//
//! Disables attributes of a uDMA channel
//!
//! \param ui32ChannelNum is the channel to configure.
//! \param ui32Attr is a combination of attributes for the channel.
//!
//! This function is used to disable attributes of a uDMA channel.
//!
//! The \e ui32Attr parameter is the logical OR of any of the following:
//!
//! - \b UDMA_ATTR_USEBURST is used to restrict transfers to use only a burst
//!   mode.
//! - \b UDMA_ATTR_ALTSELECT is used to select the alternate control structure
//!   for this channel.
//! - \b UDMA_ATTR_HIGH_PRIORITY is used to set this channel to high priority.
//! - \b UDMA_ATTR_REQMASK is used to mask the hardware request signal from the
//!   peripheral for this channel.
//!
//! \return None
//
//*****************************************************************************
void
uDMAChannelAttributeDisable(uint32_t ui32ChannelNum, uint32_t ui32Attr)
{
    //
    // Check the arguments.
    //
    ASSERT((ui32ChannelNum & 0xffff) < 32);
    ASSERT((ui32Attr & ~(UDMA_ATTR_USEBURST | UDMA_ATTR_ALTSELECT |
                         UDMA_ATTR_HIGH_PRIORITY | UDMA_ATTR_REQMASK)) == 0);

    //
    // In case a channel selector macro (like UDMA_CH0_USB0EP1RX) was
    // passed as the ui32ChannelNum parameter, extract just the channel number
    // from this parameter.
    //
    ui32ChannelNum &= 0x1f;

    //
    // Clear the useburst bit for this channel if set in ui32Config.
    //
    if(ui32Attr & UDMA_ATTR_USEBURST)
    {
        HWREG(UDMA_USEBURSTCLR) = 1 << ui32ChannelNum;
    }

    //
    // Clear the alternate control select bit for this channel, if set in
    // ui32Config.
    //
    if(ui32Attr & UDMA_ATTR_ALTSELECT)
    {
        HWREG(UDMA_ALTCLR) = 1 << ui32ChannelNum;
    }

    //
    // Clear the high priority bit for this channel, if set in ui32Config.
    //
    if(ui32Attr & UDMA_ATTR_HIGH_PRIORITY)
    {
        HWREG(UDMA_PRIOCLR) = 1 << ui32ChannelNum;
    }

    //
    // Clear the request mask bit for this channel, if set in ui32Config.
    //
    if(ui32Attr & UDMA_ATTR_REQMASK)
    {
        HWREG(UDMA_REQMASKCLR) = 1 << ui32ChannelNum;
    }
}

//*****************************************************************************
//
//! Gets the enabled attributes of a uDMA channel
//!
//! \param ui32ChannelNum is the channel to configure.
//!
//! This function returns a combination of flags representing the attributes of
//! the uDMA channel.
//!
//! \return Returns the logical OR of the attributes of the uDMA channel, which
//! can be any of the following:
//! - \b UDMA_ATTR_USEBURST is used to restrict transfers to use only a burst
//!   mode.
//! - \b UDMA_ATTR_ALTSELECT is used to select the alternate control structure
//!   for this channel.
//! - \b UDMA_ATTR_HIGH_PRIORITY is used to set this channel to high priority.
//! - \b UDMA_ATTR_REQMASK is used to mask the hardware request signal from the
//!   peripheral for this channel.
//
//*****************************************************************************
uint32_t
uDMAChannelAttributeGet(uint32_t ui32ChannelNum)
{
    uint32_t ui32Attr = 0;

    //
    // Check the arguments.
    //
    ASSERT((ui32ChannelNum & 0xffff) < 32);

    //
    // In case a channel selector macro (like UDMA_CH0_USB0EP1RX) was
    // passed as the ui32ChannelNum parameter, extract just the channel number
    // from this parameter.
    //
    ui32ChannelNum &= 0x1f;

    //
    // Check to see if useburst bit is set for this channel.
    //
    if(HWREG(UDMA_USEBURSTSET) & (1 << ui32ChannelNum))
    {
        ui32Attr |= UDMA_ATTR_USEBURST;
    }

    //
    // Check to see if the alternate control bit is set for this channel.
    //
    if(HWREG(UDMA_ALTSET) & (1 << ui32ChannelNum))
    {
        ui32Attr |= UDMA_ATTR_ALTSELECT;
    }

    //
    // Check to see if the high priority bit is set for this channel.
    //
    if(HWREG(UDMA_PRIOSET) & (1 << ui32ChannelNum))
    {
        ui32Attr |= UDMA_ATTR_HIGH_PRIORITY;
    }

    //
    // Check to see if the request mask bit is set for this channel.
    //
    if(HWREG(UDMA_REQMASKSET) & (1 << ui32ChannelNum))
    {
        ui32Attr |= UDMA_ATTR_REQMASK;
    }

    //
    // Return the configuration flags.
    //
    return(ui32Attr);
}

//*****************************************************************************
//
//! Sets the control parameters for a uDMA channel control structure
//!
//! \param ui32ChannelStructIndex is the logical OR of the uDMA channel number
//! with \b UDMA_PRI_SELECT or \b UDMA_ALT_SELECT.
//! \param ui32Control is logical OR of several control values to set the control
//! parameters for the channel.
//!
//! This function is used to set control parameters for a uDMA transfer.  These
//! are typically parameters that are not changed often.
//!
//! The \e ui32ChannelStructIndex parameter should be the logical OR of the
//! channel number with one of \b UDMA_PRI_SELECT or \b UDMA_ALT_SELECT to
//! choose whether the primary or alternate data structure is used.
//!
//! The \e ui32Control parameter is the logical OR of five values: the data size,
//! the source address increment, the destination address increment, the
//! arbitration size, and the use burst flag.  The choices available for each
//! of these values is described below.
//!
//! Choose the data size from one of \b UDMA_SIZE_8, \b UDMA_SIZE_16, or
//! \b UDMA_SIZE_32 to select a data size of 8, 16, or 32 bits.
//!
//! Choose the source address increment from one of \b UDMA_SRC_INC_8,
//! \b UDMA_SRC_INC_16, \b UDMA_SRC_INC_32, or \b UDMA_SRC_INC_NONE to select
//! an address increment of 8-bit bytes, 16-bit halfwords, 32-bit words, or
//! to select non-incrementing.
//!
//! Choose the destination address increment from one of \b UDMA_DST_INC_8,
//! \b UDMA_DST_INC_16, \b UDMA_DST_INC_32, or \b UDMA_DST_INC_NONE to select
//! an address increment of 8-bit bytes, 16-bit halfwords, 32-bit words, or
//! to select non-incrementing.
//!
//! The arbitration size determines how many items are transferred before
//! the uDMA controller re-arbitrates for the bus.  Choose the arbitration size
//! from one of \b UDMA_ARB_1, \b UDMA_ARB_2, \b UDMA_ARB_4, \b UDMA_ARB_8,
//! through \b UDMA_ARB_1024 to select the arbitration size from 1 to 1024
//! items, in powers of 2.
//!
//! The value \b UDMA_NEXT_USEBURST is used to force the channel to only
//! respond to burst requests at the tail end of a scatter-gather transfer.
//!
//! \note The address increment cannot be smaller than the data size.
//!
//! \return None
//
//*****************************************************************************
void
uDMAChannelControlSet(uint32_t ui32ChannelStructIndex,
                      uint32_t ui32Control)
{
    tDMAControlTable *pCtl;

    //
    // Check the arguments.
    //
    ASSERT((ui32ChannelStructIndex & 0xffff) < 64);
    ASSERT(HWREG(UDMA_CTLBASE) != 0);

    //
    // In case a channel selector macro (like UDMA_CH0_USB0EP1RX) was
    // passed as the ui32ChannelStructIndex parameter, extract just the channel
    // index from this parameter.
    //
    ui32ChannelStructIndex &= 0x3f;

    //
    // Get the base address of the control table.
    //
    pCtl = (tDMAControlTable *)HWREG(UDMA_CTLBASE);

    //
    // Get the current control word value and mask off the fields to be
    // changed, then OR in the new settings.
    //
    pCtl[ui32ChannelStructIndex].ui32Control =
        ((pCtl[ui32ChannelStructIndex].ui32Control &
          ~(UDMACHCTL_CHCTL_DSTINC_M |
            UDMACHCTL_CHCTL_DSTSIZE_M |
            UDMACHCTL_CHCTL_SRCINC_M |
            UDMACHCTL_CHCTL_SRCSIZE_M |
            UDMACHCTL_CHCTL_ARBSIZE_M |
            UDMACHCTL_CHCTL_NXTUSEBURST)) |
         ui32Control);
}

//*****************************************************************************
//
//! Sets the transfer parameters for a uDMA channel control structure
//!
//! \param ui32ChannelStructIndex is the logical OR of the uDMA channel number
//! with either \b UDMA_PRI_SELECT or \b UDMA_ALT_SELECT.
//! \param ui32Mode is the type of uDMA transfer.
//! \param pvSrcAddr is the source address for the transfer.
//! \param pvDstAddr is the destination address for the transfer.
//! \param ui32TransferSize is the number of data items to transfer.
//!
//! This function is used to set the parameters for a uDMA transfer.  These are
//! typically parameters that are changed often.  The function
//! uDMAChannelControlSet() MUST be called at least once for this channel prior
//! to calling this function.
//!
//! The \e ui32ChannelStructIndex parameter should be the logical OR of the
//! channel number with one of \b UDMA_PRI_SELECT or \b UDMA_ALT_SELECT to
//! choose whether the primary or alternate data structure is used.
//!
//! The \e ui32Mode parameter should be one of the following values:
//!
//! - \b UDMA_MODE_STOP stops the uDMA transfer.  The controller sets the mode
//!   to this value at the end of a transfer.
//! - \b UDMA_MODE_BASIC to perform a basic transfer based on request.
//! - \b UDMA_MODE_AUTO to perform a transfer that will always complete once
//!   started even if request is removed.
//! - \b UDMA_MODE_PINGPONG to set up a transfer that switches between the
//!   primary and alternate control structures for the channel.  This allows
//!   use of ping-pong buffering for uDMA transfers.
//! - \b UDMA_MODE_MEM_SCATTER_GATHER to set up a memory scatter-gather
//!   transfer.
//! - \b UDMA_MODE_PER_SCATTER_GATHER to set up a peripheral scatter-gather
//!   transfer.
//!
//! The \e pvSrcAddr and \e pvDstAddr parameters are pointers to the first
//! location of the data to be transferred.  These addresses should be aligned
//! according to the item size.  The compiler will take care of this if the
//! pointers are pointing to storage of the appropriate data type.
//!
//! The \e ui32TransferSize parameter is the number of data items, not the number
//! of bytes.
//!
//! The two scatter/gather modes, memory and peripheral, are actually different
//! depending on whether the primary or alternate control structure is
//! selected.  This function will look for the \b UDMA_PRI_SELECT and
//! \b UDMA_ALT_SELECT flag along with the channel number and will set the
//! scatter/gather mode as appropriate for the primary or alternate control
//! structure.
//!
//! The channel must also be enabled using uDMAChannelEnable() after calling
//! this function.  The transfer will not begin until the channel has been set
//! up and enabled.  Note that the channel is automatically disabled after the
//! transfer is completed, meaning that uDMAChannelEnable() must be called
//! again after setting up the next transfer.
//!
//! \note Great care must be taken to not modify a channel control structure
//! that is in use or else the results will be unpredictable, including the
//! possibility of undesired data transfers to or from memory or peripherals.
//! For BASIC and AUTO modes, it is safe to make changes when the channel is
//! disabled, or the uDMAChannelModeGet() returns \b UDMA_MODE_STOP.  For
//! PINGPONG or one of the SCATTER_GATHER modes, it is safe to modify the
//! primary or alternate control structure only when the other is being used.
//! The uDMAChannelModeGet() function will return \b UDMA_MODE_STOP when a
//! channel control structure is inactive and safe to modify.
//!
//! \return None
//
//*****************************************************************************
void
uDMAChannelTransferSet(uint32_t ui32ChannelStructIndex,
                       uint32_t ui32Mode, void *pvSrcAddr, void *pvDstAddr,
                       uint32_t ui32TransferSize)
{
    tDMAControlTable *pControlTable;
    uint32_t ui32Control;
    uint32_t ui32Inc;
    uint32_t ui32BufferBytes;

    //
    // Check the arguments.
    //
    ASSERT((ui32ChannelStructIndex & 0xffff) < 64);
    ASSERT(HWREG(UDMA_CTLBASE) != 0);
    ASSERT(ui32Mode <= UDMA_MODE_PER_SCATTER_GATHER);
    ASSERT((uint32_t)pvSrcAddr >= 0x20000000);
    ASSERT((uint32_t)pvDstAddr >= 0x20000000);
    ASSERT((ui32TransferSize != 0) && (ui32TransferSize <= 1024));

    //
    // In case a channel selector macro (like UDMA_CH0_USB0EP1RX) was
    // passed as the ui32ChannelStructIndex parameter, extract just the channel
    // index from this parameter.
    //
    ui32ChannelStructIndex &= 0x3f;

    //
    // Get the base address of the control table.
    //
    pControlTable = (tDMAControlTable *)HWREG(UDMA_CTLBASE);

    //
    // Get the current control word value and mask off the mode and size
    // fields.
    //
    ui32Control = (pControlTable[ui32ChannelStructIndex].ui32Control &
                   ~(UDMACHCTL_CHCTL_XFERSIZE_M | UDMACHCTL_CHCTL_XFERMODE_M));

    //
    // Adjust the mode if the alt control structure is selected.
    //
    if(ui32ChannelStructIndex & UDMA_ALT_SELECT)
    {
        if((ui32Mode == UDMA_MODE_MEM_SCATTER_GATHER) ||
                (ui32Mode == UDMA_MODE_PER_SCATTER_GATHER))
        {
            ui32Mode |= UDMA_MODE_ALT_SELECT;
        }
    }

    //
    // Set the transfer size and mode in the control word (but don't write the
    // control word yet as it could kick off a transfer).
    //
    ui32Control |= ui32Mode | ((ui32TransferSize - 1) << 4);

    //
    // Get the address increment value for the source, from the control word.
    //
    ui32Inc = (ui32Control & UDMACHCTL_CHCTL_SRCINC_M);

    //
    // Compute the ending source address of the transfer.  If the source
    // increment is set to none, then the ending address is the same as the
    // beginning.
    //
    if(ui32Inc != UDMA_SRC_INC_NONE)
    {
        ui32Inc = ui32Inc >> 26;
        ui32BufferBytes = ui32TransferSize << ui32Inc;
        pvSrcAddr = (void *)((uint32_t)pvSrcAddr + ui32BufferBytes - 1);
    }

    //
    // Load the source ending address into the control block.
    //
    pControlTable[ui32ChannelStructIndex].pvSrcEndAddr = pvSrcAddr;

    //
    // Get the address increment value for the destination, from the control
    // word.
    //
    ui32Inc = ui32Control & UDMACHCTL_CHCTL_DSTINC_M;

    //
    // Compute the ending destination address of the transfer.  If the
    // destination increment is set to none, then the ending address is the
    // same as the beginning.
    //
    if(ui32Inc != UDMA_DST_INC_NONE)
    {
        //
        // There is a special case if this is setting up a scatter-gather
        // transfer.  The destination pointer needs to point to the end of
        // the alternate structure for this channel instead of calculating
        // the end of the buffer in the normal way.
        //
        if((ui32Mode == UDMA_MODE_MEM_SCATTER_GATHER) ||
                (ui32Mode == UDMA_MODE_PER_SCATTER_GATHER))
        {
            pvDstAddr =
                (void *)&pControlTable[ui32ChannelStructIndex |
                                       UDMA_ALT_SELECT].ui32Spare;
        }
        //
        // Not a scatter-gather transfer, calculate end pointer normally.
        //
        else
        {
            ui32Inc = ui32Inc >> 30;
            ui32BufferBytes = ui32TransferSize << ui32Inc;
            pvDstAddr = (void *)((uint32_t)pvDstAddr + ui32BufferBytes - 1);
        }
    }

    //
    // Load the destination ending address into the control block.
    //
    pControlTable[ui32ChannelStructIndex].pvDstEndAddr = pvDstAddr;

    //
    // Write the new control word value.
    //
    pControlTable[ui32ChannelStructIndex].ui32Control = ui32Control;
}

//*****************************************************************************
//
//! Configures a uDMA channel for scatter-gather mode
//!
//! \param ui32ChannelNum is the uDMA channel number.
//! \param ui32TaskCount is the number of scatter-gather tasks to execute.
//! \param pvTaskList is a pointer to the beginning of the scatter-gather
//! task list.
//! \param ui32IsPeriphSG is a flag to indicate it is a peripheral scatter-gather
//! transfer (else it will be memory scatter-gather transfer)
//!
//! This function is used to configure a channel for scatter-gather mode.
//! The caller must have already set up a task list, and pass a pointer to
//! the start of the task list as the \e pvTaskList parameter.  The
//! \e ui32TaskCount parameter is the count of tasks in the task list, not the
//! size of the task list.  The flag \e bIsPeriphSG should be used to indicate
//! if the scatter-gather should be configured for a peripheral or memory
//! scatter-gather operation.
//!
//! \sa uDMATaskStructEntry
//!
//! \return None
//
//*****************************************************************************
void
uDMAChannelScatterGatherSet(uint32_t ui32ChannelNum,
                            uint32_t ui32TaskCount, void *pvTaskList,
                            uint32_t ui32IsPeriphSG)
{
    tDMAControlTable *pControlTable;
    tDMAControlTable *pTaskTable;

    //
    // Check the parameters
    //
    ASSERT((ui32ChannelNum & 0xffff) < 32);
    ASSERT(HWREG(UDMA_CTLBASE) != 0);
    ASSERT(pvTaskList != 0);
    ASSERT(ui32TaskCount <= 1024);
    ASSERT(ui32TaskCount != 0);

    //
    // In case a channel selector macro (like UDMA_CH0_USB0EP1RX) was
    // passed as the ui32ChannelNum parameter, extract just the channel number
    // from this parameter.
    //
    ui32ChannelNum &= 0x1f;

    //
    // Get the base address of the control table.
    //
    pControlTable = (tDMAControlTable *)HWREG(UDMA_CTLBASE);

    //
    // Get a handy pointer to the task list
    //
    pTaskTable = (tDMAControlTable *)pvTaskList;

    //
    // Compute the ending address for the source pointer.  This will be the
    // last element of the last task in the task table
    //
    pControlTable[ui32ChannelNum].pvSrcEndAddr =
        &pTaskTable[ui32TaskCount - 1].ui32Spare;

    //
    // Compute the ending address for the destination pointer.  This will be
    // the end of the alternate structure for this channel.
    //
    pControlTable[ui32ChannelNum].pvDstEndAddr =
        &pControlTable[ui32ChannelNum | UDMA_ALT_SELECT].ui32Spare;

    //
    // Compute the control word.  Most configurable items are fixed for
    // scatter-gather.  Item and increment sizes are all 32-bit and arb
    // size must be 4.  The count is the number of items in the task list
    // times 4 (4 words per task).
    //
    pControlTable[ui32ChannelNum].ui32Control =
        (UDMA_CHCTL_DSTINC_32 | UDMA_CHCTL_DSTSIZE_32 |
         UDMA_CHCTL_SRCINC_32 | UDMA_CHCTL_SRCSIZE_32 |
         UDMA_CHCTL_ARBSIZE_4 |
         (((ui32TaskCount * 4) - 1) << UDMACHCTL_CHCTL_XFERSIZE_S) |
         (ui32IsPeriphSG ? UDMA_CHCTL_XFERMODE_PER_SG :
          UDMA_CHCTL_XFERMODE_MEM_SG));
}

//*****************************************************************************
//
//! Gets the current transfer size for a uDMA channel control structure
//!
//! \param ui32ChannelStructIndex is the logical OR of the uDMA channel number
//! with either \b UDMA_PRI_SELECT or \b UDMA_ALT_SELECT.
//!
//! This function is used to get the uDMA transfer size for a channel.  The
//! transfer size is the number of items to transfer, where the size of an item
//! might be 8, 16, or 32 bits.  If a partial transfer has already occurred,
//! then the number of remaining items will be returned.  If the transfer is
//! complete, then 0 will be returned.
//!
//! \return Returns the number of items remaining to transfer.
//
//*****************************************************************************
uint32_t
uDMAChannelSizeGet(uint32_t ui32ChannelStructIndex)
{
    tDMAControlTable *pControlTable;
    uint32_t ui32Control;

    //
    // Check the arguments.
    //
    ASSERT((ui32ChannelStructIndex & 0xffff) < 64);
    ASSERT(HWREG(UDMA_CTLBASE) != 0);

    //
    // In case a channel selector macro (like UDMA_CH0_USB0EP1RX) was
    // passed as the ui32ChannelStructIndex parameter, extract just the channel
    // index from this parameter.
    //
    ui32ChannelStructIndex &= 0x3f;

    //
    // Get the base address of the control table.
    //
    pControlTable = (tDMAControlTable *)HWREG(UDMA_CTLBASE);

    //
    // Get the current control word value and mask off all but the size field
    // and the mode field.
    //
    ui32Control = (pControlTable[ui32ChannelStructIndex].ui32Control &
                   (UDMACHCTL_CHCTL_XFERSIZE_M | UDMACHCTL_CHCTL_XFERMODE_M));

    //
    // If the size field and mode field are 0 then the transfer is finished
    // and there are no more items to transfer
    //
    if(ui32Control == 0)
    {
        return(0);
    }

    //
    // Otherwise, if either the size field or more field is non-zero, then
    // not all the items have been transferred.
    //
    else
    {
        //
        // Shift the size field and add one, then return to user.
        //
        return((ui32Control >> 4) + 1);
    }
}

//*****************************************************************************
//
//! Gets the transfer mode for a uDMA channel control structure
//!
//! \param ui32ChannelStructIndex is the logical OR of the uDMA channel number
//! with either \b UDMA_PRI_SELECT or \b UDMA_ALT_SELECT.
//!
//! This function is used to get the transfer mode for the uDMA channel.  It
//! can be used to query the status of a transfer on a channel.  When the
//! transfer is complete the mode will be \b UDMA_MODE_STOP.
//!
//! \return Returns the transfer mode of the specified channel and control
//! structure, which will be one of the following values: \b UDMA_MODE_STOP,
//! \b UDMA_MODE_BASIC, \b UDMA_MODE_AUTO, \b UDMA_MODE_PINGPONG,
//! \b UDMA_MODE_MEM_SCATTER_GATHER, or \b UDMA_MODE_PER_SCATTER_GATHER.
//
//*****************************************************************************
uint32_t
uDMAChannelModeGet(uint32_t ui32ChannelStructIndex)
{
    tDMAControlTable *pControlTable;
    uint32_t ui32Control;

    //
    // Check the arguments.
    //
    ASSERT((ui32ChannelStructIndex & 0xffff) < 64);
    ASSERT(HWREG(UDMA_CTLBASE) != 0);

    //
    // In case a channel selector macro (like UDMA_CH0_USB0EP1RX) was
    // passed as the ui32ChannelStructIndex parameter, extract just the channel
    // index from this parameter.
    //
    ui32ChannelStructIndex &= 0x3f;

    //
    // Get the base address of the control table.
    //
    pControlTable = (tDMAControlTable *)HWREG(UDMA_CTLBASE);

    //
    // Get the current control word value and mask off all but the mode field.
    //
    ui32Control = (pControlTable[ui32ChannelStructIndex].ui32Control &
                   UDMACHCTL_CHCTL_XFERMODE_M);

    //
    // Check if scatter/gather mode, and if so, mask off the alt bit.
    //
    if(((ui32Control & ~UDMA_MODE_ALT_SELECT) == UDMA_MODE_MEM_SCATTER_GATHER) ||
            ((ui32Control & ~UDMA_MODE_ALT_SELECT) == UDMA_MODE_PER_SCATTER_GATHER))
    {
        ui32Control &= ~UDMA_MODE_ALT_SELECT;
    }

    //
    // Return the mode to the caller.
    //
    return(ui32Control);
}


//*****************************************************************************
//
//! Registers an interrupt handler for the uDMA controller
//!
//! \param ui32IntChannel identifies which uDMA interrupt is to be registered.
//! \param pfnHandler is a pointer to the function to be called when the
//! interrupt is activated.
//!
//! This sets and enables the handler to be called when the uDMA controller
//! generates an interrupt.  The \e ui32IntChannel parameter should be one of the
//! following:
//!
//! - \b UDMA_INT_SW to register an interrupt handler to process interrupts
//!   from the uDMA software channel (UDMA_CH30_SW)
//! - \b UDMA_INT_ERR to register an interrupt handler to process uDMA error
//!   interrupts
//!
//! \sa IntRegister() for important information about registering interrupt
//! handlers.
//!
//! \note The interrupt handler for uDMA is for transfer completion when the
//! channel UDMA_CH30W is used, and for error interrupts.  The
//! interrupts for each peripheral channel are handled through the individual
//! peripheral interrupt handlers.
//!
//! \return None
//
//*****************************************************************************
void
uDMAIntRegister(uint32_t ui32IntChannel, void (*pfnHandler)(void))
{
    //
    // Check the arguments.
    //
    ASSERT(pfnHandler);
    ASSERT((ui32IntChannel == UDMA_INT_SW) || (ui32IntChannel == UDMA_INT_ERR));

    //
    // Register the interrupt handler.
    //
    IntRegister(ui32IntChannel, pfnHandler);

    //
    // Enable the memory management fault.
    //
    IntEnable(ui32IntChannel);
}

//*****************************************************************************
//
//! Unregisters an interrupt handler for the uDMA controller
//!
//! \param ui32IntChannel identifies which uDMA interrupt to unregister.
//!
//! This function will disable and clear the handler to be called for the
//! specified uDMA interrupt.  The \e ui32IntChannel parameter should be one of
//! \b UDMA_INT_SW or \b UDMA_INT_ERR as documented for the function
//! uDMAIntRegister().
//!
//! \sa IntRegister() for important information about registering interrupt
//! handlers.
//!
//! \return None
//
//*****************************************************************************
void
uDMAIntUnregister(uint32_t ui32IntChannel)
{
    //
    // Disable the interrupt.
    //
    IntDisable(ui32IntChannel);

    //
    // Unregister the interrupt handler.
    //
    IntUnregister(ui32IntChannel);
}

//*****************************************************************************
//
//! Gets the uDMA controller channel interrupt status
//!
//! This function is used to get the interrupt status of the uDMA controller.
//! The returned value is a 32-bit bit mask that indicates which channels are
//! requesting an interrupt.  This function can be used from within an
//! interrupt handler to determine or confirm which uDMA channel has requested
//! an interrupt.
//!
//! \note This function is only available on devices that have the DMA Channel
//! Interrupt Status Register (DMACHIS).  Please consult the data sheet for
//! your part.
//!
//! \return Returns a 32-bit mask which indicates requesting uDMA channels.
//! There is a bit for each channel, and a 1 in a bit indicates that channel
//! is requesting an interrupt.  Multiple bits can be set.
//
//*****************************************************************************
uint32_t
uDMAIntStatus(void)
{
    //
    // Return the value of the uDMA interrupt status register
    //
    return(HWREG(UDMA_CHIS));
}

//*****************************************************************************
//
//! Clears uDMA interrupt status
//!
//! \param ui32ChanMask is a 32-bit mask with one bit for each uDMA channel.
//!
//! Clears bits in the uDMA interrupt status register according to which bits
//! are set in \e ui32ChanMask. There is one bit for each channel. If a a bit
//! is set in \e ui32ChanMask, then that corresponding channel's interrupt
//! status will be cleared (if it was set).
//!
//! \note This function is only available on devices that have the DMA Channel
//! Interrupt Status Register (DMACHIS).  Please consult the data sheet for
//! your part.
//!
//! \return None
//
//*****************************************************************************
void
uDMAIntClear(uint32_t ui32ChanMask)
{
    //
    // Clear the requested bits in the uDMA interrupt status register
    //
    HWREG(UDMA_CHIS) = ui32ChanMask;
}

//*****************************************************************************
//
//! Assigns a peripheral mapping for a uDMA channel
//!
//! \param ui32Mapping is a macro specifying the peripheral assignment for
//! a channel
//!
//! This function assigns a peripheral mapping to a uDMA channel.  It is
//! used to select which peripheral is used for a uDMA channel.  The parameter
//! \e ui32Mapping should be one of the macros named \b UDMA_CHn_tttt from the
//! header file \e udma.h.  For example, to assign uDMA channel 8 to the
//! UART0RX channel, the parameter should be the macro \b UDMA_CH8_UART0RX.
//!
//! Please consult the cc2538 data sheet for a table showing all the
//! possible peripheral assignments for the uDMA channels for a particular
//! device.
//!
//! \note This function is only available on devices that have the DMA Channel
//! Map Select registers (DMACHMAP0-3).  Please consult the data sheet for
//! your part.
//!
//! \return None
//
//*****************************************************************************
void
uDMAChannelAssign(uint32_t ui32Mapping)
{
    uint32_t ui32MapReg;
    uint32_t ui32MapShift;
    uint32_t ui32ChannelNum;
    uint32_t ui32Encoding;

    //
    // Check the parameters
    //
    ASSERT((ui32Mapping & 0xffffff00) < 0x00050000);

    //
    // Extract the channel number and map encoding value from the parameter.
    //
    ui32ChannelNum = ui32Mapping & 0xff;
    ui32Encoding   = ui32Mapping >> 16;

    //
    // Find the uDMA channel mapping register and shift value to use for this
    // channel
    //
    ui32MapReg = UDMA_CHMAP0 + ((ui32ChannelNum / 8) * 4);
    ui32MapShift = (ui32ChannelNum % 8) * 4;

    //
    // Set the channel map encoding for this channel
    //
    HWREG(ui32MapReg) = (HWREG(ui32MapReg) & ~(0xf << ui32MapShift)) |
                        ui32Encoding << ui32MapShift;
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
