/**
\brief This program shows the use of the "radio" bsp module.

find scum channels with an OpenMote :)

author: anonymous, not the hacker, obviously. this code is really bad
*/

#include "stdint.h"
#include "string.h"
#include "board.h"
#include "radio.h"
#include "leds.h"
#include "uart.h"
#include "sctimer.h"

//=========================== defines =========================================

#define LENGTH_PACKET        10+LENGTH_CRC // maximum length is 127 bytes
#define LENGTH_PACKET_TX	 16+LENGTH_CRC
#define RX_CHANNEL_START     11             // 24ghz: 11 = 2.405GHz, subghz: 11 = 865.325 in  FSK operating mode #1
#define RX_CHANNEL_MAX		 26
#define LENGTH_SERIAL_FRAME  40              // length of the serial frame
#define TIMER_PERIOD		 (32768>>7)		// (32768>>1) = 500ms @ 32kHz
#define TIMER_PERIOD_RX		 (32768>>1)		// (32768>>8) = 7.8125ms @ 32kHz -> this is the ping timer when SCuM is transmitting (OpenMote receives)
#define TIMER_PERIOD_TX		 (32768>>6)		// (32768>>1) = 500ms @ 32kHz -> this is the timeout timer when SCuM is receiving (OpenMote transmits and listens for ack)

//=========================== variables =======================================

typedef struct {
    uint8_t    num_radioTimerCompare;
    uint8_t    num_startFrame;
    uint8_t    num_endFrame;
	uint8_t	   num_scTimerCompare;
} app_dbg_t;

app_dbg_t app_dbg;

