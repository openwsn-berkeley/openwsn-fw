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
#include "samr21_timer.h"
#include "samr21_sysctrl.h"
#include "samr21_gclk.h"
#include "cm0plus_interrupt.h"
#include "samr21_gpio.h"


void tc4_ovf_callback(void) __attribute__ ((weak, alias("dummy_irq")));
void tc4_cca1_callback(void) __attribute__ ((weak, alias("dummy_irq")));
void tc3_cca0_callback(void) __attribute__ ((weak, alias("dummy_irq")));

/** \brief timer3_init This will initialize the timer3 with default value

     

    \param [in] None
    \return None
 */
void timer3_init(void)
{
 Tc *const hw = TC3;
 
 /* Check for the Reset */	
 while(hw->COUNT8.CTRLA.reg & TC_CTRLA_SWRST){
 }
 
 /* Turn on module in PM */
 PM->APBCMASK.reg |= PM_APBCMASK_TC3;
 
 /* Set up the GCLK for the module */
 gclk_chan_config(TC3_GCLK_ID, GCLK_GENERATOR_2);
 gclk_enable(TC3_GCLK_ID);
 
 /* Wait for Sync */
 while(hw->COUNT8.STATUS.reg & TC_STATUS_SYNCBUSY);
 
 hw->COUNT8.CTRLA.reg = TC_CTRLA_MODE_COUNT16 | TC_CTRLA_WAVEGEN_NFRQ | \
						TC_CTRLA_PRESCSYNC_GCLK | TC_CTRLA_PRESCALER_DIV1\
						/*TC_CTRLA_RUNSTDBY*/;
						
 /* wait for Sync */
 while(hw->COUNT8.STATUS.reg & TC_STATUS_SYNCBUSY);
 /* Reset All bits in CTRLB */
 hw->COUNT8.CTRLBCLR.reg = 0xFF;
 
  /* wait for Sync */
  while(hw->COUNT8.STATUS.reg & TC_STATUS_SYNCBUSY);
  
  hw->COUNT8.CTRLBSET.reg = TC_COUNT_DIRECTION_UP;
  
  /* wait for Sync */
  while(hw->COUNT8.STATUS.reg & TC_STATUS_SYNCBUSY);
   
  /* No Capture & No Waveform Invert Enable */
  hw->COUNT8.CTRLC.reg = 0x00;
  
  /* wait for Sync */
  while(hw->COUNT8.STATUS.reg & TC_STATUS_SYNCBUSY);
  hw->COUNT16.COUNT.reg = 0x0000;
  
  /* wait for Sync */
  while(hw->COUNT8.STATUS.reg & TC_STATUS_SYNCBUSY);
  hw->COUNT16.CC[0].reg = UINT16_MAX;
  
  /* wait for Sync */
  while(hw->COUNT8.STATUS.reg & TC_STATUS_SYNCBUSY);
  hw->COUNT16.CC[1].reg = 0;
  
  /* wait for Sync */
  while(hw->COUNT8.STATUS.reg & TC_STATUS_SYNCBUSY);
  hw->COUNT16.READREQ.reg |= (TC_READREQ_RCONT | TC_READREQ_ADDR(0x10));
  
  /* wait for Sync */
  while(hw->COUNT8.STATUS.reg & TC_STATUS_SYNCBUSY);
  /* Enable TC module */
  hw->COUNT8.CTRLA.reg |= TC_CTRLA_ENABLE;  
  
  /* Clear all the flag */
  TC3->COUNT8.INTFLAG.reg = TC_INTFLAG_ERR | TC_INTFLAG_SYNCRDY |\
                            TC_INTFLAG_MC0 | TC_INTFLAG_MC1 |\
							TC_INTFLAG_OVF;
}

/** \brief timer4_init This will initialize the timer0 with default value

     

    \param [in] None
    \return None
 */
