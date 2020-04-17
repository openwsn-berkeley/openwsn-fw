/**
 * Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Modified: Tengfei Chang (tengfei.chang@eecs.berkeley.edu)
 * Date:   July 2013
 * Description: CC2538-specific board information bsp module.
 */

#ifndef __BOARD_INFO_H
#define __BOARD_INFO_H

#include <stdint.h>
#include <string.h>

#include <source/cpu.h>
#include <source/interrupt.h>

//=========================== defines =========================================

//===== interrupt state

#define INTERRUPT_DECLARATION()
#define DISABLE_INTERRUPTS() IntMasterDisable()

#define ENABLE_INTERRUPTS() IntMasterEnable()

//===== timer

#define PORT_TIMER_WIDTH                    uint32_t
#define PORT_RADIOTIMER_WIDTH               uint32_t

#define PORT_SIGNED_INT_WIDTH               int32_t
#define PORT_TICS_PER_MS                    33
#define PORT_US_PER_TICK                    30 // number of us per 32kHz clock tick
// on GINA, we use the comparatorA interrupt for the OS
#define SCHEDULER_WAKEUP()
#define SCHEDULER_ENABLE_INTERRUPT()

// this is a workaround from the fact that the interrupt pin for the GINA radio
// is not connected to a pin on the MSP which allows time capture.
#define CAPTURE_TIME()

/* sleep timer interrupt */
#define HAL_INT_PRIOR_ST        (4 << 5)
/* MAC interrupts */
#define HAL_INT_PRIOR_MAC       (4 << 5)
/* UART interrupt */
#define HAL_INT_PRIOR_UART      (5 << 5)

//===== pinout

// [P4.7] radio SLP_TR_CNTL
#define PORT_PIN_RADIO_SLP_TR_CNTL_HIGH()
#define PORT_PIN_RADIO_SLP_TR_CNTL_LOW()
// radio reset line
// on cc2538, the /RST line is not connected to the uC
#define PORT_PIN_RADIO_RESET_HIGH()    // nothing
#define PORT_PIN_RADIO_RESET_LOW()     // nothing

//===== Slot Information
/*
    Indexes for slot templates:
    
    10 : 10 ms tempalte
    20 : 20 ms template for cc2538 24ghz radio --> Default
    41 : 40 ms template for cc2538 24ghz radio
    42 : 40 ms template for Atmel FSK FEC radio
    43 : 40 ms template for Atmel OFSM1 radios MCS 0,1,2, and 3
*/
#define SLOT_TEMPLATE 20                

//===== Radios

// Number of available radios
#define MAX_RADIOS  7

//===== IEEE802154E timing
    
typedef struct {
    uint8_t SLOTDURATION;
    uint8_t PORT_TsSlotDuration;

    // execution speed related
    // also implementation related (e.g. SoC radio or spi-connected radio because of the time to load a packet to/from radio)
    uint8_t PORT_maxTxDataPrepare;
    uint8_t PORT_maxRxAckPrepare;
    uint8_t PORT_maxRxDataPrepare;
    uint8_t PORT_maxTxAckPrepare;

    // radio speed related
    // also implementation related (because the time to execute the Tx/Rx command is highly dependent on the radio AND the configuration)
    uint8_t PORT_delayTx;                       
    uint8_t PORT_delayRx;
} slot_board_vars_t; //board specific slot vars

// available slot templates
typedef enum{
    SLOT_10ms,
    SLOT_20ms_24GHZ,
    SLOT_40ms_24GHZ,
    SLOT_40ms_FSK_SUBGHZ,
    SLOT_40ms_OFDM1MCS0_3_SUBGHZ,
    MAX_SLOT_TYPES,
} slotType_t;

//===== adaptive_sync accuracy

#define SYNC_ACCURACY                       1     // ticks

//===== per-board number of sensors

#define NUMSENSORS      7

//====== Antenna options ====
#define BSP_ANTENNA_BASE            GPIO_D_BASE
#define BSP_ANTENNA_CC2538_24GHZ    GPIO_PIN_4      //!< PD4 -- 2.4ghz
#define BSP_ANTENNA_AT215_24GHZ     GPIO_PIN_3      //!< PD3 -- subghz
//#define DAGROOT

//=========================== typedef  ========================================

//=========================== variables =======================================

static const uint8_t rreg_uriquery[]        = "h=ucb";
static const uint8_t infoBoardname[]        = "openmote-b";
static const uint8_t infouCName[]           = "CC2538";
static const uint8_t infoRadioName[]        = "CC2538 SoC";

//=========================== prototypes ======================================

//=========================== public ==========================================
void eraseFlash(void);
//=========================== private =========================================

#endif
