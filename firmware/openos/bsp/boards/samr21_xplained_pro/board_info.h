#ifndef __BOARD_INFO_H
#define __BOARD_INFO_H

#include "stdint.h"
#include "string.h"
#include "port.h"
#include "samr21_xplained_pro.h"
#include <compiler.h>
#include "extint_callback.h"

//=========================== defines =========================================

#define port_INLINE                         inline

#define PRAGMA(x)  _Pragma(#x)
#define PACK(x)     pack(x)

//TODO in case previous declaration fails in certain compilers. Remove this 
//one if it works with GNU GCC
//#define PACK_START  _Pragma("pack(1)")
//#define PACK_END    _Pragma("pack()")
#define INTERRUPT_DECLARATION() irqflags_t irq_flags;//no declaration

#define DISABLE_INTERRUPTS()    irq_flags = cpu_irq_save();
#define ENABLE_INTERRUPTS()     cpu_irq_restore(irq_flags);

//===== timer

#define PORT_TIMER_WIDTH                    uint16_t
#define PORT_RADIOTIMER_WIDTH               uint16_t

#define PORT_SIGNED_INT_WIDTH               int32_t
#define PORT_TICS_PER_MS                    33
#define SCHEDULER_WAKEUP()                  debugpins_isr_set();\
										    debugpins_isr_clr();      
#define SCHEDULER_ENABLE_INTERRUPT()        

//===== pinout

#define PORT_PIN_RADIO_SLP_TR_CNTL_HIGH()      SLP_TR_HIGH()
#define PORT_PIN_RADIO_SLP_TR_CNTL_LOW()       SLP_TR_LOW()

#define PORT_PIN_RADIO_RESET_HIGH()            //RST_HIGH()   
#define PORT_PIN_RADIO_RESET_LOW()             //RST_LOW()

//===== IEEE802154E timing
// time-slot related
#define PORT_TsSlotDuration                 492
#define PORT_maxTxDataPrepare               66
#define PORT_maxRxAckPrepare                10 
#define PORT_maxRxDataPrepare               33
#define PORT_maxTxAckPrepare                22 
#define PORT_delayTx                        15
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
