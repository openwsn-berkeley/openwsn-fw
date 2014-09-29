/**
* Copyright (c) 2014 Atmel Corporation. All rights reserved. 
*  
* Redistribution and use in source and binary forms, with or without 
* modification, are permitted provided that the following conditions are met:
* 
* 1. Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
* 
* 2. Redistributions in binary form must reproduce the above copyright notice, 
* this list of conditions and the following disclaimer in the documentation 
* and/or other materials provided with the distribution.
* 
* 3. The name of Atmel may not be used to endorse or promote products derived 
* from this software without specific prior written permission.  
* 
* 4. This software may only be redistributed and used in connection with an 
* Atmel microcontroller product.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
* GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* 
* 
* 
*/

#ifndef sys_ctrl_h__
#define sys_ctrl_h__

#include "base_type.h"

/* Wait state for internal flash can be changed, 
   based on CPU speed and voltage */
#define INTERNAL_FLASH_WAIT_STATE (2)

/* DFLL Multiplier value */
#define DFLL_MUL_VAL	(1024)

/* DFLL Input clock frequency */
#define DFLL_INPUT_CLOCK GCLK_CLKCTRL_GEN_GCLK2


/**
 * \brief List of available GCLK generators.
 *
 * List of Available GCLK generators. This enum is used in the peripheral
 * device drivers to select the GCLK generator to be used for its operation.
 *
 * The number of GCLK generators available is device dependent.
 */
enum gclk_generator {
	/** GCLK generator channel 0. */
	GCLK_GENERATOR_0,
#if defined(__DOXYGEN__) || (GCLK_GEN_NUM_MSB > 0)
	/** GCLK generator channel 1. */
	GCLK_GENERATOR_1,
#endif
#if defined(__DOXYGEN__) || (GCLK_GEN_NUM_MSB > 1)
	/** GCLK generator channel 2. */
	GCLK_GENERATOR_2,
#endif
#if defined(__DOXYGEN__) || (GCLK_GEN_NUM_MSB > 2)
	/** GCLK generator channel 3. */
	GCLK_GENERATOR_3,
#endif
#if defined(__DOXYGEN__) || (GCLK_GEN_NUM_MSB > 3)
	/** GCLK generator channel 4. */
	GCLK_GENERATOR_4,
#endif
#if defined(__DOXYGEN__) || (GCLK_GEN_NUM_MSB > 4)
	/** GCLK generator channel 5. */
	GCLK_GENERATOR_5,
#endif
#if defined(__DOXYGEN__) || (GCLK_GEN_NUM_MSB > 5)
	/** GCLK generator channel 6. */
	GCLK_GENERATOR_6,
#endif
#if defined(__DOXYGEN__) || (GCLK_GEN_NUM_MSB > 6)
	/** GCLK generator channel 7. */
	GCLK_GENERATOR_7,
#endif
#if defined(__DOXYGEN__) || (GCLK_GEN_NUM_MSB > 7)
	/** GCLK generator channel 8. */
	GCLK_GENERATOR_8,
#endif
#if defined(__DOXYGEN__) || (GCLK_GEN_NUM_MSB > 8)
	/** GCLK generator channel 9. */
	GCLK_GENERATOR_9,
#endif
#if defined(__DOXYGEN__) || (GCLK_GEN_NUM_MSB > 9)
	/** GCLK generator channel 10. */
	GCLK_GENERATOR_10,
#endif
#if defined(__DOXYGEN__) || (GCLK_GEN_NUM_MSB > 10)
	/** GCLK generator channel 11. */
	GCLK_GENERATOR_11,
#endif
#if defined(__DOXYGEN__) || (GCLK_GEN_NUM_MSB > 11)
	/** GCLK generator channel 12. */
	GCLK_GENERATOR_12,
#endif
#if defined(__DOXYGEN__) || (GCLK_GEN_NUM_MSB > 12)
	/** GCLK generator channel 13. */
	GCLK_GENERATOR_13,
#endif
#if defined(__DOXYGEN__) || (GCLK_GEN_NUM_MSB > 13)
	/** GCLK generator channel 14. */
	GCLK_GENERATOR_14,
#endif
#if defined(__DOXYGEN__) || (GCLK_GEN_NUM_MSB > 14)
	/** GCLK generator channel 15. */
	GCLK_GENERATOR_15,
#endif
#if defined(__DOXYGEN__) || (GCLK_GEN_NUM_MSB > 15)
	/** GCLK generator channel 16. */
	GCLK_GENERATOR_16,
#endif
};

/**
 * \brief Available start-up times for the XOSC32K
 *
 * Available external 32KHz oscillator start-up times, as a number of external
 * clock cycles.
 */
enum system_xosc32k_startup {
	/** Wait 0 clock cycles until the clock source is considered stable */
	SYSTEM_XOSC32K_STARTUP_0,
	/** Wait 32 clock cycles until the clock source is considered stable */
	SYSTEM_XOSC32K_STARTUP_32,
	/** Wait 2048 clock cycles until the clock source is considered stable */
	SYSTEM_XOSC32K_STARTUP_2048,
	/** Wait 4096 clock cycles until the clock source is considered stable */
	SYSTEM_XOSC32K_STARTUP_4096,
	/** Wait 16384 clock cycles until the clock source is considered stable */
	SYSTEM_XOSC32K_STARTUP_16384,
	/** Wait 32768 clock cycles until the clock source is considered stable */
	SYSTEM_XOSC32K_STARTUP_32768,
	/** Wait 65536 clock cycles until the clock source is considered stable */
	SYSTEM_XOSC32K_STARTUP_65536,
	/** Wait 131072 clock cycles until the clock source is considered stable */
	SYSTEM_XOSC32K_STARTUP_131072,
};

