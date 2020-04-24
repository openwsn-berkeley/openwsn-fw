#ifndef __BOARD_H
#define __BOARD_H

/**
\addtogroup BSP
\{
\addtogroup board
\{

\brief Cross-platform declaration "board" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "board_info.h"
#include "toolchain_defs.h"

//=========================== define ==========================================
#define SLOTDURATION_MS     board_getSlotDuration()/PORT_TICS_PER_MS

typedef enum {
   DO_NOT_KICK_SCHEDULER = 0,
   KICK_SCHEDULER        = 1,
} kick_scheduler_t;



//=========================== typedef =========================================
typedef struct {
    uint16_t slotDuration;

    // execution speed related
    // also implementation related (e.g. SoC radio or spi-connected radio because of the time to load a packet to/from radio)
    uint16_t maxTxDataPrepare;
    uint16_t maxRxAckPrepare;
    uint16_t maxRxDataPrepare;
    uint16_t maxTxAckPrepare;

    // radio speed related
    // also implementation related (because the time to execute the Tx/Rx command is highly dependent on the radio AND the configuration)
    uint16_t delayTx;                       
    uint16_t delayRx;
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

//=========================== variables =======================================

//=========================== prototypes ======================================

void              board_init(void);
void              board_sleep(void);
void              board_reset(void);
uint16_t          board_getSlotDuration (void);
slot_board_vars_t board_selectSlotTemplate(slotType_t slot_type);
/**
\}
\}
*/
//=========================== private =========================================
void              board_init_slot_vars(void);
#endif
