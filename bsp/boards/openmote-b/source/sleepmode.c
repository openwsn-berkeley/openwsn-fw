/******************************************************************************
*  Filename:       sleepmode.c
*  Revised:        $Date: 2013-03-22 16:13:31 +0100 (Fri, 22 Mar 2013) $
*  Revision:       $Revision: 9513 $
*
*  Description:    Driver for the Sleep Mode Timer Module.
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
//! \addtogroup sleepmodetimer_api
//! @{
//
//*****************************************************************************

#include <headers/hw_ints.h>
#include <headers/hw_memmap.h>
#include <headers/hw_smwdthrosc.h>
#include "debug.h"
#include "interrupt.h"
#include "sleepmode.h"

//*****************************************************************************
//
//! Registers an interrupt handler for Sleep Mode Timer interrupt
//!
//! \param pfnHandler is a pointer to the function to be called when the
//! Sleep Mode Timer interrupt occurs.
//!
//! This function does the actual registering of the interrupt handler, thus
//! enabling the global interrupt in the interrupt controller.
//!
//! \sa IntRegister() for important information about registering interrupt
//! handlers.
//!
//! \return None
//
//*****************************************************************************
void
SleepModeIntRegister(void (*pfnHandler)(void))
{
    //
    // Register the interrupt handler.
    //
    IntRegister(INT_SMTIM, pfnHandler);

    //
    // Enable the sleep mode timer interrupt.
    //
    IntEnable(INT_SMTIM);
}

//*****************************************************************************
//
//! Unregisters an interrupt handler for the sleep mode timer interrupt
//!
//! This function does the actual unregistering of the interrupt handler.  This
//! function clears the handler to be called when a compare
//! interrupt occurs and masks off the interrupt in the interrupt controller
//! so that the interrupt handler no longer is called.
//!
//! \sa IntRegister() for important information about registering interrupt
//! handlers.
//!
//! \return None
//
//*****************************************************************************
void
SleepModeIntUnregister(void)
{
    //
    // Disable the interrupt.
    //
    IntDisable(INT_SMTIM);

    //
    // Unregister the interrupt handler.
    //
    IntUnregister(INT_SMTIM);
}

//*****************************************************************************
//
//! Get current value of the sleep mode timer
//!
//! This function returns the current value of the sleep mode timer (that is,
//! the timer count)
//!
//! \return Current value of the sleep mode timer
//
//*****************************************************************************
uint32_t
SleepModeTimerCountGet(void)
{
    uint32_t ui32Val;

    ui32Val = HWREG(SMWDTHROSC_ST0);
    ui32Val |= HWREG(SMWDTHROSC_ST1) << 8;
    ui32Val |= HWREG(SMWDTHROSC_ST2) << 16;
    ui32Val |= HWREG(SMWDTHROSC_ST3) << 24;

    return ui32Val;
}


//*****************************************************************************
//
//! Selects capture port and pin
//!
//! \param ui32Port is the port.
//! \param ui32Pin is the pin number.
//!
//! This function sets the port and pin on which values are to be captured.
//!
//! The \e ui32Port argument must be only one of the following values:
//! \b SLEEPMODE_PORT_A, \b SLEEPMODE_PORT_B,
//! \b SLEEPMODE_PORT_C, \b SLEEPMODE_PORT_D,
//! \b SLEEPMODE_PORT_USB.
//!
//! The \e ui32Pin argument must be only one of the following values:
//! \b SLEEPMODE_PIN_0, \b SLEEPMODE_PIN_1, \b SLEEPMODE_PIN_2,
//! \b SLEEPMODE_PIN_3, \b SLEEPMODE_PIN_4, \b SLEEPMODE_PIN_5,
//! \b SLEEPMODE_PIN_6, \b SLEEPMODE_PIN_7.
//!
//! \note if \e ui32Port is set to \b SLEEPMODE_PORT_USB, only \e ui32Pin
//! \b SLEEPMODE_PIN_0 can be used.
//!
//! \return None
//
//*****************************************************************************
void
SleepModeCaptureConfig(uint32_t ui32Port, uint32_t ui32Pin)
{
    uint32_t  ui32Val;

    ASSERT(ui32Port == SLEEPMODE_PORT_A ||
           ui32Port == SLEEPMODE_PORT_B ||
           ui32Port == SLEEPMODE_PORT_C ||
           ui32Port == SLEEPMODE_PORT_D ||
           (ui32Port == SLEEPMODE_PORT_USB && ui32Pin == SLEEPMODE_PIN_0));

    ASSERT(ui32Pin == SLEEPMODE_PIN_0 ||
           ui32Pin == SLEEPMODE_PIN_1 ||
           ui32Pin == SLEEPMODE_PIN_2 ||
           ui32Pin == SLEEPMODE_PIN_3 ||
           ui32Pin == SLEEPMODE_PIN_4 ||
           ui32Pin == SLEEPMODE_PIN_5 ||
           ui32Pin == SLEEPMODE_PIN_6 ||
           ui32Pin == SLEEPMODE_PIN_7);


    ui32Val = HWREG(SMWDTHROSC_STCC);
    ui32Val &= ~(SMWDTHROSC_STCC_PORT_M | SMWDTHROSC_STCC_PIN_M);
    ui32Val |= ui32Port | ui32Pin;
    HWREG(SMWDTHROSC_STCC) = ui32Val;

}

//*****************************************************************************
//
//! Set compare value of the sleep mode timer
//!
//! \param ui32Compare is a 32-bit compare value.
//!
//! This function sets the compare value of the sleep mode timer.
//! A timer compare interrupt is generated when the timer value is equal to
//! the compare value.
//!
//! \note When setting a new compare value, the value must be at least 5 more
//! than the current sleep timer value. Otherwise, the timer compare event
//! might be lost.
//!
//! \return None
//
//*****************************************************************************
void
SleepModeTimerCompareSet(uint32_t ui32Compare)
{
    //
    // Wait for ST0, ST3 regs to be ready for writing
    //
    while(!(HWREG(SMWDTHROSC_STLOAD) & SMWDTHROSC_STLOAD_STLOAD))
    {
    }

    HWREG(SMWDTHROSC_ST3) = (ui32Compare >> 24) & 0x000000ff;
    HWREG(SMWDTHROSC_ST2) = (ui32Compare >> 16) & 0x000000ff;
    HWREG(SMWDTHROSC_ST1) = (ui32Compare >>  8) & 0x000000ff;
    HWREG(SMWDTHROSC_ST0) = ui32Compare & 0x000000ff;
}


//*****************************************************************************
//
//! Get last capture value
//!
//! This function returns the last captured value.
//!
//! \note The captured value is one more than the value at the instant for the
//! event on the I/O pin. Software should therefore subtract 1 from the
//! captured value if absolute timing is required.
//!
//! \sa SleepModeCaptureNew(), SleepModeCaptureIsValid()
//!
//! \return Last captured value
//
//*****************************************************************************
uint32_t
SleepModeCaptureGet(void)
{
    uint32_t ui32Val;

    ui32Val = HWREG(SMWDTHROSC_STCV0);
    ui32Val |= HWREG(SMWDTHROSC_STCV1) << 8;
    ui32Val |= HWREG(SMWDTHROSC_STCV2) << 16;
    ui32Val |= HWREG(SMWDTHROSC_STCV3) << 24;

    return ui32Val;
}

//*****************************************************************************
//
//! Checks if capture value has been updated
//!
//! This function returns true if a value has been captured.
//!
//! \sa SleepModeCaptureGet(), SleepModeCaptureNew()
//!
//! \return Returns true if capture value has been updated
//
//*****************************************************************************
bool
SleepModeCaptureIsValid(void)
{
    bool bValid;

    bValid = HWREG(SMWDTHROSC_STCS) & SMWDTHROSC_STCS_VALID;

    return bValid;
}

//*****************************************************************************
//
//! Prepares for a new value to  be captured
//!
//! This function prepares the capture logic to capture a new value.
//!
//! The relevant pin interrupt flag must be cleared after calling this
//! function using IntPendClear().
//!
//! \sa SleepModeCaptureGet(), SleepModeCaptureIsValid()
//!
//! \return None
//
//*****************************************************************************
void
SleepModeCaptureNew(void)
{
    uint32_t  ui32Val;

    ui32Val = HWREG(SMWDTHROSC_STCS);
    ui32Val &= ~SMWDTHROSC_STCS_VALID;
    HWREG(SMWDTHROSC_STCS) = ui32Val;
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

