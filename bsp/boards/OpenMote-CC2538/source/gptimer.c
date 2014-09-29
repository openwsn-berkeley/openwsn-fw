/******************************************************************************
*  Filename:       gptimer.c
*  Revised:        $Date: 2013-04-12 14:54:28 +0200 (Fri, 12 Apr 2013) $
*  Revision:       $Revision: 9731 $
*
*  Description:    Driver for the general purpose timer module.
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
//! \addtogroup timer_api
//! @{
//
//*****************************************************************************

#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_gptimer.h"
#include "debug.h"
#include "interrupt.h"
#include "gptimer.h"

//*****************************************************************************
//
//! \internal
//! Checks a timer base address
//!
//! \param ui32Base is the base address of the timer module.
//!
//! This function determines if a timer module base address is valid.
//!
//! \return Returns \b true if the base address is valid and \b false
//! otherwise.
//
//*****************************************************************************
#ifdef ENABLE_ASSERT
static bool
TimerBaseValid(uint32_t ui32Base)
{
    return((ui32Base == GPTIMER0_BASE) || (ui32Base == GPTIMER1_BASE) ||
           (ui32Base == GPTIMER2_BASE) || (ui32Base == GPTIMER3_BASE));
}
#endif

//*****************************************************************************
//
//! Enables the timer(s)
//!
//! \param ui32Base is the base address of the timer module.
//! \param ui32Timer specifies the timer(s) to enable; must be one of \b GPTIMER_A,
//! \b GPTIMER_B, or \b GPTIMER_BOTH.
//!
//! This function enables operation of the timer module.  The timer must be
//! configured before it is enabled.
//!
//! \return None
//
//*****************************************************************************
void
TimerEnable(uint32_t ui32Base, uint32_t ui32Timer)
{
    //
    // Check the arguments.
    //
    ASSERT(TimerBaseValid(ui32Base));
    ASSERT((ui32Timer == GPTIMER_A) || (ui32Timer == GPTIMER_B) ||
           (ui32Timer == GPTIMER_BOTH));

    //
    // Enable the timer(s) module.
    //
    HWREG(ui32Base + GPTIMER_O_CTL) |= ui32Timer & (GPTIMER_CTL_TAEN | GPTIMER_CTL_TBEN);
}

//*****************************************************************************
//
//! Disables the timer(s)
//!
//! \param ui32Base is the base address of the timer module.
//! \param ui32Timer specifies the timer(s) to disable; must be one of
//! \b GPTIMER_A, \b GPTIMER_B, or \b GPTIMER_BOTH.
//!
//! This function disables operation of the timer module.
//!
//! \return None
//
//*****************************************************************************
void
TimerDisable(uint32_t ui32Base, uint32_t ui32Timer)
{
    //
    // Check the arguments.
    //
    ASSERT(TimerBaseValid(ui32Base));
    ASSERT((ui32Timer == GPTIMER_A) || (ui32Timer == GPTIMER_B) ||
           (ui32Timer == GPTIMER_BOTH));

    //
    // Disable the timer module.
    //
    HWREG(ui32Base + GPTIMER_O_CTL) &= ~(ui32Timer &
                                         (GPTIMER_CTL_TAEN | GPTIMER_CTL_TBEN));
}


//*****************************************************************************
//
//! Configures the timer(s)
//!
//! \param ui32Base is the base address of the timer module.
//! \param ui32Config is the configuration for the timer.
//!
//! This function configures the operating mode of the timer(s).  The timer
//! module is disabled before being configured, and is left in the disabled
//! state.
//! The 16/32-bit timer is comprised of two 16-bit timers that can
//! operate independently or be concatenated to form a 32-bit timer.
//!
//! The configuration is specified in \e ui32Config as one of the following
//! values:
//!
//! - \b GPTIMER_CFG_ONE_SHOT - Full-width one-shot timer
//! - \b GPTIMER_CFG_ONE_SHOT_UP - Full-width one-shot timer that counts up
//!   instead of down (not available on all parts)
//! - \b GPTIMER_CFG_PERIODIC - Full-width periodic timer
//! - \b GPTIMER_CFG_PERIODIC_UP - Full-width periodic timer that counts up
//!   instead of down (not available on all parts)
//! - \b GPTIMER_CFG_SPLIT_PAIR - Two half-width timers
//!
//! When configured for a pair of half-width timers, each timer is separately
//! configured.  The first timer is configured by setting \e ui32Config to
//! the result of a logical OR operation between one of the following values
//! and \e ui32Config:
//!
//! - \b GPTIMER_CFG_A_ONE_SHOT - Half-width one-shot timer
//! - \b GPTIMER_CFG_A_ONE_SHOT_UP - Half-width one-shot timer that counts up
//!   instead of down (not available on all parts)
//! - \b GPTIMER_CFG_A_PERIODIC - Half-width periodic timer
//! - \b GPTIMER_CFG_A_PERIODIC_UP - Half-width periodic timer that counts up
//!   instead of down (not available on all parts)
//! - \b GPTIMER_CFG_A_CAP_COUNT - Half-width edge count capture
//! - \b GPTIMER_CFG_A_CAP_COUNT_UP - Half-width edge count capture that counts
//!   up instead of down (not available on all parts)
//! - \b GPTIMER_CFG_A_CAP_TIME - Half-width edge time capture
//! - \b GPTIMER_CFG_A_CAP_TIME_UP - Half-width edge time capture that
//!   counts up instead of down (not available on all parts)
//! - \b GPTIMER_CFG_A_PWM - Half-width PWM output
//!
//! Similarly, the second timer is configured by setting \e ui32Config to
//! the result of a logical OR operation between one of the corresponding
//! \b GPTIMER_CFG_B_* values and \e ui32Config.
//!
//! \return None
//
//*****************************************************************************
void
TimerConfigure(uint32_t ui32Base, uint32_t ui32Config)
{
    //
    // Check the arguments.
    //
    ASSERT(TimerBaseValid(ui32Base));
    ASSERT((ui32Config == GPTIMER_CFG_ONE_SHOT) ||
           (ui32Config == GPTIMER_CFG_ONE_SHOT_UP) ||
           (ui32Config == GPTIMER_CFG_PERIODIC) ||
           (ui32Config == GPTIMER_CFG_PERIODIC_UP) ||
           ((ui32Config & 0xff000000) == GPTIMER_CFG_SPLIT_PAIR));
    ASSERT(((ui32Config & 0xff000000) != GPTIMER_CFG_SPLIT_PAIR) ||
           ((((ui32Config & 0x000000ff) == GPTIMER_CFG_A_ONE_SHOT) ||
             ((ui32Config & 0x000000ff) == GPTIMER_CFG_A_ONE_SHOT_UP) ||
             ((ui32Config & 0x000000ff) == GPTIMER_CFG_A_PERIODIC) ||
             ((ui32Config & 0x000000ff) == GPTIMER_CFG_A_PERIODIC_UP) ||
             ((ui32Config & 0x000000ff) == GPTIMER_CFG_A_CAP_COUNT) ||
             ((ui32Config & 0x000000ff) == GPTIMER_CFG_A_CAP_TIME) ||
             ((ui32Config & 0x000000ff) == GPTIMER_CFG_A_PWM)) &&
            (((ui32Config & 0x0000ff00) == GPTIMER_CFG_B_ONE_SHOT) ||
             ((ui32Config & 0x0000ff00) == GPTIMER_CFG_B_ONE_SHOT_UP) ||
             ((ui32Config & 0x0000ff00) == GPTIMER_CFG_B_PERIODIC) ||
             ((ui32Config & 0x0000ff00) == GPTIMER_CFG_B_PERIODIC_UP) ||
             ((ui32Config & 0x0000ff00) == GPTIMER_CFG_B_CAP_COUNT) ||
             ((ui32Config & 0x0000ff00) == GPTIMER_CFG_B_CAP_COUNT_UP) ||
             ((ui32Config & 0x0000ff00) == GPTIMER_CFG_B_CAP_TIME) ||
             ((ui32Config & 0x0000ff00) == GPTIMER_CFG_B_CAP_TIME_UP) ||
             ((ui32Config & 0x0000ff00) == GPTIMER_CFG_B_PWM))));

    //
    // Disable the timers.
    //
    HWREG(ui32Base + GPTIMER_O_CTL) &= ~(GPTIMER_CTL_TAEN | GPTIMER_CTL_TBEN);

    //
    // Set the global timer configuration.
    //
    HWREG(ui32Base + GPTIMER_O_CFG) = ui32Config >> 24;

    //
    // Set the configuration of the A and B timers.  Note that the B timer
    // configuration is ignored by the hardware in 32-bit modes.
    //
    HWREG(ui32Base + GPTIMER_O_TAMR) = (ui32Config & 255) | GPTIMER_TAMR_TAPWMIE;
    HWREG(ui32Base + GPTIMER_O_TBMR) =
        ((ui32Config >> 8) & 255) | GPTIMER_TBMR_TBPWMIE;
}

//*****************************************************************************
//
//! Controls the output level
//!
//! \param ui32Base is the base address of the timer module.
//! \param ui32Timer specifies the timer(s) to adjust; must be one of \b GPTIMER_A,
//! \b GPTIMER_B, or \b GPTIMER_BOTH.
//! \param bInvert specifies the output level.
//!
//! This function sets the PWM output level for the specified timer.  If the
//! \e bInvert parameter is \b true, then the timer's output is made active
//! low; otherwise, it is made active high.
//!
//! \return None
//
//*****************************************************************************
void
TimerControlLevel(uint32_t ui32Base, uint32_t ui32Timer,
                  bool bInvert)
{
    //
    // Check the arguments.
    //
    ASSERT(TimerBaseValid(ui32Base));
    ASSERT((ui32Timer == GPTIMER_A) || (ui32Timer == GPTIMER_B) ||
           (ui32Timer == GPTIMER_BOTH));

    //
    // Set the output levels as requested.
    //
    ui32Timer &= GPTIMER_CTL_TAPWML | GPTIMER_CTL_TBPWML;
    HWREG(ui32Base + GPTIMER_O_CTL) = (bInvert ?
                                       (HWREG(ui32Base + GPTIMER_O_CTL) | ui32Timer) :
                                       (HWREG(ui32Base + GPTIMER_O_CTL) & ~(ui32Timer)));
}

//*****************************************************************************
//
//! Enables or disables the trigger output
//!
//! \param ui32Base is the base address of the timer module.
//! \param ui32Timer specifies the timer to adjust; must be one of \b GPTIMER_A,
//! \b GPTIMER_B, or \b GPTIMER_BOTH.
//! \param bEnable specifies the desired trigger state.
//!
//! This function controls the trigger output for the specified timer.  If the
//! \e bEnable parameter is \b true, then the timer's output trigger is
//! enabled; otherwise it is disabled.
//!
//! \return None
//
//*****************************************************************************
void
TimerControlTrigger(uint32_t ui32Base, uint32_t ui32Timer,
                    bool bEnable)
{
    //
    // Check the arguments.
    //
    ASSERT(TimerBaseValid(ui32Base));
    ASSERT((ui32Timer == GPTIMER_A) || (ui32Timer == GPTIMER_B) ||
           (ui32Timer == GPTIMER_BOTH));

    //
    // Set the trigger output as requested.
    //
    ui32Timer &= GPTIMER_CTL_TAOTE | GPTIMER_CTL_TBOTE;
    HWREG(ui32Base + GPTIMER_O_CTL) = (bEnable ?
                                       (HWREG(ui32Base + GPTIMER_O_CTL) | ui32Timer) :
                                       (HWREG(ui32Base + GPTIMER_O_CTL) & ~(ui32Timer)));
}

//*****************************************************************************
//
//! Controls the event type
//!
//! \param ui32Base is the base address of the timer module.
//! \param ui32Timer specifies the timer(s) to be adjusted; must be one of
//! \b GPTIMER_A, \b GPTIMER_B, or \b GPTIMER_BOTH.
//! \param ui32Event specifies the type of event; must be one of
//! \b GPTIMER_EVENT_POS_EDGE, \b GPTIMER_EVENT_NEG_EDGE, or
//! \b GPTIMER_EVENT_BOTH_EDGES.
//!
//! This function sets the signal edge(s) that triggers the timer when in
//! capture mode.
//!
//! \return None
//
//*****************************************************************************
void
TimerControlEvent(uint32_t ui32Base, uint32_t ui32Timer,
                  uint32_t ui32Event)
{
    //
    // Check the arguments.
    //
    ASSERT(TimerBaseValid(ui32Base));
    ASSERT((ui32Timer == GPTIMER_A) || (ui32Timer == GPTIMER_B) ||
           (ui32Timer == GPTIMER_BOTH));

    //
    // Set the event type.
    //
    ui32Timer &= GPTIMER_CTL_TAEVENT_M | GPTIMER_CTL_TBEVENT_M;
    HWREG(ui32Base + GPTIMER_O_CTL) = ((HWREG(ui32Base + GPTIMER_O_CTL) & ~ui32Timer) |
                                       (ui32Event & ui32Timer));
}

//*****************************************************************************
//
//! Controls the stall handling
//!
//! \param ui32Base is the base address of the timer module.
//! \param ui32Timer specifies the timer(s) to be adjusted; must be one of
//! \b GPTIMER_A, \b GPTIMER_B, or \b GPTIMER_BOTH.
//! \param bStall specifies the response to a stall signal.
//!
//! This function controls the stall response for the specified timer.  If the
//! \e bStall parameter is \b true, then the timer stops counting if the
//! processor enters debug mode; otherwise the timer keeps running while in
//! debug mode.
//!
//! \return None
//
//*****************************************************************************
void
TimerControlStall(uint32_t ui32Base, uint32_t ui32Timer,
                  bool bStall)
{
    //
    // Check the arguments.
    //
    ASSERT(TimerBaseValid(ui32Base));
    ASSERT((ui32Timer == GPTIMER_A) || (ui32Timer == GPTIMER_B) ||
           (ui32Timer == GPTIMER_BOTH));

    //
    // Set the stall mode.
    //
    ui32Timer &= GPTIMER_CTL_TASTALL | GPTIMER_CTL_TBSTALL;
    HWREG(ui32Base + GPTIMER_O_CTL) = (bStall ?
                                       (HWREG(ui32Base + GPTIMER_O_CTL) | ui32Timer) :
                                       (HWREG(ui32Base + GPTIMER_O_CTL) & ~(ui32Timer)));
}

//*****************************************************************************
//
//! Controls the wait on trigger handling
//!
//! \param ui32Base is the base address of the timer module.
//! \param ui32Timer specifies the timer(s) to be adjusted; must be one of
//! \b GPTIMER_A, \b GPTIMER_B, or \b GPTIMER_BOTH.
//! \param bWait specifies if the timer should wait for a trigger input.
//!
//! This function controls whether or not a timer waits for a trigger input to
//! start counting.  When enabled, the previous timer in the trigger chain must
//! count to its timeout in order for this timer to start counting.  Refer to
//! the part's data sheet for a description of the trigger chain.
//!
//! \note This functionality is not available on all parts.
//!
//! \return None
//
//*****************************************************************************
void
TimerControlWaitOnTrigger(uint32_t ui32Base, uint32_t ui32Timer,
                          bool bWait)
{
    //
    // Check the arguments.
    //
    ASSERT(TimerBaseValid(ui32Base));
    ASSERT((ui32Timer == GPTIMER_A) || (ui32Timer == GPTIMER_B) ||
           (ui32Timer == GPTIMER_BOTH));

    //
    // Set the wait on trigger mode for timer A.
    //
    if((ui32Timer & GPTIMER_A) != 0)
    {
        if(bWait)
        {
            HWREG(ui32Base + GPTIMER_O_TAMR) |= GPTIMER_TAMR_TAWOT;
        }
        else
        {
            HWREG(ui32Base + GPTIMER_O_TAMR) &= ~(GPTIMER_TAMR_TAWOT);
        }
    }

    //
    // Set the wait on trigger mode for timer B.
    //
    if((ui32Timer & GPTIMER_B) != 0)
    {
        if(bWait)
        {
            HWREG(ui32Base + GPTIMER_O_TBMR) |= GPTIMER_TBMR_TBWOT;
        }
        else
        {
            HWREG(ui32Base + GPTIMER_O_TBMR) &= ~(GPTIMER_TBMR_TBWOT);
        }
    }
}

//*****************************************************************************
//
//! Set the timer prescale value
//!
//! \param ui32Base is the base address of the timer module.
//! \param ui32Timer specifies the timer(s) to adjust; must be one of \b GPTIMER_A,
//! \b GPTIMER_B, or \b GPTIMER_BOTH.
//! \param ui32Value is the timer prescale value; must be between 0 and 255,
//! inclusive.
//!
//! This function sets the value of the input clock prescaler.  The prescaler
//! is only operational when in 16-bit mode and is used to extend the range of
//! the 16-bit timer modes.
//!
//! \note The availability of the prescaler varies with the timer mode in use.
//! Please consult the datasheet for the part you are using
//! to determine whether this support is available.
//!
//! \return None
//
//*****************************************************************************
void
TimerPrescaleSet(uint32_t ui32Base, uint32_t ui32Timer,
                 uint32_t ui32Value)
{
    //
    // Check the arguments.
    //
    ASSERT(TimerBaseValid(ui32Base));
    ASSERT((ui32Timer == GPTIMER_A) || (ui32Timer == GPTIMER_B) ||
           (ui32Timer == GPTIMER_BOTH));
    ASSERT(ui32Value < 256);

    //
    // Set the timer A prescaler if requested.
    //
    if(ui32Timer & GPTIMER_A)
    {
        HWREG(ui32Base + GPTIMER_O_TAPR) = ui32Value;
    }

    //
    // Set the timer B prescaler if requested.
    //
    if(ui32Timer & GPTIMER_B)
    {
        HWREG(ui32Base + GPTIMER_O_TBPR) = ui32Value;
    }
}

//*****************************************************************************
//
//! Get the timer prescale value
//!
//! \param ui32Base is the base address of the timer module.
//! \param ui32Timer specifies the timer; must be one of \b GPTIMER_A or
//! \b GPTIMER_B.
//!
//! This function gets the value of the input clock prescaler.  The prescaler
//! is only operational when in 16-bit mode and is used to extend the range of
//! the 16-bit timer modes.
//!
//! \note The availability of the prescaler varies with the timer mode in use.
//! Please consult the datasheet for the part you are using
//! to determine whether this support is available.
//!
//! \return The value of the timer prescaler
//
//*****************************************************************************
uint32_t
TimerPrescaleGet(uint32_t ui32Base, uint32_t ui32Timer)
{
    //
    // Check the arguments.
    //
    ASSERT(TimerBaseValid(ui32Base));
    ASSERT((ui32Timer == GPTIMER_A) || (ui32Timer == GPTIMER_B) ||
           (ui32Timer == GPTIMER_BOTH));

    //
    // Return the appropriate prescale value.
    //
    return((ui32Timer == GPTIMER_A) ? HWREG(ui32Base + GPTIMER_O_TAPR) :
           HWREG(ui32Base + GPTIMER_O_TBPR));
}

//*****************************************************************************
//
//! Set the timer prescale match value
//!
//! \param ui32Base is the base address of the timer module.
//! \param ui32Timer specifies the timer(s) to adjust; must be one of
//! \b GPTIMER_A, \b GPTIMER_B, or \b GPTIMER_BOTH.
//! \param ui32Value is the timer prescale match value; must be between 0 and
//! 255, inclusive.
//!
//! This function sets the value of the input clock prescaler match value.
//! When in a 16-bit mode that uses the counter match and the prescaler, the
//! prescale match effectively extends the range of the counter to 24-bits.
//!
//! \note The availability of the prescaler match varies with the timer mode
//! in use.  Please consult the datasheet for the part you are using to
//! determine whether this support is available.
//!
//! \return None
//
//*****************************************************************************
void
TimerPrescaleMatchSet(uint32_t ui32Base, uint32_t ui32Timer,
                      uint32_t ui32Value)
{
    //
    // Check the arguments.
    //
    ASSERT(TimerBaseValid(ui32Base));
    ASSERT((ui32Timer == GPTIMER_A) || (ui32Timer == GPTIMER_B) ||
           (ui32Timer == GPTIMER_BOTH));
    ASSERT(ui32Value < 256);

    //
    // Set the timer A prescale match if requested.
    //
    if(ui32Timer & GPTIMER_A)
    {
        HWREG(ui32Base + GPTIMER_O_TAPMR) = ui32Value;
    }

    //
    // Set the timer B prescale match if requested.
    //
    if(ui32Timer & GPTIMER_B)
    {
        HWREG(ui32Base + GPTIMER_O_TBPMR) = ui32Value;
    }
}

//*****************************************************************************
//
//! Get the timer prescale match value
//!
//! \param ui32Base is the base address of the timer module.
//! \param ui32Timer specifies the timer; must be one of \b GPTIMER_A or
//! \b GPTIMER_B.
//!
//! This function gets the value of the input clock prescaler match value.
//! When in a 16-bit mode that uses the counter match and prescaler, the
//! prescale match effectively extends the range of the counter to 24-bits.
//!
//! \note The availability of the prescaler match varies with the timer mode
//! in use.  Please consult the datasheet for the part you are using to
//! determine whether this support is available.
//!
//! \return The value of the timer prescale match.
//
//*****************************************************************************
uint32_t
TimerPrescaleMatchGet(uint32_t ui32Base, uint32_t ui32Timer)
{
    //
    // Check the arguments.
    //
    ASSERT(TimerBaseValid(ui32Base));
    ASSERT((ui32Timer == GPTIMER_A) || (ui32Timer == GPTIMER_B) ||
           (ui32Timer == GPTIMER_BOTH));

    //
    // Return the appropriate prescale match value.
    //
    return((ui32Timer == GPTIMER_A) ? HWREG(ui32Base + GPTIMER_O_TAPMR) :
           HWREG(ui32Base + GPTIMER_O_TBPMR));
}

//*****************************************************************************
//
//! Sets the timer load value
//!
//! \param ui32Base is the base address of the timer module.
//! \param ui32Timer specifies the timer(s) to adjust; must be one of:
//! \b GPTIMER_A, \b GPTIMER_B, or \b GPTIMER_BOTH. Only \b GPTIMER_A should
//! be used when the timer is configured for 32-bit operation.
//! \param ui32Value is the load value.
//!
//! This function sets the timer load value; if the timer is running then the
//! value will be immediately loaded into the timer.
//!
//! \return None
//
//*****************************************************************************
void
TimerLoadSet(uint32_t ui32Base, uint32_t ui32Timer,
             uint32_t ui32Value)
{
    //
    // Check the arguments.
    //
    ASSERT(TimerBaseValid(ui32Base));
    ASSERT((ui32Timer == GPTIMER_A) || (ui32Timer == GPTIMER_B) ||
           (ui32Timer == GPTIMER_BOTH));

    //
    // Set the timer A load value if requested.
    //
    if(ui32Timer & GPTIMER_A)
    {
        HWREG(ui32Base + GPTIMER_O_TAILR) = ui32Value;
    }

    //
    // Set the timer B load value if requested.
    //
    if(ui32Timer & GPTIMER_B)
    {
        HWREG(ui32Base + GPTIMER_O_TBILR) = ui32Value;
    }
}

//*****************************************************************************
//
//! Gets the timer load value
//!
//! \param ui32Base is the base address of the timer module.
//! \param ui32Timer specifies the timer; must be one of \b GPTIMER_A or
//! \b GPTIMER_B.  Only \b GPTIMER_A should be used when the timer is
//! configured for 32-bit operation.
//!
//! This function gets the currently programmed interval load value for the
//! specified timer.
//!
//! \return Returns the load value for the timer.
//
//*****************************************************************************
uint32_t
TimerLoadGet(uint32_t ui32Base, uint32_t ui32Timer)
{
    //
    // Check the arguments.
    //
    ASSERT(TimerBaseValid(ui32Base));
    ASSERT((ui32Timer == GPTIMER_A) || (ui32Timer == GPTIMER_B));

    //
    // Return the appropriate load value.
    //
    return((ui32Timer == GPTIMER_A) ? HWREG(ui32Base + GPTIMER_O_TAILR) :
           HWREG(ui32Base + GPTIMER_O_TBILR));
}



//*****************************************************************************
//
//! Gets the current timer value
//!
//! \param ui32Base is the base address of the timer module.
//! \param ui32Timer specifies the timer; must be one of \b GPTIMER_A or
//! \b GPTIMER_B.  Only \b GPTIMER_A should be used when the timer is
//! configured for 32-bit operation.
//!
//! This function reads the current value of the specified timer.
//!
//! \return Returns the current value of the timer.
//
//*****************************************************************************
uint32_t
TimerValueGet(uint32_t ui32Base, uint32_t ui32Timer)
{
    //
    // Check the arguments.
    //
    ASSERT(TimerBaseValid(ui32Base));
    ASSERT((ui32Timer == GPTIMER_A) || (ui32Timer == GPTIMER_B));

    //
    // Return the appropriate timer value.
    //
    return((ui32Timer == GPTIMER_A) ? HWREG(ui32Base + GPTIMER_O_TAR) :
           HWREG(ui32Base + GPTIMER_O_TBR));
}

//*****************************************************************************
//
//! Sets the timer match value
//!
//! \param ui32Base is the base address of the timer module.
//! \param ui32Timer specifies the timer(s) to adjust; must be one of
//! \b GPTIMER_A, \b GPTIMER_B, or \b GPTIMER_BOTH.  Only \b GPTIMER_A should
//! be used when the timer is configured for 32-bit operation.
//! \param ui32Value is the match value.
//!
//! This function sets the match value for a timer.  This is used in capture
//! count mode to determine when to interrupt the processor and in PWM mode to
//! determine the duty cycle of the output signal.
//!
//! \return None
//
//*****************************************************************************
void
TimerMatchSet(uint32_t ui32Base, uint32_t ui32Timer,
              uint32_t ui32Value)
{
    //
    // Check the arguments.
    //
    ASSERT(TimerBaseValid(ui32Base));
    ASSERT((ui32Timer == GPTIMER_A) || (ui32Timer == GPTIMER_B) ||
           (ui32Timer == GPTIMER_BOTH));

    //
    // Set the timer A match value if requested.
    //
    if(ui32Timer & GPTIMER_A)
    {
        HWREG(ui32Base + GPTIMER_O_TAMATCHR) = ui32Value;
    }

    //
    // Set the timer B match value if requested.
    //
    if(ui32Timer & GPTIMER_B)
    {
        HWREG(ui32Base + GPTIMER_O_TBMATCHR) = ui32Value;
    }
}

//*****************************************************************************
//
//! Gets the timer match value
//!
//! \param ui32Base is the base address of the timer module.
//! \param ui32Timer specifies the timer; must be one of \b GPTIMER_A or
//! \b GPTIMER_B.  Only \b GPTIMER_A should be used when the timer is
//! configured for 32-bit operation.
//!
//! This function gets the match value for the specified timer.
//!
//! \return Returns the match value for the timer.
//
//*****************************************************************************
uint32_t
TimerMatchGet(uint32_t ui32Base, uint32_t ui32Timer)
{
    //
    // Check the arguments.
    //
    ASSERT(TimerBaseValid(ui32Base));
    ASSERT((ui32Timer == GPTIMER_A) || (ui32Timer == GPTIMER_B));

    //
    // Return the appropriate match value.
    //
    return((ui32Timer == GPTIMER_A) ? HWREG(ui32Base + GPTIMER_O_TAMATCHR) :
           HWREG(ui32Base + GPTIMER_O_TBMATCHR));
}

//*****************************************************************************
//
//! Registers an interrupt handler for the timer interrupt
//!
//! \param ui32Base is the base address of the timer module.
//! \param ui32Timer specifies the timer(s); must be one of \b GPTIMER_A,
//! \b GPTIMER_B, or \b GPTIMER_BOTH.
//! \param pfnHandler is a pointer to the function to be called when the timer
//! interrupt occurs.
//!
//! This function sets the handler to be called when a timer interrupt occurs.
//! In addition, this function enables the global interrupt in the interrupt
//! controller; specific timer interrupts must be enabled via TimerIntEnable().
//! It is the interrupt handler's responsibility to clear the interrupt source
//! via TimerIntClear().
//!
//! \sa See IntRegister() for important information about registering interrupt
//! handlers.
//!
//! \return None
//
//*****************************************************************************
void
TimerIntRegister(uint32_t ui32Base, uint32_t ui32Timer,
                 void (*pfnHandler)(void))
{
    //
    // Check the arguments.
    //
    ASSERT(TimerBaseValid(ui32Base));
    ASSERT((ui32Timer == GPTIMER_A) || (ui32Timer == GPTIMER_B) ||
           (ui32Timer == GPTIMER_BOTH));

    //
    // Get the interrupt number for this timer module.
    //
    ui32Base = ((ui32Base == GPTIMER0_BASE) ? INT_TIMER0A :
                ((ui32Base == GPTIMER1_BASE) ? INT_TIMER1A :
                 ((ui32Base == GPTIMER2_BASE) ? INT_TIMER2A : INT_TIMER3A)));

    //
    // Register an interrupt handler for timer A if requested.
    //
    if(ui32Timer & GPTIMER_A)
    {
        //
        // Register the interrupt handler.
        //
        IntRegister(ui32Base, pfnHandler);

        //
        // Enable the interrupt.
        //
        IntEnable(ui32Base);
    }

    //
    // Register an interrupt handler for timer B if requested.
    //
    if(ui32Timer & GPTIMER_B)
    {
        //
        // Register the interrupt handler.
        //
        IntRegister(ui32Base + 1, pfnHandler);

        //
        // Enable the interrupt.
        //
        IntEnable(ui32Base + 1);
    }
}

//*****************************************************************************
//
//! Unregisters an interrupt handler for the timer interrupt
//!
//! \param ui32Base is the base address of the timer module.
//! \param ui32Timer specifies the timer(s); must be one of \b GPTIMER_A,
//! \b GPTIMER_B, or \b GPTIMER_BOTH.
//!
//! This function clears the handler to be called when a timer interrupt
//! occurs.  This function also masks off the interrupt in the interrupt
//! controller so that the interrupt handler no longer is called.
//!
//! \sa See IntRegister() for important information about registering interrupt
//! handlers.
//!
//! \return None
//
//*****************************************************************************
void
TimerIntUnregister(uint32_t ui32Base, uint32_t ui32Timer)
{
    //
    // Check the arguments.
    //
    ASSERT(TimerBaseValid(ui32Base));
    ASSERT((ui32Timer == GPTIMER_A) || (ui32Timer == GPTIMER_B) ||
           (ui32Timer == GPTIMER_BOTH));

    //
    // Get the interrupt number for this timer module.
    //
    ui32Base = ((ui32Base == GPTIMER0_BASE) ? INT_TIMER0A :
                ((ui32Base == GPTIMER1_BASE) ? INT_TIMER1A :
                 ((ui32Base == GPTIMER2_BASE) ? INT_TIMER2A : INT_TIMER3A)));

    //
    // Unregister the interrupt handler for timer A if requested.
    //
    if(ui32Timer & GPTIMER_A)
    {
        //
        // Disable the interrupt.
        //
        IntDisable(ui32Base);

        //
        // Unregister the interrupt handler.
        //
        IntUnregister(ui32Base);
    }

    //
    // Unregister the interrupt handler for timer B if requested.
    //
    if(ui32Timer & GPTIMER_B)
    {
        //
        // Disable the interrupt.
        //
        IntDisable(ui32Base + 1);

        //
        // Unregister the interrupt handler.
        //
        IntUnregister(ui32Base + 1);
    }
}

//*****************************************************************************
//
//! Enables individual timer interrupt sources
//!
//! \param ui32Base is the base address of the timer module.
//! \param ui32IntFlags is the bit mask of the interrupt sources to be enabled.
//!
//! Enables the indicated timer interrupt sources.  Only the sources that are
//! enabled can be reflected to the processor interrupt; disabled sources have
//! no effect on the processor.
//!
//! The \e ui32IntFlags parameter must be the logical OR of any combination of
//! the following:
//!
//! - \b GPTIMER_CAPB_EVENT  - Capture B event interrupt
//! - \b GPTIMER_CAPB_MATCH  - Capture B match interrupt
//! - \b GPTIMER_TIMB_TIMEOUT  - Timer B timeout interrupt
//! - \b GPTIMER_CAPA_EVENT  - Capture A event interrupt
//! - \b GPTIMER_CAPA_MATCH  - Capture A match interrupt
//! - \b GPTIMER_TIMA_TIMEOUT  - Timer A timeout interrupt
//!
//! \return None
//
//*****************************************************************************
void
TimerIntEnable(uint32_t ui32Base, uint32_t ui32IntFlags)
{
    //
    // Check the arguments.
    //
    ASSERT(TimerBaseValid(ui32Base));

    //
    // Enable the specified interrupts.
    //
    HWREG(ui32Base + GPTIMER_O_IMR) |= ui32IntFlags;
}

//*****************************************************************************
//
//! Disables individual timer interrupt sources
//!
//! \param ui32Base is the base address of the timer module.
//! \param ui32IntFlags is the bit mask of the interrupt sources to be disabled.
//!
//! Disables the indicated timer interrupt sources.  Only the sources that are
//! enabled can be reflected to the processor interrupt; disabled sources have
//! no effect on the processor.
//!
//! The \e ui32IntFlags parameter has the same definition as the \e ui32IntFlags
//! parameter to TimerIntEnable().
//!
//! \return None
//
//*****************************************************************************
void
TimerIntDisable(uint32_t ui32Base, uint32_t ui32IntFlags)
{
    //
    // Check the arguments.
    //
    ASSERT(TimerBaseValid(ui32Base));

    //
    // Disable the specified interrupts.
    //
    HWREG(ui32Base + GPTIMER_O_IMR) &= ~(ui32IntFlags);
}

//*****************************************************************************
//
//! Gets the current interrupt status
//!
//! \param ui32Base is the base address of the timer module.
//! \param bMasked is false if the raw interrupt status is required and true if
//! the masked interrupt status is required.
//!
//! This function returns the interrupt status for the timer module.  Either
//! the raw interrupt status or the status of interrupts that are allowed to
//! reflect to the processor can be returned.
//!
//! \return The current interrupt status, enumerated as a bit field of
//! values described in TimerIntEnable().
//
//*****************************************************************************
uint32_t
TimerIntStatus(uint32_t ui32Base, bool bMasked)
{
    //
    // Check the arguments.
    //
    ASSERT(TimerBaseValid(ui32Base));

    //
    // Return either the interrupt status or the raw interrupt status as
    // requested.
    //
    return(bMasked ? HWREG(ui32Base + GPTIMER_O_MIS) :
           HWREG(ui32Base + GPTIMER_O_RIS));
}

//*****************************************************************************
//
//! Clears timer interrupt sources
//!
//! \param ui32Base is the base address of the timer module.
//! \param ui32IntFlags is a bit mask of the interrupt sources to be cleared.
//!
//! The specified timer interrupt sources are cleared, so that they no longer
//! assert.  This function must be called in the interrupt handler to keep the
//! interrupt from being triggered again immediately upon exit.
//!
//! The \e ui32IntFlags parameter has the same definition as the \e ui32IntFlags
//! parameter to TimerIntEnable().
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
TimerIntClear(uint32_t ui32Base, uint32_t ui32IntFlags)
{
    //
    // Check the arguments.
    //
    ASSERT(TimerBaseValid(ui32Base));

    //
    // Clear the requested interrupt sources.
    //
    HWREG(ui32Base + GPTIMER_O_ICR) = ui32IntFlags;
}

//*****************************************************************************
//
//! Synchronizes the counters in a set of timers
//!
//! \param ui32Base is the base address of the timer module.  This must be the
//! base address of Timer0 (in other words, \b GPTIMER0_BASE).
//! \param ui32Timers is the set of timers to synchronize.
//!
//! This function will synchronize the counters in a specified set of timers.
//! When a timer is running in half-width mode, each half can be included or
//! excluded in the synchronization event.  When a timer is running in
//! full-width mode, only the A timer can be synchronized (specifying the B
//! timer has no effect).
//!
//! The \e ui32Timers parameter is the logical OR of any of the following
//! defines:
//!
//! - \b GPTIMER_0A_SYNC
//! - \b GPTIMER_0B_SYNC
//! - \b GPTIMER_1A_SYNC
//! - \b GPTIMER_1B_SYNC
//! - \b GPTIMER_2A_SYNC
//! - \b GPTIMER_2B_SYNC
//! - \b GPTIMER_3A_SYNC
//! - \b GPTIMER_3B_SYNC
//!
//! \note This functionality is not available on all parts.
//!
//! \return None
//
//*****************************************************************************
void
TimerSynchronize(uint32_t ui32Base, uint32_t ui32Timers)
{
    //
    // Check the arguments.
    //
    ASSERT(ui32Base == GPTIMER0_BASE);

    //
    // Synchronize the specified timers.
    //
    HWREG(ui32Base + GPTIMER_O_SYNC) = ui32Timers;
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
