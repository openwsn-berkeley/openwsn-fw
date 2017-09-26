/**
\brief This program shows the use of the "radio" bsp module.

Since the bsp modules for different platforms have the same declaration, you
can use this project with any platform.

The board running this program will send a packet on channel CHANNEL every
TIMER_PERIOD ticks. The packet contains LENGTH_PACKET bytes. The first byte
is the packet number, which increments for each transmitted packet. The
remainder of the packet contains an incrementing bytes.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2014.
*/

#include "stdint.h"
#include "string.h"
#include "board.h"
#include "radio.h"
#include "leds.h"
#include "sctimer.h"
#include "at86rf215.h"

#include <source/gpio.h>
#include <headers/hw_memmap.h>
#include <headers/hw_types.h>
//=========================== defines =========================================

#define LENGTH_PACKET   125+LENGTH_CRC // maximum length is 127 bytes
#define CHANNEL         11             // 11 = 2.405GHz
#define TIMER_PERIOD    (32768>>1)     // (32768>>1) = 500ms @ 32kHz

//=========================== variables =======================================

typedef struct {
   uint8_t              num_radioTimerCompare;
   uint8_t              num_radioTimerOverflows;
   uint8_t              num_startFrame;
   uint8_t              num_endFrame;
} app_dbg_t;

app_dbg_t app_dbg;

typedef struct {
   uint8_t              txpk_txNow;
   uint8_t              txpk_buf[LENGTH_PACKET];
   uint8_t              txpk_len;
   uint8_t              txpk_num;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

void cb_radioTimerOverflows(void);
void cb_radioTimerCompare(void);
void cb_startFrame(PORT_TIMER_WIDTH timestamp);
void cb_endFrame(PORT_TIMER_WIDTH timestamp);

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {
   
   // clear local variables
   memset(&app_vars,0,sizeof(app_vars_t));
   
   // initialize board
   board_init();
   // enable power
   GPIOPinWrite(GPIO_C_BASE, GPIO_PIN_0, 0);
   //reset the radio
   GPIOPinWrite(GPIO_D_BASE, GPIO_PIN_1, GPIO_PIN_1);
   
   at86rf215_spiWriteReg( RG_RF_RST, CMD_RF_RESET); 
   at86rf215_spiStrobe(CMD_RF_TRXOFF);
   while(at86rf215_status() != RF_STATE_TRXOFF);
    
   if ((at86rf215_spiReadReg(RG_RF_PN) != 0x34) | (at86rf215_spiReadReg(RG_RF_VN) != 0x03)) {
       leds_error_on();
   }else{
       leds_radio_on();
   }  
   while (1);
  return 0;
}

//=========================== callbacks =======================================

void cb_radioTimerCompare(void) {
   
   // update debug vals
   app_dbg.num_radioTimerCompare++;
}

void cb_radioTimerOverflows(void) {
   
   // update debug vals
   app_dbg.num_radioTimerOverflows++;
   
   // ready to send next packet
   app_vars.txpk_txNow = 1;
}

void cb_startFrame(PORT_TIMER_WIDTH timestamp) {
   
   // update debug vals
   app_dbg.num_startFrame++;
   
   // led
   leds_sync_on();
}

void cb_endFrame(PORT_TIMER_WIDTH timestamp) {
   
   // update debug vals
   app_dbg.num_endFrame++;
   
   // led
   leds_sync_off();
}
