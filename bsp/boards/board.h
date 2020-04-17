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

typedef enum {
   DO_NOT_KICK_SCHEDULER = 0,
   KICK_SCHEDULER        = 1,
} kick_scheduler_t;

//=========================== typedef =========================================
typedef struct {
    uint16_t PORT_SLOTDURATION;
    uint16_t PORT_TsSlotDuration;

    // execution speed related
    // also implementation related (e.g. SoC radio or spi-connected radio because of the time to load a packet to/from radio)
    uint16_t PORT_maxTxDataPrepare;
    uint16_t PORT_maxRxAckPrepare;
    uint16_t PORT_maxRxDataPrepare;
    uint16_t PORT_maxTxAckPrepare;

    // radio speed related
    // also implementation related (because the time to execute the Tx/Rx command is highly dependent on the radio AND the configuration)
    uint16_t PORT_delayTx;                       
    uint16_t PORT_delayRx;
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
extern slot_board_vars_t slot_board_vars [MAX_SLOT_TYPES];
extern slotType_t selected_slot_type;

//=========================== prototypes ======================================

void board_init(void);
void board_sleep(void);
void board_reset(void);
void board_init_slot_vars(void);

/**
\}
\}
*/

#endif