typedef struct {
    // rx packet
    volatile    uint8_t    rxpk_done;
				uint8_t	   txpk_txNow;
                uint8_t    rxpk_buf[LENGTH_PACKET];
				uint8_t    txpk_buf[LENGTH_PACKET_TX];
                uint8_t    rxpk_len;
                uint8_t	   txpk_len;
				uint16_t   rxpk_num;
				uint16_t   txpk_num;
                int8_t     rxpk_rssi;
                uint8_t    rxpk_lqi;
                bool       rxpk_crc;
                uint8_t    rxpk_freq_offset;
	// rx channel
				uint8_t	   rx_channel;
    // uart
                uint8_t    uart_txFrame[LENGTH_SERIAL_FRAME];
                uint8_t    uart_lastTxByte;
    volatile    uint8_t    uart_done;
	
	// SCuM hunt state machine
				uint8_t	   rx_tx; // 0 if it is looking for SCuM's TX channels, 1 if it is looking for RX channels
				uint8_t	   increment_channel_request; // 1 if SCuM's receiver requested a channel increment, 0 otherwise
				
	// SCuM's TX code buffer
				uint8_t		scum_tx_codes[16*4][3]; 	// 16 channels, 4 candidate codes, [coarse, mid, fine]
				uint8_t		scum_tx_code_buffer[128][3]; // 128 possible received codes per channel, a bit overkill, [coarse, mid, fine]
				uint8_t		rx_valid_packet_counter;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

// radiotimer
void cb_radioTimerOverflows(void);
// timer
void cb_scTimerCompare(void);
// radio
void cb_startFrame(PORT_TIMER_WIDTH timestamp);
void cb_endFrame(PORT_TIMER_WIDTH timestamp);
// uart
void cb_uartTxDone(void);
uint8_t cb_uartRxCb(void);

// printing crap
void print_packet_received(void);
void print_rx_timeout(void);
void print_scum_tx_codes(void);
void print_debug(void);

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {

    uint8_t i;

    // clear local variables
    memset(&app_vars,0,sizeof(app_vars_t));
	
    // initialize to channel 11
    app_vars.rx_channel = 11;

    // initialize board
    board_init();

    // add callback functions radio
    radio_setStartFrameCb(cb_startFrame);
    radio_setEndFrameCb(cb_endFrame);
	
    // callback functions sc timer
    sctimer_set_callback(cb_scTimerCompare);

    // setup UART
    uart_setCallbacks(cb_uartTxDone,cb_uartRxCb);

    // prepare radio
    radio_rfOn();
    // freq type only effects on scum port
    radio_setFrequency(app_vars.rx_channel, FREQ_RX);

    //// switch in RX
    radio_rxEnable();
    radio_rxNow();

    // do not start timer!!
	app_vars.rx_tx = 0;
	
	// debug SCuM RX mode:
	// app_vars.rx_tx = 1;
	// sctimer_setCompare(sctimer_readCounter() + TIMER_PERIOD_RX);

    while (1) {

        // sleep while waiting for either a transmit or receive
        app_vars.rxpk_done = 0;
		app_vars.txpk_txNow = 0;
        while ((app_vars.rxpk_done==0)&&(app_vars.txpk_txNow==0)) {
            board_sleep();
        }

        if (app_vars.rxpk_done==1) {
			// if I get here, I just received a packet
					
			// packet received, update the timeout timer
			if (app_vars.rx_tx == 0) { // only update timeout timer if we're in OpenMote RX mode
				sctimer_setCompare(sctimer_readCounter() + TIMER_PERIOD_RX);
				sctimer_enable();
			}
	
			// led
			leds_error_on();

			//===== send notification over serial port
			print_packet_received();
	
			// led
			leds_error_off();
			/*
			if (app_vars.rx_channel == RX_CHANNEL_MAX) { // print results and switch to TX mode
				print_scum_tx_codes();
				
				// switch to TX mode
				app_vars.rx_tx = 1; // switch to TX mode
				
				// set ch. to 11
				app_vars.rx_channel = RX_CHANNEL_START; // set channel to 11
				
				// go into RX mode
				radio_setFrequency(app_vars.rx_channel, FREQ_RX);
				radio_rxEnable();
				radio_rxNow();
				
				// and start the TX timer
				sctimer_setCompare(sctimer_readCounter() + TIMER_PERIOD_TX);
			}*/
		}

		if (app_vars.txpk_txNow==1) {
			// freq type only effects on scum port
			radio_setFrequency(app_vars.rx_channel, FREQ_TX);
			// led
			leds_error_toggle();

			// prepare packet
			app_vars.txpk_num++;
			app_vars.txpk_len 	= sizeof(app_vars.txpk_buf);
			app_vars.txpk_buf[0]	= app_vars.txpk_num;
			app_vars.txpk_buf[1]	= app_vars.rx_channel;
			for (i=0;i<3;i++) {
				app_vars.txpk_buf[i+2]	= app_vars.scum_tx_codes[(app_vars.rx_channel-11)*4][i];
				app_vars.txpk_buf[i+5]	= app_vars.scum_tx_codes[(app_vars.rx_channel-11)*4+1][i];
				app_vars.txpk_buf[i+8]	= app_vars.scum_tx_codes[(app_vars.rx_channel-11)*4+2][i];
				app_vars.txpk_buf[i+11]	= app_vars.scum_tx_codes[(app_vars.rx_channel-11)*4+3][i];
			}


			// send packet
			radio_loadPacket(app_vars.txpk_buf, app_vars.txpk_len);
			radio_txEnable();
			radio_txNow();
			
			// switch to RX mode
			radio_setFrequency(app_vars.rx_channel, FREQ_RX);
			radio_rxEnable();
			radio_rxNow();
			
			// debug print
			if (app_vars.increment_channel_request == 1) {
				print_debug();
				app_vars.rx_channel++;
				app_vars.increment_channel_request = 0;
			}
			
			// and start the timeout counter
			sctimer_setCompare(sctimer_readCounter() + TIMER_PERIOD_TX);

		}
    }
}

//=========================== callbacks =======================================

//===== timer

void cb_scTimerCompare(void) {
	
	// loop variables
	uint8_t i;
	uint8_t j;
	
	// scum codes
	uint8_t coarse;
	uint8_t mid;
	uint8_t mid_temp;
	uint8_t fine;
	uint8_t fine_start;
	uint8_t fine_end;

	// led
	leds_error_on();

	// update debug vals
	app_dbg.num_scTimerCompare++;
	
	if (app_vars.rx_tx == 0) { // continuous receiving mode - timeout occurred
	
		// figure out which settings are "best" of the buffer settings
		coarse = app_vars.scum_tx_code_buffer[0][0];
		mid_temp = app_vars.scum_tx_code_buffer[0][1];
		fine_start = app_vars.scum_tx_code_buffer[0][2];
		i = 0;
		j = 0;
		
		// check if SCuM was at the end of a mid code (coarse will roll over).
		// if so: break and pause timer
		if (app_vars.rxpk_buf[7] >= 30) {
			
			// disable timer - will be re-enabled upon next reception on this channel
			sctimer_disable();
			
			// continue to listen
			radio_setFrequency(app_vars.rx_channel,FREQ_RX);
			radio_rxEnable();
			radio_rxNow();
		}
		
		else { // continue as usual, store the channel settings
		
			for (j=0;j<app_vars.rx_valid_packet_counter;j++) {
				if (app_vars.scum_tx_code_buffer[j][1] != mid_temp) { // find average fine code for a set of mid codes
					coarse = app_vars.scum_tx_code_buffer[j-1][0];
					fine_end = app_vars.scum_tx_code_buffer[j-1][2];
					app_vars.scum_tx_codes[(app_vars.rx_channel-11)*4+i][0] = coarse;
					app_vars.scum_tx_codes[(app_vars.rx_channel-11)*4+i][1] = mid_temp;
					app_vars.scum_tx_codes[(app_vars.rx_channel-11)*4+i][2] = (fine_start + fine_end)/2;
					
					mid_temp = app_vars.scum_tx_code_buffer[j][1];
					fine_start = app_vars.scum_tx_code_buffer[j][2];
					i++;
				}
				if (app_vars.scum_tx_code_buffer[j][0] == 0) {
					break;
				}
			}
			
			if (app_vars.rx_channel == RX_CHANNEL_MAX) { // last channel: switch to OpenMote TX mode (SCuM RX)
				// switch to TX mode
				app_vars.rx_tx = 1; // switch to TX mode
				
				// set ch. to 11
				app_vars.rx_channel = RX_CHANNEL_START; // set channel to 11
				
				// go into RX mode
				radio_setFrequency(app_vars.rx_channel, FREQ_RX);
				radio_rxEnable();
				radio_rxNow();
				
				// and start the TX timer
				sctimer_setCompare(sctimer_readCounter() + TIMER_PERIOD_TX);
			}
			else {
				// reset the RX packet counter
				app_vars.rx_valid_packet_counter = 0;
				
				// increment channel
				app_vars.rx_channel++;
				
				// and return to listen:
				radio_setFrequency(app_vars.rx_channel,FREQ_RX);
				radio_rxEnable();
				radio_rxNow();
			}
		}
	}
	
	else if (app_vars.rx_tx == 1) { // handshake mode, timeout reached without reception, transmit a packet
		app_vars.txpk_txNow = 1;
	}

}

//===== radio

void cb_startFrame(PORT_TIMER_WIDTH timestamp) {

    leds_sync_on();
    // update debug stats
    app_dbg.num_startFrame++;
	
}

void cb_endFrame(PORT_TIMER_WIDTH timestamp) {
    uint8_t  i;
    bool     expectedFrame;
	
	if (app_vars.txpk_txNow == 1) {
		// finished transmitting a packet, switch to RX mode
		app_vars.txpk_txNow = 0;
	}
	
	else {
	
		// received a packet
		// update debug stats
		app_dbg.num_endFrame++;

		memset(&app_vars.rxpk_buf[0],0,LENGTH_PACKET);

		app_vars.rxpk_freq_offset = radio_getFrequencyOffset();

		// get packet from radio
		radio_getReceivedFrame(
			app_vars.rxpk_buf,
			&app_vars.rxpk_len,
			sizeof(app_vars.rxpk_buf),
			&app_vars.rxpk_rssi,
			&app_vars.rxpk_lqi,
			&app_vars.rxpk_crc
		);

		// check the frame is sent by radio_tx project
		expectedFrame = TRUE;

		if (app_vars.rxpk_len>LENGTH_PACKET){
			expectedFrame = FALSE;
		} 

		if (app_vars.rxpk_crc == 0) {
			expectedFrame = FALSE;
		}

		// read the packet number
		//app_vars.rxpk_num = app_vars.rxpk_buf[0];

		// toggle led if the frame is expected
		if (expectedFrame){
			// increment the valid packet counter
			app_vars.rxpk_num++;

			// indicate I just received a packet from bsp_radio_tx mote
			app_vars.rxpk_done = 1;

			leds_debug_toggle();
		}

		// turn off the receiver if it's in handshake mode
		if (app_vars.rx_tx == 1){
			
			// check if SCuM sent a "change channel" request:
			if (app_vars.rxpk_buf[4] == 0xFF) {
				//app_vars.rx_channel++;
				app_vars.increment_channel_request = 1;
			}
			
			radio_rfOff();
		}

		// led
		leds_sync_off();
	}
}

//===== uart
void cb_uartTxDone(void) {

    uart_clearTxInterrupts();

    // prepare to send the next byte
    app_vars.uart_lastTxByte++;

    if (app_vars.uart_lastTxByte<sizeof(app_vars.uart_txFrame)) {
        uart_writeByte(app_vars.uart_txFrame[app_vars.uart_lastTxByte]);
    } else {
        app_vars.uart_done=1;
    }
}

uint8_t cb_uartRxCb(void) {

    //  uint8_t byte;
    uart_clearRxInterrupts();
    return 1;
}

//===== printing and storing nonsense
void print_packet_received(void){
	
	if ((app_vars.rxpk_buf[6]==0)&&(app_vars.rxpk_buf[7]==0)&&(app_vars.rxpk_buf[8]==0)) {
		// packet got messed up for unknown reasons?
		app_vars.uart_txFrame[0]	= 'F';
		app_vars.uart_txFrame[1]	= 'U';
		app_vars.uart_txFrame[2]	= 'C';
		app_vars.uart_txFrame[3]	= 'K';
		app_vars.uart_txFrame[4]	= ' ';
		app_vars.uart_txFrame[5]	= 'T';
		app_vars.uart_txFrame[6]	= 'H';
		app_vars.uart_txFrame[7]	= 'i';
		app_vars.uart_txFrame[8]	= 's';
		app_vars.uart_txFrame[9]	= '\r';
		app_vars.uart_txFrame[10]	= '\n';
	}
	
	else {
		// store the received TX settings in the buffer
		app_vars.scum_tx_code_buffer[app_vars.rx_valid_packet_counter][0] = app_vars.rxpk_buf[6];
		app_vars.scum_tx_code_buffer[app_vars.rx_valid_packet_counter][1] = app_vars.rxpk_buf[7];
		app_vars.scum_tx_code_buffer[app_vars.rx_valid_packet_counter][2] = app_vars.rxpk_buf[8];
		
		app_vars.uart_txFrame[0]  = app_vars.rxpk_crc + 42;
		app_vars.uart_txFrame[1]  = app_vars.rx_channel/10 + 48;
		app_vars.uart_txFrame[2]  = app_vars.rx_channel%10 + 48;
		app_vars.uart_txFrame[3]  = ' ';
		app_vars.uart_txFrame[4]  = app_vars.scum_tx_code_buffer[app_vars.rx_valid_packet_counter][0]/10 + 48;
		app_vars.uart_txFrame[5]  = app_vars.scum_tx_code_buffer[app_vars.rx_valid_packet_counter][0]%10 + 48;
		app_vars.uart_txFrame[6]  = ' ';
		app_vars.uart_txFrame[7]  = app_vars.scum_tx_code_buffer[app_vars.rx_valid_packet_counter][1]/10 + 48;
		app_vars.uart_txFrame[8]  = app_vars.scum_tx_code_buffer[app_vars.rx_valid_packet_counter][1]%10 + 48;
		app_vars.uart_txFrame[9]  = ' ';
		app_vars.uart_txFrame[10]  = app_vars.scum_tx_code_buffer[app_vars.rx_valid_packet_counter][2]/10 + 48;
		app_vars.uart_txFrame[11]  = app_vars.scum_tx_code_buffer[app_vars.rx_valid_packet_counter][2]%10 + 48;
		app_vars.uart_txFrame[12]  = ' ';
		app_vars.uart_txFrame[13]  = app_vars.rxpk_num/1000 + 48;
		app_vars.uart_txFrame[14] = app_vars.rxpk_num/100 + 48;
		app_vars.uart_txFrame[15] = (app_vars.rxpk_num - 100*(app_vars.rxpk_num/100))/10 + 48;
		app_vars.uart_txFrame[16] = app_vars.rxpk_num%10 + 48;
		app_vars.uart_txFrame[17] = '\r';
		app_vars.uart_txFrame[18] = '\n';
		app_vars.uart_done          = 0;
		app_vars.uart_lastTxByte    = 0;
		
		app_vars.rx_valid_packet_counter++;
		

		// send app_vars.uart_txFrame over UART
		uart_clearTxInterrupts();
		uart_clearRxInterrupts();
		uart_enableInterrupts();
		uart_writeByte(app_vars.uart_txFrame[app_vars.uart_lastTxByte]);
		while (app_vars.uart_done==0); // busy wait to finish
		uart_disableInterrupts();
	}
}

void print_rx_timeout(void) {
	app_vars.uart_txFrame[0] = 'T';
	app_vars.uart_txFrame[1] = 'I';
	app_vars.uart_txFrame[2] = 'M';
	app_vars.uart_txFrame[3] = 'E';
	app_vars.uart_txFrame[4] = ' ';
	app_vars.uart_txFrame[5] = 'O';
	app_vars.uart_txFrame[6] = 'U';
	app_vars.uart_txFrame[7] = 'T';
	app_vars.uart_txFrame[8] = '\r';
	app_vars.uart_txFrame[9] = '\n';
	app_vars.uart_done 		 = 0;
	app_vars.uart_lastTxByte = 0;
	
	// send app_vars.uart_txFrame over UART
	uart_clearTxInterrupts();
	uart_clearRxInterrupts();
	uart_enableInterrupts();
	uart_writeByte(app_vars.uart_txFrame[app_vars.uart_lastTxByte]);
	while (app_vars.uart_done==0); // busy wait to finish
	uart_disableInterrupts();
}

void print_scum_tx_codes(void) {
	
	app_vars.uart_txFrame[0] = app_vars.scum_tx_codes[0][0]/10+'0';
	app_vars.uart_txFrame[1] = app_vars.scum_tx_codes[1][0]%10+'0';
	app_vars.uart_txFrame[2] = ' ';
	app_vars.uart_txFrame[3] = app_vars.scum_tx_codes[0][1]/10+'0';
	app_vars.uart_txFrame[4] = app_vars.scum_tx_codes[0][1]%10+'0';
	app_vars.uart_txFrame[5] = ' ';
	app_vars.uart_txFrame[6] = app_vars.scum_tx_codes[0][2]/10+'0';
	app_vars.uart_txFrame[7] = app_vars.scum_tx_codes[0][2]%10+'0';
	app_vars.uart_txFrame[8] = ' ';
	app_vars.uart_txFrame[9] = app_vars.scum_tx_codes[1][0]/10+'0';
	app_vars.uart_txFrame[10] = app_vars.scum_tx_codes[1][0]%10+'0';
	app_vars.uart_txFrame[11] = ' ';
	app_vars.uart_txFrame[12] = app_vars.scum_tx_codes[1][1]/10+'0';
	app_vars.uart_txFrame[13] = app_vars.scum_tx_codes[1][1]%10+'0';
	app_vars.uart_txFrame[14] = ' ';
	app_vars.uart_txFrame[15] = app_vars.scum_tx_codes[1][2]/10+'0';
	app_vars.uart_txFrame[16] = app_vars.scum_tx_codes[1][2]%10+'0';
	app_vars.uart_txFrame[17] = ' ';
	app_vars.uart_txFrame[18] = app_vars.scum_tx_codes[2][0]/10+'0';
	app_vars.uart_txFrame[19] = app_vars.scum_tx_codes[2][0]%10+'0';
	app_vars.uart_txFrame[20] = ' ';
	app_vars.uart_txFrame[21] = app_vars.scum_tx_codes[2][1]/10+'0';
	app_vars.uart_txFrame[22] = app_vars.scum_tx_codes[2][1]%10+'0';
	app_vars.uart_txFrame[23] = ' ';
	app_vars.uart_txFrame[24] = app_vars.scum_tx_codes[2][2]/10+'0';
	app_vars.uart_txFrame[25] = app_vars.scum_tx_codes[2][2]%10+'0';
	app_vars.uart_txFrame[26] = ' ';
	app_vars.uart_txFrame[27] = app_vars.scum_tx_codes[3][0]/10+'0';
	app_vars.uart_txFrame[28] = app_vars.scum_tx_codes[3][0]%10+'0';
	app_vars.uart_txFrame[29] = ' ';
	app_vars.uart_txFrame[30] = app_vars.scum_tx_codes[3][1]/10+'0';
	app_vars.uart_txFrame[31] = app_vars.scum_tx_codes[3][1]%10+'0';
	app_vars.uart_txFrame[32] = ' ';
	app_vars.uart_txFrame[33] = app_vars.scum_tx_codes[3][2]/10+'0';
	app_vars.uart_txFrame[34] = app_vars.scum_tx_codes[3][2]%10+'0';
	app_vars.uart_txFrame[35] = '\r';
	app_vars.uart_txFrame[36] = '\n';
	
	app_vars.uart_done 		 = 0;
	app_vars.uart_lastTxByte = 0;
	// send app_vars.uart_txFrame over UART
	uart_clearTxInterrupts();
	uart_clearRxInterrupts();
	uart_enableInterrupts();
	uart_writeByte(app_vars.uart_txFrame[app_vars.uart_lastTxByte]);
	while (app_vars.uart_done==0); // busy wait to finish
	uart_disableInterrupts();
	// send app_vars.uart_txFrame over UART
}

void print_debug(void) {
	app_vars.uart_txFrame[0] = 'G';
	app_vars.uart_txFrame[1] = 'O';
	app_vars.uart_txFrame[2] = 'T';
	app_vars.uart_txFrame[3] = ' ';
	app_vars.uart_txFrame[4] = 'A';
	app_vars.uart_txFrame[5] = 'N';
	app_vars.uart_txFrame[6] = ' ';
	app_vars.uart_txFrame[7] = 'F';
	app_vars.uart_txFrame[8] = 'F';
	app_vars.uart_txFrame[9] = '\r';
	app_vars.uart_txFrame[10] = '\n';
	app_vars.uart_done 		 = 0;
	app_vars.uart_lastTxByte = 0;
	
	// send app_vars.uart_txFrame over UART
	uart_clearTxInterrupts();
	uart_clearRxInterrupts();
	uart_enableInterrupts();
	uart_writeByte(app_vars.uart_txFrame[app_vars.uart_lastTxByte]);
	while (app_vars.uart_done==0); // busy wait to finish
	uart_disableInterrupts();
}
