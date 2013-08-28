/**
\brief CC2538-specific definition of the "board" bsp module.

\author Xavier Vilajosana <xvilajosana@eecs.berkeley.edu>, August 2013.
*/


#include "board.h"
#include "bsp_timer.h"

// bsp modules
#include "leds.h"
#include "hw_ioc.h"             // Access to IOC register defines
#include "hw_ssi.h"             // Access to SSI register defines
#include "ioc.h"                // Access to driverlib ioc fns
#include "gpio.h"               // Access to driverlib gpio fns
#include "sys_ctrl.h"           // Access to driverlib SysCtrl fns
#include "interrupt.h"          // Access to driverlib interrupt fns
#include "bsp_timer.h"


//=========================== variables =======================================

//=========================== prototypes ======================================
void clockInit(uint32_t ui32SysClockSpeed);
//=========================== main ============================================

extern int mote_main();

int main() {
   return mote_main();
}

//=========================== public ==========================================

void board_init() {

   clockInit(SYS_CTRL_32MHZ);

   leds_init();
  // bsp_timer_init();

}

void board_sleep() {
	SysCtrlSleep();
}

void board_reset() {

}

//=========================== private =========================================


/**************************************************************************//**
* @brief    Function initializes the CC2538 clocks and I/O for use on
*           SmartRF06EB.
*
*           The function assumes an external crystal oscillator to be available
*           to the CC2538. The CC2538 system clock is set to the frequency given
*           by input argument \c ui32SysClockSpeed. The clock speed of other
*           internal clocks are set to the maximum value allowed based on the
*           system clock speed given by \c ui32SysClockSpeed.
*
*           If the value of \c ui32SysClockSpeed is invalid, the system clock
*           will be set to the highest allowed value.
*
* @param    ui32SysClockSpeed   The system clock speed in Hz. Must be one of
*                               the following:
*           \li \c SYS_CTRL_32MHZ
*           \li \c SYS_CTRL_16MHZ
*           \li \c SYS_CTRL_8MHZ
*           \li \c SYS_CTRL_4MHZ
*           \li \c SYS_CTRL_2MHZ
*           \li \c SYS_CTRL_1MHZ
*           \li \c SYS_CTRL_500KHZ
*           \li \c SYS_CTRL_250KHZ
*
* @return   None
******************************************************************************/
void clockInit(uint32_t ui32SysClockSpeed)
{
    uint32_t ui32SysDiv;

    //
    // Disable global interrupts
    //
    bool bIntDisabled = IntMasterDisable();

    //
    // Determine sys clock divider and realtime clock
    //
    switch(ui32SysClockSpeed)
    {
    case SYS_CTRL_250KHZ:
        ui32SysDiv = SYS_CTRL_SYSDIV_250KHZ;
        break;
    case SYS_CTRL_500KHZ:
        ui32SysDiv = SYS_CTRL_SYSDIV_500KHZ;
        break;
    case SYS_CTRL_1MHZ:
        ui32SysDiv = SYS_CTRL_SYSDIV_1MHZ;
        break;
    case SYS_CTRL_2MHZ:
        ui32SysDiv = SYS_CTRL_SYSDIV_2MHZ;
        break;
    case SYS_CTRL_4MHZ:
        ui32SysDiv = SYS_CTRL_SYSDIV_4MHZ;
        break;
    case SYS_CTRL_8MHZ:
        ui32SysDiv = SYS_CTRL_SYSDIV_8MHZ;
        break;
    case SYS_CTRL_16MHZ:
        ui32SysDiv = SYS_CTRL_SYSDIV_16MHZ;
        break;
    case SYS_CTRL_32MHZ:
    default:
        ui32SysDiv = SYS_CTRL_SYSDIV_32MHZ;
        break;
    }

    //
    // Set system clock and realtime clock
    //
    SysCtrlClockSet(false, false, ui32SysDiv);

    //
    // Set IO clock to the same as system clock
    //
    SysCtrlIOClockSet(ui32SysDiv);

    //
    // Re-enable interrupt if initially enabled.
    //
    if(!bIntDisabled)
    {
        IntMasterEnable();
    }
}
