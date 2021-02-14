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

//=========================== defines =========================================

#define LENGTH_BLE_CRC  3
#define LENGTH_PACKET   5+LENGTH_BLE_CRC
#define CHANNEL         0            // 24ghz: 11 = 2.405GHz, subghz: 11 = 865.325 in  FSK operating mode #1
#define TIMER_PERIOD    (32768>>0)    // (32768>>1) = 500ms @ 32kHz
#define ENABLE_DF       1

//=========================== variables =======================================

typedef struct {
    uint8_t              num_scTimerCompare;
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

void cb_scTimerCompare(void);
void cb_startFrame(PORT_TIMER_WIDTH timestamp);
void cb_endFrame(PORT_TIMER_WIDTH timestamp);

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {
    uint8_t  i;

    // clear local variables
    memset(&app_vars,0,sizeof(app_vars_t));

    // initialize board
    board_init();

    // add radio callback functions
    sctimer_set_callback(cb_scTimerCompare);
    radio_setStartFrameCb(cb_startFrame);
    radio_setEndFrameCb(cb_endFrame);

    // prepare radio
    radio_rfOn();
    // freq type only effects on scum port
    radio_setFrequency(CHANNEL, FREQ_TX);

#if ENABLE_DF == 1
    radio_configure_direction_finding_manual();
#endif

    radio_rfOff();

    // start periodic overflow
    sctimer_setCompare(sctimer_readCounter()+ TIMER_PERIOD);
    sctimer_enable();
    while(1) {

        // wait for timer to elapse
        app_vars.txpk_txNow = 0;
        while (app_vars.txpk_txNow==0) {
            board_sleep();
        }
        // freq type only effects on scum port
        radio_setFrequency(CHANNEL, FREQ_TX);
        // led
        leds_error_toggle();

        // prepare packet
        app_vars.txpk_num++;
        app_vars.txpk_len           = sizeof(app_vars.txpk_buf);
        app_vars.txpk_buf[0]        = 0x20;
        app_vars.txpk_buf[1]        = 0x03;
        app_vars.txpk_buf[2]        = 0x00;
        app_vars.txpk_buf[3]        = 0x00;
        app_vars.txpk_buf[4]        = 0x00;

        // send packet
        radio_loadPacket(app_vars.txpk_buf,app_vars.txpk_len);
        radio_txEnable();
        radio_txNow();
    }
}

//=========================== callbacks =======================================

void cb_scTimerCompare(void) {

    // update debug vals
    app_dbg.num_scTimerCompare++;
    // ready to send next packet
    app_vars.txpk_txNow = 1;
    // schedule again
    sctimer_setCompare(sctimer_readCounter()+ TIMER_PERIOD);
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