/**
 * \brief Division prescalers for the internal 8MHz system clock
 *
 * Available prescalers for the internal 8MHz (nominal) system clock.
 */
enum system_osc8m_div {
	/** Do not divide the 8MHz RC oscillator output */
	SYSTEM_OSC8M_DIV_1,
	/** Divide the 8MHz RC oscillator output by 2 */
	SYSTEM_OSC8M_DIV_2,
	/** Divide the 8MHz RC oscillator output by 4 */
	SYSTEM_OSC8M_DIV_4,
	/** Divide the 8MHz RC oscillator output by 8 */
	SYSTEM_OSC8M_DIV_8,
};

/**
 * \brief Main CPU and APB/AHB bus clock source prescaler values
 *
 * Available division ratios for the CPU and APB/AHB bus clocks.
 */
enum system_main_clock_div {
	/** Divide Main clock by 1 */
	SYSTEM_MAIN_CLOCK_DIV_1,
	/** Divide Main clock by 2 */
	SYSTEM_MAIN_CLOCK_DIV_2,
	/** Divide Main clock by 4 */
	SYSTEM_MAIN_CLOCK_DIV_4,
	/** Divide Main clock by 8 */
	SYSTEM_MAIN_CLOCK_DIV_8,
	/** Divide Main clock by 16 */
	SYSTEM_MAIN_CLOCK_DIV_16,
	/** Divide Main clock by 32 */
	SYSTEM_MAIN_CLOCK_DIV_32,
	/** Divide Main clock by 64 */
	SYSTEM_MAIN_CLOCK_DIV_64,
	/** Divide Main clock by 128 */
	SYSTEM_MAIN_CLOCK_DIV_128,
};

/**
 * \brief Operating modes of the DFLL clock source.
 *
 * Available operating modes of the DFLL clock source module,
 */
enum system_clock_dfll_loop_mode {
	/** The DFLL is operating in open loop mode with no feedback */
	SYSTEM_CLOCK_DFLL_LOOP_MODE_OPEN,
	/** The DFLL is operating in closed loop mode with frequency feedback from
	 *  a low frequency reference clock
	 */
	SYSTEM_CLOCK_DFLL_LOOP_MODE_CLOSED,
	/** The DFLL is operating in USB recovery mode with frequency feedback
	 *  from USB SOF
	 */
	SYSTEM_CLOCK_DFLL_LOOP_MODE_USB_RECOVERY = (1 << 5)/*SYSCTRL_DFLLCTRL_USBCRM*/,
};


/**
 * \brief Device sleep modes.
 *
 * List of available sleep modes in the device. A table of clocks available in
 * different sleep modes can be found in \ref asfdoc_sam0_system_module_overview_sleep_mode.
 */
enum system_sleepmode {
	/** IDLE 0 sleep mode. */
	SYSTEM_SLEEPMODE_IDLE_0,
	/** IDLE 1 sleep mode. */
	SYSTEM_SLEEPMODE_IDLE_1,
	/** IDLE 2 sleep mode. */
	SYSTEM_SLEEPMODE_IDLE_2,
	/** Standby sleep mode. */
	SYSTEM_SLEEPMODE_STANDBY,
};

/**
 * \brief Determines if the hardware module(s) are currently synchronizing to the bus.
 *
 * Checks to see if the underlying hardware peripheral module(s) are currently
 * synchronizing across multiple clock domains to the hardware bus, This
 * function can be used to delay further operations on a module until such time
 * that it is ready, to prevent blocking delays for synchronization in the
 * user application.
 *
 * \return Synchronization status of the underlying hardware module(s).
 *
 * \retval true  If the module has completed synchronization
 * \retval false If the module synchronization is ongoing
 */
static inline bool extint_is_syncing(void)
{
	Eic *const eics[EIC_INST_NUM] = EIC_INSTS;

	for (uint32_t i = 0; i < EIC_INST_NUM; i++) {
		if (eics[i]->STATUS.reg & EIC_STATUS_SYNCBUSY) {
			return true;
		}
	}

	return false;
}

/**
 * \brief Set the sleep mode of the device
 *
 * Sets the sleep mode of the device; the configured sleep mode will be entered
 * upon the next call of the \ref system_sleep() function.
 *
 * For an overview of which systems are disabled in sleep for the different
 * sleep modes, see \ref asfdoc_sam0_system_module_overview_sleep_mode.
 *
 * \param[in] sleep_mode  Sleep mode to configure for the next sleep operation
 *
 * \retval STATUS_OK               Operation completed successfully
 * \retval STATUS_ERR_INVALID_ARG  The requested sleep mode was invalid or not
 *                                 available
 */
static inline void system_set_sleepmode(const enum system_sleepmode sleep_mode)
{
	switch (sleep_mode) 
	{
		case SYSTEM_SLEEPMODE_IDLE_0:
		case SYSTEM_SLEEPMODE_IDLE_1:
		case SYSTEM_SLEEPMODE_IDLE_2:
			SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
			PM->SLEEP.reg = sleep_mode;
			break;

		case SYSTEM_SLEEPMODE_STANDBY:
			SCB->SCR |=  SCB_SCR_SLEEPDEEP_Msk;
			break;

		default:
		break;
	}
}

/**
 * \brief Put the system to sleep waiting for interrupt
 *
 * Executes a device DSB (Data Synchronization Barrier) instruction to ensure
 * all ongoing memory accesses have completed, then a WFI (Wait For Interrupt)
 * instruction to place the device into the sleep mode specified by
 * \ref system_set_sleepmode until woken by an interrupt.
 */
static inline void system_sleep(void)
{
	__DSB();
	__WFI();
}

void sys_clock_init(void);

#endif // sys_ctrl_h__