/**
\brief This program shows the use of the "radio" bsp module.

Since the bsp modules for different platforms have the same declaration, you
can use this project with any platform.

This application places the mote in receive mode, and prints, over the serial
port, all information about the received packet. The frame printed over the
serial port for each received packet is formatted as follows:
- [1B] the length of the packet, an unsigned integer
- [1B] the first byte of the packet, an unsigned integer
- [1B] the receive signal strength of tehe packet, an signed integer
- [1B] the link quality indicator, an unsigned integer
- [1B] whether the receive packet passed CRC (1) or not (0)
- [3B] closing flags, each of value 0xff

You can run the 01bsp_radio_rx.py script to listen to your mote and parse those
serial frames. The application can connect directly to the mote's serial port,
or to its TCP port when running on the IoT-LAB platform.

Example when running locally:
----------------------------

 ___                 _ _ _  ___  _ _
| . | ___  ___ ._ _ | | | |/ __>| \ |
| | || . \/ ._>| ' || | | |\__ \|   |
`___'|  _/\___.|_|_||__/_/ <___/|_\_|
     |_|                  openwsn.org

running IoT-lAB? (Y|N): N
name of serial port (e.g. COM10): COM25
len=127 num=176 rssi=-43  lqi=107 crc=1
len=127 num=177 rssi=-43  lqi=107 crc=1
len=127 num=178 rssi=-43  lqi=106 crc=1
len=127 num=179 rssi=-43  lqi=107 crc=1
len=127 num=180 rssi=-43  lqi=108 crc=1
len=127 num=181 rssi=-43  lqi=107 crc=1
len=127 num=182 rssi=-43  lqi=107 crc=1
len=127 num=183 rssi=-43  lqi=107 crc=1


Example when running on the IoT-LAB platform:
--------------------------------------------

 ___                 _ _ _  ___  _ _
| . | ___  ___ ._ _ | | | |/ __>| \ |
| | || . \/ ._>| ' || | | |\__ \|   |
`___'|  _/\___.|_|_||__/_/ <___/|_\_|
     |_|                  openwsn.org

running IoT-lAB? (Y|N): Y
motename? (e.g. wsn430-35): wsn430-35
len=17  num=84  rssi=-80  lqi=107 crc=1
len=17  num=84  rssi=-81  lqi=107 crc=1
len=17  num=84  rssi=-80  lqi=107 crc=1
len=17  num=84  rssi=-81  lqi=105 crc=1
len=17  num=84  rssi=-80  lqi=108 crc=1
len=17  num=84  rssi=-81  lqi=108 crc=1


\author Xavi Vilajosana xvilajosana@eecs.berkeley.edu>, June 2012.
\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2014.
*/

#include "stdint.h"
#include "string.h"
#include "board.h"
#include "radio.h"
#include "leds.h"
#include "uart.h"
//#include "sctimer.h"

//=========================== defines =========================================

#define LENGTH_PACKET        125+LENGTH_CRC // maximum length is 127 bytes
#define CHANNEL              11             // 24ghz: 11 = 2.405GHz, subghz: 11 = 865.325 in  FSK operating mode #1
#define LENGTH_SERIAL_FRAME  9              // length of the serial frame

//=========================== variables =======================================

typedef struct {
    uint8_t    num_radioTimerCompare;
    uint8_t    num_startFrame;
    uint8_t    num_endFrame;
} app_dbg_t;

app_dbg_t app_dbg;

typedef struct {
    // rx packet
    volatile    uint8_t    rxpk_done;
                uint8_t    rxpk_buf[LENGTH_PACKET];
                uint8_t    rxpk_len;
                uint8_t    rxpk_num;
                int8_t     rxpk_rssi;
                uint8_t    rxpk_lqi;
                bool       rxpk_crc;
                uint8_t    rxpk_freq_offset;
    // uart
                uint8_t    uart_txFrame[LENGTH_SERIAL_FRAME];
                uint8_t    uart_lastTxByte;
    volatile    uint8_t    uart_done;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

// radiotimer
void cb_radioTimerOverflows(void);
// radio
void cb_startFrame(PORT_TIMER_WIDTH timestamp);
void cb_endFrame(PORT_TIMER_WIDTH timestamp);
// uart
void cb_uartTxDone(void);
uint8_t cb_uartRxCb(void);

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {

    uint8_t i;

    // clear local variables
    memset(&app_vars,0,sizeof(app_vars_t));

    // initialize board
    board_init();

    // add callback functions radio
    radio_setStartFrameCb(cb_startFrame);
    radio_setEndFrameCb(cb_endFrame);

    // setup UART
    uart_setCallbacks(cb_uartTxDone,cb_uartRxCb);

    // prepare radio
    radio_rfOn();
    // freq type only effects on scum port
    radio_setFrequency(CHANNEL, FREQ_RX);

    // switch in RX
    radio_rxEnable();
    radio_rxNow();

    while (1) {

        // sleep while waiting for at least one of the rxpk_done to be set

        app_vars.rxpk_done = 0;
        while (app_vars.rxpk_done==0) {
            board_sleep();
        }

        // if I get here, I just received a packet

        //===== send notification over serial port

        // led
        leds_error_on();

        // format frame to send over serial port
        i = 0;
        app_vars.uart_txFrame[i++] = app_vars.rxpk_len;  // packet length
        app_vars.uart_txFrame[i++] = app_vars.rxpk_num;  // packet number
        app_vars.uart_txFrame[i++] = app_vars.rxpk_rssi; // RSSI
        app_vars.uart_txFrame[i++] = app_vars.rxpk_lqi;  // LQI
        app_vars.uart_txFrame[i++] = app_vars.rxpk_crc;  // CRC
        app_vars.uart_txFrame[i++] = app_vars.rxpk_freq_offset; // freq_offset
        app_vars.uart_txFrame[i++] = 0xff;               // closing flag
        app_vars.uart_txFrame[i++] = 0xff;               // closing flag
        app_vars.uart_txFrame[i++] = 0xff;               // closing flag

        app_vars.uart_done          = 0;
        app_vars.uart_lastTxByte    = 0;

        // send app_vars.uart_txFrame over UART
        uart_clearTxInterrupts();
        uart_clearRxInterrupts();
        uart_enableInterrupts();
        uart_writeByte(app_vars.uart_txFrame[app_vars.uart_lastTxByte]);
        while (app_vars.uart_done==0); // busy wait to finish
        uart_disableInterrupts();

        // led
        leds_error_off();
    }
}

//=========================== callbacks =======================================

//===== radio

void cb_startFrame(PORT_TIMER_WIDTH timestamp) {

    leds_sync_on();
    // update debug stats
    app_dbg.num_startFrame++;
}

void cb_endFrame(PORT_TIMER_WIDTH timestamp) {
    uint8_t  i;
    bool     expectedFrame;

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
    } else {
        for(i=1;i<10;i++){
            if(app_vars.rxpk_buf[i]!=i){
                expectedFrame = FALSE;
                break;
            }
        }
    }

    // read the packet number
    app_vars.rxpk_num = app_vars.rxpk_buf[0];

    // toggle led if the frame is expected
    if (expectedFrame){
        // indicate I just received a packet from bsp_radio_tx mote
        app_vars.rxpk_done = 1;

        leds_debug_toggle();
    }

    // keep listening (needed for at86rf215 radio)
    radio_rxEnable();
    radio_rxNow();

    // led
    leds_sync_off();
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
