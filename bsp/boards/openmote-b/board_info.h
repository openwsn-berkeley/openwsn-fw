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

//===== IEEE802154E timing

// #define SLOTDURATION_10MS // by default, we use 10ms time slot

#ifdef SLOTDURATION_10MS
    // time-slot related
    #define PORT_TsSlotDuration                 328   // counter counts one extra count, see datasheet
    // execution speed related
    #define PORT_maxTxDataPrepare               10    //  305us (measured  82us)
    #define PORT_maxRxAckPrepare                10    //  305us (measured  83us)
    #define PORT_maxRxDataPrepare                4    //  122us (measured  22us)
    #define PORT_maxTxAckPrepare                10    //  122us (measured  94us)
    // radio speed related
    #ifdef L2_SECURITY_ACTIVE
    #define PORT_delayTx                        14    //  366us (measured xxxus)
    #else
    #define PORT_delayTx                        12    //  366us (measured xxxus)
    #endif
    #define PORT_delayRx                         0    //    0us (can not measure)
    // radio watchdog
#else // 40 ms
    // time-slot related
    #define PORT_TsSlotDuration                 1310   // counter counts one extra count, see datasheet
    // execution speed related
    #define PORT_maxTxDataPrepare               40    // 2014us (measured 746us) 40
    #define PORT_maxRxAckPrepare                30    //  305us (measured  83us)
    #define PORT_maxRxDataPrepare               30    // 1007us (measured  84us)
    #define PORT_maxTxAckPrepare                40    //  305us (measured 219us)
    // radio speed related
    #define delayTx_2FSK_50                     67
    #define delayTx_OFDM1                       40       

    #define PORT_wdAckDuration                 260
    #define PORT_wdRadioTx                     140      //  for 2FSK 50 kbps 140
    #define PORT_wdDataDuration                754      //  for 2FSK 50 kbps 
    // radio watchdog
#endif

//===== adaptive_sync accuracy

#define SYNC_ACCURACY                       1     // ticks

//===== per-board number of sensors

#define NUMSENSORS      7

//====== Antenna options ====
#define BSP_ANTENNA_BASE            GPIO_D_BASE
#define BSP_ANTENNA_CC2538_24GHZ    GPIO_PIN_4      //!< PD4 -- 2.4ghz
#define BSP_ANTENNA_AT215_24GHZ     GPIO_PIN_3      //!< PD3 -- subghz


// number of radios in this board.
#define MAX_NUM_MODEM         2    // sub-GHz and 2.4 GHz interfaces or modem
#define MAX_NUM_RADIOS        3    // amount of active PHYs.  3        
//#define DAGROOT
   

//#define wdAckDuration              260                  //  5400us using 50 kbps
#define NUM_CHANNELS                3 // number of channels to channel hop on
#define DEFAULT_CH_SPACING          1200 // default channel spacing for subghz
#define DEFAULT_FREQUENCY_CENTER    863625 // defualt freque   
#define delayTx_SUBGHZ              delayTx_OFDM1
#define delayRx_SUBGHZ              0
#define NUM_CHANNELS_SUBGHZ         3

//=========================== typedef  ========================================

typedef enum {
   MODEM_2D4GHZ          = 0,
   MODEM_SUBGHZ          = 1
} modem_t;

//=========================== variables =======================================

static const uint8_t rreg_uriquery[]        = "h=ucb";
static const uint8_t infoBoardname[]        = "CC2538";
static const uint8_t infouCName[]           = "CC2538";
static const uint8_t infoRadioName[]        = "CC2538 SoC";

//=========================== prototypes ======================================

//=========================== public ==========================================
void eraseFlash(void);
//=========================== private =========================================

#endif
