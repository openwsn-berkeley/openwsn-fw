/******************************************************************************
*  Filename:       watchdog.c
*  Revised:        $Date: 2013-04-04 15:31:10 +0200 (Thu, 04 Apr 2013) $
*  Revision:       $Revision: 9634 $
*
*  Description:    Driver for the Watchdog Timer Module.
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
//! \addtogroup watchdog_api
//! @{
//
//*****************************************************************************

#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_smwdthrosc.h"
#include "debug.h"
#include "interrupt.h"
#include "watchdog.h"

//*****************************************************************************
//
//! Enables the watchdog timer
//!
//! \param ui32Interval is the timer interval setting.
//!
//! This function sets the timer interval and enables the watchdog timer.
//! A timeout causes a chip reset.
//!
//! The \e ui32Interval argument must be only one of the following values:
//! \b WATCHDOG_INTERVAL_32768, \b WATCHDOG_INTERVAL_8192,
//! \b WATCHDOG_INTERVAL_512,   \b WATCHDOG_INTERVAL_64.
//!
//! \sa WatchdogDisable()
//!
//! \return None
//
//*****************************************************************************
void
WatchdogEnable(uint32_t ui32Interval)
{
    uint32_t ui32Regval;
    //
    // Check the arguments.
    //
    ASSERT(ui32Interval == WATCHDOG_INTERVAL_32768 ||
           ui32Interval == WATCHDOG_INTERVAL_8192  ||
           ui32Interval == WATCHDOG_INTERVAL_512   ||
           ui32Interval == WATCHDOG_INTERVAL_64);

    // Disable Timer to set interval
    HWREG(SMWDTHROSC_WDCTL) &= ~SMWDTHROSC_WDCTL_EN;
    ui32Regval = HWREG(SMWDTHROSC_WDCTL);
    ui32Regval &= ~SMWDTHROSC_WDCTL_INT_M;
    ui32Regval |= ui32Interval;
    HWREG(SMWDTHROSC_WDCTL) = ui32Regval;

    //
    // Enable the watchdog timer module.
    //
    ui32Regval = HWREG(SMWDTHROSC_WDCTL);
    ui32Regval &= ~0x4;
    ui32Regval |= SMWDTHROSC_WDCTL_EN;
    HWREG(SMWDTHROSC_WDCTL) = ui32Regval;
}

//*****************************************************************************
//
//! Clear watch dog timer
//!
//! This function clears the watch dog timer.
//! Timer must be enabled for the clear operation to take effect.
//!
//! \return None
//
//*****************************************************************************
void
WatchdogClear(void)
{
    uint32_t ui32Reg1;
    uint32_t ui32Reg2;

    //
    // Write 0xA followed by 0x5 to CLR field
    // (0x5 also clears in timer mode)
    //
    ui32Reg1 = HWREG(SMWDTHROSC_WDCTL);
    ui32Reg1 &= ~SMWDTHROSC_WDCTL_CLR_M;
    ui32Reg2 = ui32Reg1 | (0x5 << SMWDTHROSC_WDCTL_CLR_S);
    ui32Reg1 |= 0xa << SMWDTHROSC_WDCTL_CLR_S;

    //
    // The following two writes must happen within 0.5 cycle of WDT clock!
    // for clear to actually happen.
    // Note:  might need to "safe guard" this zone by disabling interrupts.
    //
    HWREG(SMWDTHROSC_WDCTL) = ui32Reg1;
    HWREG(SMWDTHROSC_WDCTL) = ui32Reg2;
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