void timer4_init(void)
{
 Tc *const hw = TC4;
 
 /* Check for the Reset */	
 while(hw->COUNT8.CTRLA.reg & TC_CTRLA_SWRST){
 }
 
 /* Turn on module in PM */
 PM->APBCMASK.reg |= PM_APBCMASK_TC4;
 
 /* Set up the GCLK for the module */
 gclk_chan_config(TC4_GCLK_ID, GCLK_GENERATOR_2);
 gclk_enable(TC4_GCLK_ID);
 
 /* Wait for Sync */
 while(hw->COUNT8.STATUS.reg & TC_STATUS_SYNCBUSY);
 
 hw->COUNT8.CTRLA.reg = TC_CTRLA_MODE_COUNT16 | TC_CTRLA_WAVEGEN_MFRQ | \
						TC_CTRLA_PRESCSYNC_GCLK | TC_CTRLA_PRESCALER(0) |\
						TC_CTRLA_RUNSTDBY;
						
 /* wait for Sync */
 while(hw->COUNT8.STATUS.reg & TC_STATUS_SYNCBUSY);
 /* Reset All bits in CTRLB */
 hw->COUNT8.CTRLBCLR.reg = 0xFF;
 
  /* wait for Sync */
  while(hw->COUNT8.STATUS.reg & TC_STATUS_SYNCBUSY);
  
  hw->COUNT8.CTRLBSET.reg = TC_COUNT_DIRECTION_UP;
  
  /* wait for Sync */
  while(hw->COUNT8.STATUS.reg & TC_STATUS_SYNCBUSY);
   
  /* No Capture & No Waveform Invert Enable */
  hw->COUNT8.CTRLC.reg = 0x00;
  
  /* wait for Sync */
  while(hw->COUNT8.STATUS.reg & TC_STATUS_SYNCBUSY);
  hw->COUNT16.COUNT.reg = 0x0000;
  
  /* wait for Sync */
  while(hw->COUNT8.STATUS.reg & TC_STATUS_SYNCBUSY);
  hw->COUNT16.CC[0].reg = UINT16_MAX;
  
  /* wait for Sync */
  while(hw->COUNT8.STATUS.reg & TC_STATUS_SYNCBUSY);
  hw->COUNT16.CC[1].reg = UINT16_MAX;
  
  /* wait for Sync */
  while(hw->COUNT8.STATUS.reg & TC_STATUS_SYNCBUSY);
  hw->COUNT16.READREQ.reg |= (TC_READREQ_RCONT | TC_READREQ_ADDR(0x10));
  
  /* wait for Sync */
  while(hw->COUNT8.STATUS.reg & TC_STATUS_SYNCBUSY);
  /* Enable TC module */
  hw->COUNT8.CTRLA.reg |= TC_CTRLA_ENABLE;
  
  /* Clear all the flag */
  TC4->COUNT8.INTENCLR.reg = TC_INTFLAG_ERR | TC_INTFLAG_SYNCRDY |\
                            TC_INTFLAG_MC0 | TC_INTFLAG_MC1 |\
							TC_INTFLAG_OVF;
}

/** \brief TC3_Handler Handler for Timer-3 Interrupts

     Handles the timer-3 interrupts and flags

    \param [in] None
    \return None
 */
void TC3_Handler(void)
{
 uint8_t isr_flag = TC3->COUNT8.INTFLAG.reg;	
 if(isr_flag & TC_INTFLAG_MC0)
 {	 
	 TC3->COUNT8.INTFLAG.reg = TC_INTFLAG_MC0;
	 tc3_cca0_callback();
 }
 if (isr_flag & TC_INTFLAG_MC1)
 {
	 TC3->COUNT8.INTFLAG.reg = TC_INTFLAG_MC1;
 }
 if (isr_flag & TC_INTFLAG_OVF)
 {
	 TC3->COUNT8.INTFLAG.reg = TC_INTFLAG_OVF;
 }
 if (isr_flag & TC_INTFLAG_ERR)
 {
	 TC3->COUNT8.INTFLAG.reg = TC_INTFLAG_ERR;
 }
}

/** \brief TC4_Handler Handler for Timer-4 Interrupts

     Handles the timer-4 interrupts and flags

    \param [in] None
    \return None
 */
void TC4_Handler(void)
{
 uint8_t isr_flag = TC4->COUNT8.INTFLAG.reg;
 if(isr_flag & TC_INTFLAG_MC0)
 {	 
	 TC4->COUNT8.INTFLAG.reg = TC_INTFLAG_MC0;
 }
 if (isr_flag & TC_INTFLAG_MC1)
 {
	 TC4->COUNT8.INTFLAG.reg = TC_INTFLAG_MC1;
	 tc4_cca1_callback();	 
 }
 if (isr_flag & TC_INTFLAG_OVF)
 {	 
	 TC4->COUNT8.INTFLAG.reg = TC_INTFLAG_OVF;
	 tc4_ovf_callback();
 }
 if (isr_flag & TC_INTFLAG_ERR)
 {
	 TC4->COUNT8.INTFLAG.reg = TC_INTFLAG_ERR;
 }
}