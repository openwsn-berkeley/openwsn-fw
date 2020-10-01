/**
\brief This program shows the use of the "radio" bsp module.
*/

#include "stdint.h"
#include "string.h"
#include "board.h"
#include "radio.h"
#include "radio_df.h"
#include "leds.h"
#include "uart.h"
#include "sctimer.h"

//=========================== defines =========================================

#define NUM_SAMPLES           SAMPLE_MAXCNT

#define LENGTH_PACKET         125+LENGTH_CRC // maximum length is 127 bytes
#define CHANNEL               0             // 24ghz: 11 = 2.405GHz, subghz: 11 = 865.325 in  FSK operating mode #1
#define LENGTH_SERIAL_FRAME   ((NUM_SAMPLES*4)+9)   // length of the serial frame

#define DF_ENABLE             1


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
               uint32_t    sample_buffer[NUM_SAMPLES];
               uint16_t    num_samples;
                uint8_t    setting_coarse;
                uint8_t    setting_mid;
                uint8_t    setting_fine;
    // uart
                uint8_t    uart_txFrame[LENGTH_SERIAL_FRAME];
               uint16_t    uart_lastTxByte;
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

#ifdef DF_ENABLE
    radio_configure_switch_antenna_array();
    radio_configure_direction_finding_antenna_switch();
    radio_configure_direction_finding_manual();
#endif

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

        for (i=0;i<NUM_SAMPLES;i++) {
            app_vars.uart_txFrame[4*i+0] = (app_vars.sample_buffer[i] >>24) & 0x000000ff;
            app_vars.uart_txFrame[4*i+1] = (app_vars.sample_buffer[i] >>16) & 0x000000ff;
            app_vars.uart_txFrame[4*i+2] = (app_vars.sample_buffer[i] >> 8) & 0x000000ff;
            app_vars.uart_txFrame[4*i+3] = (app_vars.sample_buffer[i] >> 0) & 0x000000ff;
        }
        
        app_vars.uart_txFrame[4*i+0]     = app_vars.rxpk_rssi;
        // record scum settings for transmitting
        app_vars.uart_txFrame[4*i+1]     = app_vars.setting_coarse;
        app_vars.uart_txFrame[4*i+2]     = app_vars.setting_mid;
        app_vars.uart_txFrame[4*i+3]     = app_vars.setting_fine;

        app_vars.uart_txFrame[4*i+4]     = radio_get_antenna_array_id();

        app_vars.uart_txFrame[4*i+5]     = 0xff;
        app_vars.uart_txFrame[4*i+6]     = 0xff;
        app_vars.uart_txFrame[4*i+7]     = 0xff;
        app_vars.uart_txFrame[4*i+8]     = 0xff;

        app_vars.uart_done          = 0;
        app_vars.uart_lastTxByte    = 0;

        // send app_vars.uart_txFrame over UART
        uart_clearTxInterrupts();
        uart_clearRxInterrupts();
        uart_enableInterrupts();
        uart_writeByte(app_vars.uart_txFrame[app_vars.uart_lastTxByte]);
        while (app_vars.uart_done==0); // busy wait to finish
        uart_disableInterrupts();

        debugpins_fsm_toggle();

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
    app_vars.num_samples    = radio_get_df_samples(app_vars.sample_buffer,NUM_SAMPLES);
    app_vars.setting_coarse = app_vars.rxpk_buf[2];
    app_vars.setting_mid    = app_vars.rxpk_buf[3];
    app_vars.setting_fine   = app_vars.rxpk_buf[4];

    if (app_vars.rxpk_crc && app_vars.rxpk_len==5) {
        debugpins_slot_toggle();

        // indicate I just received a packet from bsp_radio_tx mote
        app_vars.rxpk_done = 1;
    }

#ifdef DF_ENABLE
    //radio_configure_switch_antenna_array();
    radio_configure_direction_finding_antenna_switch();
    radio_configure_direction_finding_manual();
#endif

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

    if (app_vars.uart_lastTxByte<LENGTH_SERIAL_FRAME) {
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
