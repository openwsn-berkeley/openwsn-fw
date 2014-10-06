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

#include <sam.h>
#include "samr21_sysctrl.h"
#include "base_type.h"
#include "delay.h"

static void xosc32k_init(void);
static void gclk_init(void);
static void dfll_init(void);
static void osc8m_init(void);
static void gclk_config_main_clk(void);
static void event_system_init(void);
static void extint_system_init(void);

/** \brief sys_clock_init Function to initialize the system clock

     This function will initialize the clock domain and sub-modules

    \param [in] None
    \return None
 */
void sys_clock_init(void)
{
 /* Clear the Interrupt Flag bits in SYSCTRL */
 SYSCTRL->INTFLAG.reg = SYSCTRL_INTFLAG_BOD33RDY | SYSCTRL_INTFLAG_BOD33DET |\
						SYSCTRL_INTFLAG_DFLLRDY;
						
 /* set flash wait states for internal flash memory. 
    Flash wait states are based on the CPU Freq and Voltage */
 NVMCTRL->CTRLB.bit.RWS = INTERNAL_FLASH_WAIT_STATE;
 
 /* Initialize the XOSC32K Clock */
 xosc32k_init();
 
 /* Initialize the Internal 8MHz RC OSC */
 osc8m_init();
 
 /* Initialize the GCLK */
 gclk_init();
 
 /* DFLL Initialize */
 dfll_init();
 
 /* Sets the clock divider used on the main clock to provide the CPU clock */
 PM->CPUSEL.reg = (uint32_t)SYSTEM_MAIN_CLOCK_DIV_1;
 
 /* Set APBx clock divider */
 PM->APBASEL.reg = (uint32_t)SYSTEM_MAIN_CLOCK_DIV_1; 
 PM->APBBSEL.reg = (uint32_t)SYSTEM_MAIN_CLOCK_DIV_1;
 
 /* Configure the main GCLK last as it might depend on other Generators */
 gclk_config_main_clk();
 
 /* Event system Initialize */
 event_system_init();
 
 /* External interrupt module init */
 extint_system_init();
}

/** \brief xosc32k_init This function will initialize the External 32.768KHz Crystal

     This function will set all the required parameter into XOSC32K register

    \param [in] None
    \return None
 */
static void xosc32k_init(void)
{
 /* Initialize the XOSC32K External oscillator */
 SYSCTRL_XOSC32K_Type xosc32_clk = SYSCTRL->XOSC32K;
 
 /* Start up time for XOSC32K */
 xosc32_clk.bit.STARTUP = SYSTEM_XOSC32K_STARTUP_65536;
 /* External XOSC32K Enable */
 xosc32_clk.bit.XTALEN = 1;
 
 /* Auto amplitude control */
 xosc32_clk.bit.AAMPEN = true;
 /* 1KHz output Enable/Disable */
 xosc32_clk.bit.EN1K = false;
 /* 32KHz output Enable/Disable */
 xosc32_clk.bit.EN32K = true;

 /* On demand feature of the clock module */
 xosc32_clk.bit.ONDEMAND = false;
 /* Run in standby mode feature of the clock */
 xosc32_clk.bit.RUNSTDBY = true;
 /* Write once feature of the clock */
 xosc32_clk.bit.WRTLOCK  = false;
 
 SYSCTRL->XOSC32K = xosc32_clk;
 
 /* Enable the XOSC32K crystal */
 SYSCTRL->XOSC32K.reg |= SYSCTRL_XOSC32K_ENABLE;
 
 /* Check the XOSC32K Clock source is ready to use */
 while(!((SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_XOSC32KRDY) == \
								SYSCTRL_PCLKSR_XOSC32KRDY));
								
 /* Set the On demand bit here if needed */
 
}

/** \brief dfll_init Initialize and use the DFLL

     Initialize and use the digital frequency locked loop with 
	 selected clock source and required output

    \param [in] None
    \return None
 */
