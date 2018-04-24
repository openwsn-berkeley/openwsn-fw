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

#ifndef __BOARD_INFO_H
#define __BOARD_INFO_H

#include "stdint.h"
#include "string.h"
#include "cm0plus_interrupt.h"
#include "samr21_radio.h"


//=========================== defines =========================================
typedef uint32_t irqflags_t;

#define port_INLINE                         inline

#define PRAGMA(x)  _Pragma(#x)
#define PACK(x)     pack(x)

#define INTERRUPT_DECLARATION() irqflags_t irq_flags;//no declaration

#define DISABLE_INTERRUPTS()    irq_flags = cpu_irq_save();
#define ENABLE_INTERRUPTS()     cpu_irq_restore(irq_flags);

//===== timer

#define PORT_TIMER_WIDTH                    uint16_t
#define PORT_RADIOTIMER_WIDTH               uint16_t

#define PORT_SIGNED_INT_WIDTH               int16_t
#define PORT_TICS_PER_MS                    33
#define SCHEDULER_WAKEUP()                  debugpins_isr_set();\
										    debugpins_isr_clr();      
#define SCHEDULER_ENABLE_INTERRUPT()        

//===== pinout

#define PORT_PIN_RADIO_SLP_TR_CNTL_HIGH()      SLP_TR_HIGH()
#define PORT_PIN_RADIO_SLP_TR_CNTL_LOW()       SLP_TR_LOW()

#define PORT_PIN_RADIO_RESET_HIGH()            //RST_HIGH()   
#define PORT_PIN_RADIO_RESET_LOW()             //RST_LOW()

#define SLOTDURATION 15 // in miliseconds

//===== IEEE802154E timing
// time-slot related
#define PORT_TsSlotDuration                 491
#define PORT_maxTxDataPrepare               66//33//66
#define PORT_maxRxAckPrepare                20//10 
#define PORT_maxRxDataPrepare               33//33
#define PORT_maxTxAckPrepare                30//22 
#define PORT_delayTx                        20//15
#define PORT_delayRx                        0

#define SYNC_ACCURACY                       1     // ticks

//=========================== typedef  ========================================

//=========================== variables =======================================

static const uint8_t rreg_uriquery[]        = "h=ucb";
static const uint8_t infoBoardname[]        = "SAMR21 Xplained Pro";
static const uint8_t infouCName[]           = "SAMR21G18A";
static const uint8_t infoRadioName[]        = "AT86RF233";

//=========================== prototypes ======================================

//=========================== public ==========================================

//=========================== private =========================================

#endif
