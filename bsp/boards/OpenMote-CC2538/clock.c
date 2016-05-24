/**
 * Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Date:   July 2013
 * Description: CC2538-specific definition of the "board" bsp module.
 */

#include <headers/hw_ioc.h>
#include <headers/hw_memmap.h>
#include <headers/hw_ssi.h>
#include <headers/hw_sys_ctrl.h>
#include <headers/hw_types.h>

#include <source/ioc.h>
#include <source/gpio.h>
#include <source/gptimer.h>
#include <source/sys_ctrl.h>
#include <source/interrupt.h>
#include <source/flash.h> 

#include "clock.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== main ============================================

//=========================== public ==========================================

void clock_init(void) {
  /* Configure the 32 kHz pins, PD6 and PD7, for crystal operation */
  /* By default they are configured as GPIOs */
  GPIODirModeSet(GPIO_D_BASE, 0x40, GPIO_DIR_MODE_IN);
  GPIODirModeSet(GPIO_D_BASE, 0x80, GPIO_DIR_MODE_IN);
  IOCPadConfigSet(GPIO_D_BASE, 0x40, IOC_OVERRIDE_ANA);
  IOCPadConfigSet(GPIO_D_BASE, 0x80, IOC_OVERRIDE_ANA);

  /* Set the real-time clock to use the 32 kHz external crystal */
  /* Set the system clock to use the external 32 MHz crystal */
  /* Set the system clock to 32 MHz */
  SysCtrlClockSet(true, false, SYS_CTRL_SYSDIV_32MHZ);

  /* Set the IO clock to operate at 16 MHz */
  /* This way peripherals can run while the system clock is gated */
  SysCtrlIOClockSet(SYS_CTRL_SYSDIV_16MHZ);

  /* Wait until the selected clock configuration is stable */
  while (!((HWREG(SYS_CTRL_CLOCK_STA)) & (SYS_CTRL_CLOCK_STA_XOSC_STB)));

  // Configure the timer to run at 32 MHz and is 32-bit wide
  // The timer is divided by 32, whichs gives a 1 microsecond ticks
  TimerConfigure(GPTIMER2_BASE, GPTIMER_CFG_PERIODIC_UP);
  TimerEnable(GPTIMER2_BASE, GPTIMER_BOTH);
}

/**
 * Returns the current value of the timer
 * The timer is divided by 32, whichs gives a 1 microsecond ticks
 */
uint32_t clock_get(void) {
  uint32_t current;
  
  current = TimerValueGet(GPTIMER2_BASE, GPTIMER_A) >> 5;
  
  return current;
}

/**
 * Returns true if the timer has expired
 * The timer is divided by 32, whichs gives a 1 microsecond ticks
 */
bool clock_expired(uint32_t future) {
  uint32_t current;
  int32_t remaining;

  current = TimerValueGet(GPTIMER2_BASE, GPTIMER_A) >> 5;

  remaining = (int32_t) (future - current);
  
  if (remaining > 0) {
    return false;
  } else {
    return true;
  }
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================