static void dfll_init(void)
{
 uint32_t dffl_ctrl = ((SYSCTRL_DFLLCTRL_ENABLE | SYSCTRL_DFLLCTRL_MODE | SYSCTRL_DFLLCTRL_WAITLOCK)) &\
					  ((~SYSCTRL_DFLLCTRL_ONDEMAND) & (~SYSCTRL_DFLLCTRL_STABLE) &\
					  (~SYSCTRL_DFLLCTRL_CCDIS) & (~SYSCTRL_DFLLCTRL_QLDIS) &\
					  (~ SYSCTRL_DFLLCTRL_RUNSTDBY) & (~SYSCTRL_DFLLCTRL_BPLCKC) &\
					  (~SYSCTRL_DFLLCTRL_LLAW));
					  
 /* Select the requested generator channel - 0 */
 *((uint8_t*)&GCLK->CLKCTRL.reg) = SYSCTRL_GCLK_ID_DFLL48;

 /* Switch to known-working source so that the channel can be disabled */
 uint32_t prev_gen_id = GCLK->CLKCTRL.bit.GEN;
 GCLK->CLKCTRL.bit.GEN = 0;

 /* Disable the generic clock */
 GCLK->CLKCTRL.reg &= ~GCLK_CLKCTRL_CLKEN;
 while (GCLK->CLKCTRL.reg & GCLK_CLKCTRL_CLKEN) {
	 /* Wait for clock to become disabled */
 }

 /* Restore previous configured clock generator */
 GCLK->CLKCTRL.bit.GEN = prev_gen_id;
 
 /* Write the new configuration */
 GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(0) | DFLL_INPUT_CLOCK;
 
 /* Select the requested generator channel */
 *((uint8_t*)&GCLK->CLKCTRL.reg) = 0;

 /* Enable the generic clock */
 GCLK->CLKCTRL.reg |= GCLK_CLKCTRL_CLKEN;
 
 /* Enable the System Clock source */
 /* Disable ONDEMAND mode while writing configurations */
 SYSCTRL->DFLLCTRL.reg = dffl_ctrl;
						 
 /* Wait for sync to the DFLL control registers */						 
 while (!(SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY)) {
	 /* Wait for DFLL sync */
 }
 SYSCTRL->DFLLMUL.reg = SYSCTRL_DFLLMUL_CSTEP(0x1f/4) | \
						SYSCTRL_DFLLMUL_FSTEP(0xff/4) | \
						SYSCTRL_DFLLMUL_MUL(DFLL_MUL_VAL);
						
 SYSCTRL->DFLLVAL.reg = SYSCTRL_DFLLVAL_FINE(0xff/4) | SYSCTRL_DFLLVAL_FINE(0x1f/4);
						
 
 /* Write full configuration to DFLL control register */
 SYSCTRL->DFLLCTRL.reg = dffl_ctrl;
 /* Check the clock is ready */
 while(!((SYSCTRL->PCLKSR.reg & (SYSCTRL_PCLKSR_DFLLRDY | SYSCTRL_PCLKSR_DFLLLCKF |\
       SYSCTRL_PCLKSR_DFLLLCKC)) == (SYSCTRL_PCLKSR_DFLLRDY | SYSCTRL_PCLKSR_DFLLLCKF |\
       SYSCTRL_PCLKSR_DFLLLCKC)));
	   
 /* If On Demand is required set it here */
    
}

/** \brief osc8m_init This function will initialize the internal 8MHz oscillator

     Initialize and enable the 8MHz internal RC oscillator, 
	 This clock may be used for backup operation. Incase of any failures

    \param [in] None
    \return None
 */
static void osc8m_init(void)
{
 SYSCTRL_OSC8M_Type osc8m_clk = SYSCTRL->OSC8M;
 /* Set the pre-scaler output for OSC8M */
 osc8m_clk.bit.PRESC = SYSTEM_OSC8M_DIV_1;
 /* Set the On demand state for OSC8M */
 osc8m_clk.bit.ONDEMAND = false;
 /* Set the On run in standby state for OSC8M */
 osc8m_clk.bit.RUNSTDBY = false;
 /* Write into register */
 SYSCTRL->OSC8M = osc8m_clk;
 
 /* Enable the OSC8M Clock source */
 SYSCTRL->OSC8M.reg |= SYSCTRL_OSC8M_ENABLE;
}

