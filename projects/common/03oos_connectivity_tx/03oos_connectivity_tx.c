/**
\brief This program shows the use of the "radio" bsp module.

Since the bsp modules for different platforms have the same declaration, you
can use this project with any platform.

The board running this program will send a packet on channel CHANNEL every
TIMER_PERIOD ticks. The packet contains LENGTH_PACKET bytes. The first byte
is the packet number, which increments for each transmitted packet. The
remainder of the packet contains an incrementing bytes.

\author Xavier Vilajosana <xvilajosana@uoc.edu>, May 2016.
\author Pere Tuset <peretuset@uoc.edu>, May 2016.
*/

#include "stdint.h"
#include "string.h"
#include "board.h"
#include "radio.h"
#include "openrandom.h"
#include "leds.h"
#include "bsp_timer.h"
#include "idmanager.h"

//=========================== defines =========================================

#define LENGTH_PACKET   98+LENGTH_CRC // maximum length is 100 bytes
#define CHANNEL         26             // 26 = 2.405GHz
#define TIMER_PERIOD    (32768)     // (32768>>1) = 500ms @ 32kHz

//=========================== variables =======================================

typedef struct {
    uint8_t              num_radioTimerCompare;
    uint8_t              num_radioTimerOverflows;
    uint8_t              num_startFrame;
    uint8_t              num_endFrame;
} app_dbg_t;

app_dbg_t app_dbg;

typedef struct {
    uint8_t               txpk_txNow;
    uint8_t               txpk_buf[LENGTH_PACKET];
    uint8_t               txpk_len;
    //uint8_t              txpk_num;
    uint16_t              packet_counter;
    uint8_t               rollover;
    open_addr_t*          address;
    uint8_t               waitPacketEnd;
    PORT_RADIOTIMER_WIDTH packet_period;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

void cb_radioTimerOverflows(void);
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
    openrandom_init();
    idmanager_init();

    // Get EUI64
    app_vars.address = idmanager_getMyID(ADDR_64B);

    // add radio callback functions
    radio_setOverflowCb(cb_radioTimerOverflows);
    radio_setStartFrameCb(cb_startFrame);
    radio_setEndFrameCb(cb_endFrame);

    // prepare radio
    radio_rfOn();
    radio_setFrequency(CHANNEL);
    radio_rfOff();

    // random packet rate
    app_vars.packet_period = openrandom_get16b() % TIMER_PERIOD + TIMER_PERIOD ;
    radiotimer_start(app_vars.packet_period);

    while(1) {
        // Wait for timer to elapse
        app_vars.txpk_txNow = 0;
        while (app_vars.txpk_txNow==0) {
            board_sleep();
        }

        // Mote type (0xBB = Bike, 0xCC = Motorike, 0xDD = Car)
        app_vars.txpk_len    = sizeof(app_vars.txpk_buf);
        app_vars.txpk_buf[0] = 0xBB;

        // EUI64 as identifier
        memcpy(&app_vars.txpk_buf[1], &app_vars.address->addr_64b[0], 8);

        // Packet counter
        app_vars.packet_counter++;

		// Detecting rollover with lollipop counter
        if (app_vars.packet_counter % 0xffffffff == 0){
            app_vars.rollover++;
            app_vars.packet_counter = 256;
        }

        // Fill in packet counter and rollover counter
        app_vars.txpk_buf[9]   = (app_vars.packet_counter >> 8) % 0xFF;
        app_vars.txpk_buf[10]  = (app_vars.packet_counter >> 0) % 0xFF;;
        app_vars.txpk_buf[11]  = app_vars.rollover;

        // Epoch set to zero as this is a bike
        app_vars.txpk_buf[12] = 0x00;
        app_vars.txpk_buf[13] = 0x00;
        app_vars.txpk_buf[14] = 0x00;
        app_vars.txpk_buf[15] = 0x00;

        // Fill remaining of packet
        for (i = 16; i < app_vars.txpk_len; i++) {
            app_vars.txpk_buf[i] = i;
        }

        // Send packet
        radio_rfOn();
        radio_loadPacket(app_vars.txpk_buf,app_vars.txpk_len);
        radio_txEnable();
        radio_txNow();

        // Radio is asynchronous
        // Wait until the packet is complete to go to deep sleep
        app_vars.waitPacketEnd = 1;
        while (app_vars.waitPacketEnd);
        
        // Stop the radio once the packet is compete
        radio_rfOff();
    }
}

//=========================== callbacks =======================================

void cb_radioTimerOverflows(void) {
    // update debug vals
    app_dbg.num_radioTimerOverflows++;

    // ready to send next packet
    app_vars.txpk_txNow = 1;

}

void cb_startFrame(PORT_TIMER_WIDTH timestamp) {
    // update debug vals
    app_dbg.num_startFrame++;
}

void cb_endFrame(PORT_TIMER_WIDTH timestamp) {
    // update debug vals
    app_dbg.num_endFrame++;

    //the radio has finished.
    app_vars.waitPacketEnd = 0;
}
