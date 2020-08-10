/**
 * Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Date:   Jan 2016
 * Description: ezr32wg-specific definition of the "board" bsp module.
 */


#include "board.h"
#include "leds.h"
#include "bsp_timer.h"
#include "radiotimer.h"
#include "debugpins.h"
#include "uart.h"
#include "radio.h"
#include "em_cmu.h"
#include "em_chip.h"
#include "em_emu.h"
#include "spi.h"

//=========================== variables =======================================

slot_board_vars_t slot_board_vars [MAX_SLOT_TYPES];
slotType_t selected_slot_type;

//=========================== prototypes ======================================

static void button_init(void);


//=========================== main ============================================

extern int mote_main(void);

int main(void) {
   return mote_main();
}

//=========================== public ==========================================

void board_init(void) {
   CHIP_Init();

   leds_init();
   debugpins_init();
   button_init();
   bsp_timer_init();
   radiotimer_init();
   uart_init();
   radio_init();
   board_init_slot_vars();
   //spi_init();
}

//====  IEEE802154E timing: bootstrapping slot info lookup table
// 1 clock tick = 30.5 us
void board_init_slot_vars(void){

    // 20ms slot
    slot_board_vars [SLOT_10ms_24GHZ].slotDuration                   = 328 ; // tics  
    slot_board_vars [SLOT_10ms_24GHZ].maxTxDataPrepare               = 10  ; // 305us (based on measurement)
    slot_board_vars [SLOT_10ms_24GHZ].maxRxAckPrepare                = 10  ; // 305us (based on measurement)
    slot_board_vars [SLOT_10ms_24GHZ].maxRxDataPrepare               = 4   ; // 122us (based on measurement)
    slot_board_vars [SLOT_10ms_24GHZ].maxTxAckPrepare                = 10  ; // 305us (based on measurement)
    
    #ifdef OPENWSN_IEEE802154E_SECURITY_C
        slot_board_vars [SLOT_10ms_24GHZ].delayTx                    = 7   ; //  214us (measured xxxus)
    #else
        slot_board_vars [SLOT_10ms_24GHZ].delayTx                    = 12  ; //  366us (measured xxxus)
    #endif
    slot_board_vars [SLOT_10ms_24GHZ].delayRx                        = 0   ; // 0us (can not measure)
}

// To get the current slotDuration at any time (in tics)
// if you need the value in MS, divide by PORT_TICS_PER_MS (which varies by board and clock frequency and defined in board_info.h)
uint16_t board_getSlotDuration (void){
    return slot_board_vars [selected_slot_type].slotDuration;
}

// Setter/Getter function for slot_board_vars
slot_board_vars_t board_selectSlotTemplate (slotType_t slot_type){
    selected_slot_type = slot_type;
    return slot_board_vars [selected_slot_type];
}

/**
 * Puts the board to sleep
 */
void board_sleep(void) {
    //EMU_EnterEM1();
}

/**
 * Resets the board
 */
void board_reset(void) {
	NVIC_SystemReset();
}


//=========================== private =========================================


/**
 * Configures the user button as input source
 */
static void button_init(void) {

}
//=========================== interrupt handlers ==============================