/** \brief gclk_init Initialize the Generic clock for the peripheral

     This function will initialize and enable the Generic clock

    \param [in] None
    \return None
 */
static void gclk_init(void)
{
 /* Enable the APB_APBA clock - where the GCLK uses this clock */
 PM->APBAMASK.reg |= PM_APBAMASK_GCLK;
 
 /* Software reset the module to ensure it is re-initialized correctly */
 GCLK->CTRL.reg = GCLK_CTRL_SWRST;
 
 while(GCLK->CTRL.reg & GCLK_CTRL_SWRST){
	 /* wait for reset to complete */
 }
 /* Configure all GCLK generator except the main generator */ 
 
 /* Configure the GCLK-2 */
 /* Wait for synchronization */
 while((GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY)); 
 
 *((uint8_t*)&GCLK->GENDIV.reg) = GCLK_GENDIV_ID(2);
 
 /* Wait for synchronization */
 while((GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY));
 
 /* Write the Divisor value to the register */
 GCLK->GENDIV.reg  = GCLK_GENDIV_DIV(0);
 
 /* Wait for synchronization */
 while((GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY));
 
 GCLK->GENCTRL.reg = (GCLK->GENCTRL.reg & GCLK_GENCTRL_GENEN) | \
					  GCLK_CLKCTRL_ID(2) | GCLK_GENCTRL_RUNSTDBY | \
					  GCLK_GENCTRL_SRC_XOSC32K;
					  
 /* Wait for synchronization */
 while((GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY));	 
 
 *((uint8_t*)&GCLK->GENCTRL.reg) = GCLK_CLKCTRL_ID(2);
 
 /* Wait for synchronization */
 while((GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY));
  
 /* Enable generator */
 GCLK->GENCTRL.reg |= GCLK_GENCTRL_GENEN;				  
					  

 /* Configure the GCLK-1 */					  
 /* Wait for synchronization */
 while((GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY));
 
 *((uint8_t*)&GCLK->GENDIV.reg) = GCLK_GENDIV_ID(1);
 
 /* Wait for synchronization */
 while((GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY));
 
 /* Write the Divisor value to the register */
 GCLK->GENDIV.reg  = GCLK_GENDIV_DIV(0);
 
 /* Wait for synchronization */
 while((GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY));
 
 GCLK->GENCTRL.reg = (GCLK->GENCTRL.reg & GCLK_GENCTRL_GENEN) | \
 GCLK_CLKCTRL_ID(1) | GCLK_GENCTRL_SRC_DFLL48M; 
 
 /* Wait for synchronization */
 while((GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY));
 
 *((uint8_t*)&GCLK->GENCTRL.reg) = GCLK_CLKCTRL_ID(1);
 
 /* Wait for synchronization */
 while((GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY));
 
 /* Enable generator */
 GCLK->GENCTRL.reg |= GCLK_GENCTRL_GENEN;
}

/** \brief gclk_config_main_clk This function configures the main clock

     The main clock configuration happens only after the other clock initialization

    \param [in] None
    \return None
 */
static void gclk_config_main_clk(void)
{
 /* Configure the GCLK-1 */
 /* Wait for synchronization */
 while((GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY));
 
 *((uint8_t*)&GCLK->GENDIV.reg) = GCLK_GENDIV_ID(0);
 
 /* Wait for synchronization */
 while((GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY));
 
 /* Write the Divisor value to the register */
 GCLK->GENDIV.reg  = GCLK_GENDIV_DIV(0);
 
 /* Wait for synchronization */
 while((GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY));
 
 GCLK->GENCTRL.reg = (GCLK->GENCTRL.reg & GCLK_GENCTRL_GENEN) | \
					 GCLK_CLKCTRL_ID(0) | GCLK_GENCTRL_SRC_DFLL48M;
 
 /* Wait for synchronization */
 while((GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY));
 
 *((uint8_t*)&GCLK->GENCTRL.reg) = GCLK_CLKCTRL_ID(0);
 
 /* Wait for synchronization */
 while((GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY));
 
 /* Enable generator */
 GCLK->GENCTRL.reg |= GCLK_GENCTRL_GENEN;
}

