/**
\brief CC2538-specific definition of the "board" bsp module.

\author Xavier Vilajosana <xvilajosana@eecs.berkeley.edu>, August 2013.
*/


#include "board.h"

// bsp modules
#include "leds.h"
#include "hw_ioc.h"             // Access to IOC register defines
#include "hw_ssi.h"             // Access to SSI register defines
#include "hw_sys_ctrl.h"        // Clocking control
#include "ioc.h"                // Access to driverlib ioc fns
#include "gpio.h"               // Access to driverlib gpio fns
#include "sys_ctrl.h"           // Access to driverlib SysCtrl fns
#include "interrupt.h"          // Access to driverlib interrupt fns
#include "bsp_timer.h"
#include "radiotimer.h"


//=========================== variables =======================================

//=========================== prototypes ======================================
void clockInit(uint32_t ui32SysClockSpeed);
void SysCtrlDeepSleepSetting(void);
void SysCtrlSleepSetting(void);
void SysCtrlWakeupSetting(void);
//=========================== main ============================================

extern int mote_main();

int main() {
   return mote_main();
}

//=========================== public ==========================================

void board_init() {
   clockInit(SYS_CTRL_32MHZ);

   leds_init();
   debugpins_init();
   bsp_timer_init();
   radiotimer_init();

}

void board_sleep() {
	SysCtrlSleep();
}

void board_reset() {
	SysCtrlReset();
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
    // use the 32khz external crystal
    //
    SysCtrlClockSet(false, false, ui32SysDiv);

    //
    // Set IO clock to the same as system clock
    //
    SysCtrlIOClockSet(ui32SysDiv);

    while ( !((HWREG(SYS_CTRL_CLOCK_STA)) & (SYS_CTRL_CLOCK_STA_XOSC_STB)));

    //define what peripherals run at each mode.
    SysCtrlDeepSleepSetting();
    SysCtrlSleepSetting();
    SysCtrlRunSetting();
    SysCtrlWakeupSetting();

    //
    // Re-enable interrupt if initially enabled.
    //
    if(!bIntDisabled)
    {
        IntMasterEnable();
    }
}


void SysCtrlDeepSleepSetting(void)
{
  /* Disable General Purpose Timers 0, 1, 2, 3 during deep sleep */
  SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_GPT0);
  SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_GPT1);
  SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_GPT2);
  SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_GPT3);

  /* Disable SSI 0, 1 during deep sleep */
  SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_SSI0);
  SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_SSI1);

  /* Disable UART 0, 1 during deep sleep */
  SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_UART0);
  SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_UART1);

  /* Disable I2C, PKA, AES during deep sleep */
  SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_I2C);
  SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_PKA);
  SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_AES);

  /*
   * Disable RFC during deep sleep. Please note that this setting is
   * only valid for PG2.0. For PG1.0 this is just a dummy instruction.
   */
  SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_RFC);
}

void SysCtrlSleepSetting(void)
{
  /* Disable General Purpose Timers 0, 1, 2, 3 during sleep */
  SysCtrlPeripheralSleepDisable(SYS_CTRL_PERIPH_GPT0);
  SysCtrlPeripheralSleepDisable(SYS_CTRL_PERIPH_GPT1);
  SysCtrlPeripheralSleepDisable(SYS_CTRL_PERIPH_GPT2);
  SysCtrlPeripheralSleepDisable(SYS_CTRL_PERIPH_GPT3);

  /* Disable SSI 0, 1 during sleep */
  SysCtrlPeripheralSleepDisable(SYS_CTRL_PERIPH_SSI0);
  SysCtrlPeripheralSleepDisable(SYS_CTRL_PERIPH_SSI1);

  /* Disable UART 0, 1 during sleep */
  SysCtrlPeripheralSleepDisable(SYS_CTRL_PERIPH_UART0);
  SysCtrlPeripheralSleepDisable(SYS_CTRL_PERIPH_UART1);

  /* Disable I2C, PKA, AES during sleep */
  SysCtrlPeripheralSleepDisable(SYS_CTRL_PERIPH_I2C);
  SysCtrlPeripheralSleepDisable(SYS_CTRL_PERIPH_PKA);
  SysCtrlPeripheralSleepDisable(SYS_CTRL_PERIPH_AES);

  /*
   * Disable RFC during sleep. Please note that this setting is
   * only valid for PG2.0. For PG1.0 this is just a dummy instruction.
   */
  SysCtrlPeripheralSleepDisable(SYS_CTRL_PERIPH_RFC);
}


void SysCtrlRunSetting(void)
{
  /* Enable General Purpose Timers 0, 1, 2, 3 when running */
  SysCtrlPeripheralEnable(SYS_CTRL_PERIPH_GPT0);
  SysCtrlPeripheralEnable(SYS_CTRL_PERIPH_GPT1);
  SysCtrlPeripheralEnable(SYS_CTRL_PERIPH_GPT2);
  SysCtrlPeripheralEnable(SYS_CTRL_PERIPH_GPT3);

  /* Enable SSI 0, 1 when running */
  SysCtrlPeripheralEnable(SYS_CTRL_PERIPH_SSI0);
  SysCtrlPeripheralEnable(SYS_CTRL_PERIPH_SSI1);

  /* Enable UART 0, 1 when running */
  SysCtrlPeripheralEnable(SYS_CTRL_PERIPH_UART0);
  SysCtrlPeripheralEnable(SYS_CTRL_PERIPH_UART1);

  SysCtrlPeripheralReset(SYS_CTRL_PERIPH_AES);
  SysCtrlPeripheralReset(SYS_CTRL_PERIPH_PKA);

  /* Enable I2C, AES and PKA running */
  SysCtrlPeripheralEnable(SYS_CTRL_PERIPH_I2C);
  SysCtrlPeripheralEnable(SYS_CTRL_PERIPH_PKA);
  SysCtrlPeripheralEnable(SYS_CTRL_PERIPH_AES);

  /*
   * Enable RFC during run. Please note that this setting is
   * only valid for PG2.0. For PG1.0 since the RFC is always on,
   * this is only a dummy  instruction
   */
  SysCtrlPeripheralEnable(SYS_CTRL_PERIPH_RFC);
}


void SysCtrlWakeupSetting(void)
{
  /* SM Timer can wake up the processor */

  GPIOIntWakeupEnable(GPIO_IWE_SM_TIMER);


}
