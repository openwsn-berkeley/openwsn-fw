/**
 * Author: Tamas Harczos (tamas.harczos@imms.de)
 * Date:   Apr 2018
 * Description: nRF52840-specific board information bsp module.
 */

#ifndef __BOARD_INFO_H
#define __BOARD_INFO_H

#include <stdint.h>
#include <string.h>

#include "sdk/modules/nrfx/templates/nRF52840/nrfx_config.h"
#include "opendefs.h"


//=========================== defines =========================================

//===== interrupt state

#define INTERRUPT_DECLARATION()
#define DISABLE_INTERRUPTS() __asm volatile ("cpsid i" : : : "memory")    ///< or use __disable_irq() from core_cmFunc.h
#define ENABLE_INTERRUPTS() __asm volatile ("cpsie i" : : : "memory")     ///< or use __enable_irq() from core_cmFunc.h

//===== timer

#define PORT_TIMER_WIDTH                    uint32_t      ///< actually, 24 bit, but we will virtualize the 32 bit timer
#define PORT_RADIOTIMER_WIDTH               uint32_t

#define PORT_SIGNED_INT_WIDTH               int32_t
#define PORT_TICS_PER_MS                    33
#define PORT_US_PER_TICK                    30  // number of us per 32kHz clock tick

// on GINA, we use the comparatorA interrupt for the OS
#define SCHEDULER_WAKEUP()
#define SCHEDULER_ENABLE_INTERRUPT()

// this is a workaround from the fact that the interrupt pin for the GINA radio
// is not connected to a pin on the MSP which allows time capture.
#define CAPTURE_TIME()

//===== adaptive_sync accuracy

#define SYNC_ACCURACY                            1     // ticks

//===== per-board number of sensors

#define NUMSENSORS 2

//=========================== typedef  ========================================

//=========================== variables =======================================

static const uint8_t rreg_uriquery[]        = "h=ucb";
static const uint8_t infoBoardname[]        = "nRF52840";
static const uint8_t infouCName[]           = "nRF52840";
static const uint8_t infoRadioName[]        = "nRF52840 SoC";

//=========================== prototypes ======================================

//=========================== public ==========================================

//=========================== private =========================================

#endif
