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

//=========================== includes ========================================

#include "stdint.h"
#include "string.h"
#include "board.h"
#include "radio.h"
#include "openrandom.h"
#include "leds.h"
#include "bsp_timer.h"
#include "idmanager.h"

//=========================== defines =========================================

#define LENGTH_PACKET   ( 98 + LENGTH_CRC)
#define RADIO_CHANNEL   ( 26 )
#define TIMER_PERIOD    ( 32768 )

//=========================== variables =======================================

typedef struct {
    bool                  txpk_txNow;
    uint8_t               txpk_buf[LENGTH_PACKET];
    uint8_t               txpk_len;
    uint16_t              packet_counter;
    uint8_t               rollover;
    open_addr_t*          address;
    bool                  waitPacketEnd;
    PORT_RADIOTIMER_WIDTH packet_period;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

void prepare_radio_tx_frame(void);
void radio_tx_frame(void);

void cb_radioTimerOverflows(void);
void cb_startFrame(PORT_TIMER_WIDTH timestamp);
void cb_endFrame(PORT_TIMER_WIDTH timestamp);

//=========================== main ============================================

int mote_main(void) {
    // Clear local variables
    memset(&app_vars,0,sizeof(app_vars_t));

    // Initialize board
    board_init();
    openrandom_init();
    idmanager_init();

    // Get EUI64
    app_vars.address = idmanager_getMyID(ADDR_64B);

    // Add radio callback functions
    radio_setOverflowCb(cb_radioTimerOverflows);
    radio_setStartFrameCb(cb_startFrame);
    radio_setEndFrameCb(cb_endFrame);

    // Prepare radio
    radio_rfOn();
    radio_setFrequency(RADIO_CHANNEL);
    radio_rfOff();

    // Random packet transmission rate
    app_vars.packet_period = openrandom_get16b() % TIMER_PERIOD + TIMER_PERIOD;
    radiotimer_start(app_vars.packet_period);

    // Forever and ever!
    while (true) {
        // Wait for timer to elapse
        app_vars.txpk_txNow = false;
        while (app_vars.txpk_txNow == false) {
            board_sleep();
        }

        // Prepare and transmit the frame
        prepare_radio_tx_frame();
        radio_tx_frame();
    }
}

void prepare_radio_tx_frame(void) {
	uint8_t i;

    // Upate packet length
    app_vars.txpk_len = sizeof(app_vars.txpk_buf);

    // Mote type (0xAA = Bike, 0x55 = Motorike / Car)
	app_vars.txpk_buf[0] = 0xAA;

	// Copy EUI64 as identifier
	memcpy(&app_vars.txpk_buf[1], &app_vars.address->addr_64b[0], 8);

	// Increment packet counter
	app_vars.packet_counter++;

	// Detecting rollover with lollipop counter
	if (app_vars.packet_counter % 0xFFFFFFFF == 0){
		app_vars.rollover++;
		app_vars.packet_counter = 256;
	}

	// Fill in packet counter and rollover counter
	app_vars.txpk_buf[9]  = (app_vars.packet_counter >> 8) % 0xFF;
	app_vars.txpk_buf[10] = (app_vars.packet_counter >> 0) % 0xFF;;
	app_vars.txpk_buf[11] = app_vars.rollover;

	// Epoch set to zero as this is a bike
	app_vars.txpk_buf[12] = 0x00;
	app_vars.txpk_buf[13] = 0x00;
	app_vars.txpk_buf[14] = 0x00;
	app_vars.txpk_buf[15] = 0x00;

	// Fill remaining of packet
	for (i = 16; i < app_vars.txpk_len; i++) {
		app_vars.txpk_buf[i] = i;
	}
}

void radio_tx_frame(void) {
   // Enable radio
   radio_rfOn();

   // Load packet to radio
   radio_loadPacket(app_vars.txpk_buf, app_vars.txpk_len);

   // Transmit radio frame
   radio_txEnable();
   radio_txNow();

	// Radio is asynchronous
	// Wait until the packet is complete to go to deep sleep
	app_vars.waitPacketEnd = true;
	while (app_vars.waitPacketEnd == true);

	// Stop the radio once the packet is compete
	radio_rfOff();
}

//=========================== callbacks =======================================

void cb_radioTimerOverflows(void) {
    // Ready to send next packet
    app_vars.txpk_txNow = true;
}

void cb_startFrame(PORT_TIMER_WIDTH timestamp) {
}

void cb_endFrame(PORT_TIMER_WIDTH timestamp) {
    // The radio has finished
    app_vars.waitPacketEnd = false;
}