/** \brief event_system_init Initialize the event system module

     Enable the Event system register interface

    \param [in] None
    \return None
 */
static void event_system_init(void)
{
  /* Enable EVSYS register interface */
  PM->APBCMASK.reg |= PM_APBCMASK_EVSYS;
  
  /* Make sure the EVSYS module is properly reset */
  EVSYS->CTRL.reg = EVSYS_CTRL_SWRST;
  
  while (EVSYS->CTRL.reg & EVSYS_CTRL_SWRST) {
  }
}

/** \brief extint_system_init Initializes and enables the External Interrupt driver.

     Enable the clocks used by External Interrupt driver. 
	 Resets the External Interrupt driver, resetting all hardware. 
	 module registers to their power-on defaults, then enable it for further use. 
	 Reset the callback list if callback mode is used. This function must be called before 
	 attempting to use any NMI or standard external interrupt channel functions.

    \param [in] None
    \return None
 */
static void extint_system_init(void)
{
	Eic *const eics[EIC_INST_NUM] = EIC_INSTS;

	/* Turn on the digital interface clock */
	PM->APBAMASK.reg |= PM_APBAMASK_EIC;

	/* Configure the generic clock for the module and enable it */
	
	/* Select the requested generator channel */
	*((uint8_t*)&GCLK->CLKCTRL.reg) = EIC_GCLK_ID;
	
	/* Switch to known-working source so that the channel can be disabled */
	uint32_t prev_gen_id = GCLK->CLKCTRL.bit.GEN;
	GCLK->CLKCTRL.bit.GEN = 0;

	/* Disable the generic clock */
	GCLK->CLKCTRL.reg &= ~GCLK_CLKCTRL_CLKEN;
	while (GCLK->CLKCTRL.reg & GCLK_CLKCTRL_CLKEN) {
		/* Wait for clock to become disabled */
	}

	/* Restore previous configured clock generator */
	GCLK->CLKCTRL.bit.GEN = prev_gen_id;
	
	/* Write the new configuration */
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(EIC_GCLK_ID) | GCLK_CLKCTRL_GEN(GCLK_GENERATOR_0);
	
	
	/* Enables a Generic Clock that was previously configured */
	/* Select the requested generator channel */
	*((uint8_t*)&GCLK->CLKCTRL.reg) = EIC_GCLK_ID;

	/* Enable the clock anyway, since when needed it will be requested
	 * by External Interrupt driver */
	GCLK->CLKCTRL.reg |= GCLK_CLKCTRL_CLKEN;

	/* Reset all EIC hardware modules. */
	for (uint32_t i = 0; i < EIC_INST_NUM; i++) {
		eics[i]->CTRL.reg |= EIC_CTRL_SWRST;
	}

	while (extint_is_syncing()) {
		/* Wait for all hardware modules to complete synchronization */
	}

	/* Enable the External Interrupt */	
	NVIC->ISER[0] = (uint32_t)(1 << ((uint32_t)EIC_IRQn & 0x0000001f));


	/* Enables the driver for further use 
	   Enable all EIC hardware modules. */
	for (uint32_t i = 0; i < EIC_INST_NUM; i++) {
		eics[i]->CTRL.reg |= EIC_CTRL_ENABLE;
	}

	while (extint_is_syncing()) {
		/* Wait for all hardware modules to complete synchronization */
	} 
}

/* Delay Function in SAM0 */
RAMFUNC
void portable_delay_cycles(unsigned long n)
{
	UNUSED(n);

	__asm (
	"loop: DMB	\n"
	#ifdef __ICCARM__
	"SUBS r0, r0, #1 \n"
	#else
	"SUB r0, r0, #1 \n"
	#endif
	"CMP r0, #0  \n"
	"BNE loop         "
	);
}
