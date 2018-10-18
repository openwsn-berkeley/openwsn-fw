/**
\brief This is a project specifically for SCuM showsing the issue SCuM when 
using multiple timers feature to schedule different radio actions.

The main behaviours of this project is to mimic a RX slot. Three compare timers 
are used for scheduling RX_ENABLE, ENABLE_TIMEOUT and LISTENING_GUARDTIME inside
a slot. Besides, UART is used in a while loop in main function for sending some 
dummy text to show how it influence the TIMERS.

The expected result of this project is: 
1. The interrupt for RX_ENABLE fired all the time if don't use UART.
2. The interrupt for RX_ENABLE starts is missing occationally if the UART sends 
data less often (controlled with a do-nothing for loop).
3. The interrupt for RX_ENABLE is missing all the time if UART is busying 
sending always. 

\author Tengfei Chang <tengfei.chang@inria.fr>, November 2017.
*/

#include "board.h"
#include "radio.h"
#include "leds.h"
#include "sctimer.h"
#include "debugpins.h"
#include "uart.h"
#include "memory_map.h"

//=========================== defines =========================================

#define LENGTH_PACKET   125+LENGTH_CRC ///< maximum length is 127 bytes
#define CHANNEL         11             ///< 11  =2.405GHz
#define TIMER_PERIOD    491            ///< 491 = 15ms@32kHz
#define ID              0x99           ///< byte sent in the packets

// FSM timer durations (combinations of atomic durations)
// TX
#define DURATION_tt1 131-PORT_delayTx-PORT_maxTxDataPrepare
#define DURATION_tt2 131-PORT_delayTx
#define DURATION_tt3 131-PORT_delayTx+33
#define DURATION_tt4 164
#define DURATION_tt5 151-16-delayRx-PORT_maxRxAckPrepare
#define DURATION_tt6 151-16-delayRx
#define DURATION_tt7 151+16
#define DURATION_tt8 PORT_maxTxAckPrepare
// RX
#define DURATION_rt1 131-36-PORT_maxRxDataPrepare
#define DURATION_rt2 131-36
#define DURATION_rt3 131+36
#define DURATION_rt4 164
#define DURATION_rt5 151-PORT_delayTx-PORT_maxTxAckPrepare
#define DURATION_rt6 151-PORT_delayTx
#define DURATION_rt7 151-PORT_delayTx+33
#define DURATION_rt8 PORT_maxTxAckPrepare

//=========================== variables =======================================

enum {
   APP_FLAG_START_FRAME = 0x01,
   APP_FLAG_END_FRAME   = 0x02,
   APP_FLAG_TIMER       = 0x04,
};

typedef enum {
    APP_STATE_IDLE           = 0x00,
    APP_STATE_RXDATAOFFSET   = 0x01,   // waiting to prepare for Tx data
    APP_STATE_RXDATAPREPARE  = 0x02,   // preparing for Tx data
    APP_STATE_RXDATAREADY    = 0x03,   // ready to Tx data, waiting for 'go'
    APP_STATE_RXDATALISTEN   = 0x04,   // 'go' signal given, waiting for SFD Tx data
} app_state_t;

typedef struct {
   uint8_t              flags;
   app_state_t          state;
   uint8_t              packet[LENGTH_PACKET];
   uint8_t              packet_len;
    int8_t              rxpk_rssi;
   uint8_t              rxpk_lqi;
   bool                 rxpk_crc;
   uint32_t             startOfSlotReference;
    uint8_t             debug[10];
    uint8_t             debug_index;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

void     cb_startFrame(PORT_TIMER_WIDTH timestamp);
void     cb_endFrame(PORT_TIMER_WIDTH timestamp);
void     cb_timer(void);
void     cb_action_timer(void);

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
    
    // prepare packet
    app_vars.packet_len = sizeof(app_vars.packet);
    for (i=0;i<app_vars.packet_len;i++) {
        app_vars.packet[i] = ID;
    }
    
    // start bsp timer
    sctimer_set_callback(cb_timer);
    sctimer_setCompare(sctimer_readCounter()+TIMER_PERIOD);
    sctimer_enable();
    
    sctimer_set_actionCallback(cb_action_timer);
    
    // prepare radio
    radio_rfOff();
    radio_setFrequency(CHANNEL);

    while(1){
        // send dummy text
        uart_writeByte('*');
        // reduce the usage of UART
        for (i=0;i<0xff;i++);
    }
}

//=========================== callbacks =======================================

void cb_startFrame(PORT_TIMER_WIDTH timestamp) {
   // set flag
   app_vars.flags |= APP_FLAG_START_FRAME;
}

void cb_endFrame(PORT_TIMER_WIDTH timestamp) {
   // set flag
   app_vars.flags |= APP_FLAG_END_FRAME;
}

void cb_timer(void) {
    
    uint8_t i;
    debugpins_slot_toggle();
    
    app_vars.startOfSlotReference = sctimer_readCounter();
    
    sctimer_setCompare(app_vars.startOfSlotReference+TIMER_PERIOD);
    
    if (app_vars.state != APP_STATE_IDLE){
        leds_error_toggle();
    }
    debugpins_fsm_clr();
    
    for (i=0;i<10;i++){
        uart_writeByte(app_vars.debug[i]);
    }
    
    memset(&app_vars.debug[0],0,10);
    app_vars.debug[8] = '\r';
    app_vars.debug[9] = '\n';
    
    app_vars.debug_index = 0;
    // time slot starts, use the register here
    app_vars.debug[app_vars.debug_index++] = 'S';
    // change {0-9} to char
    app_vars.debug[app_vars.debug_index++] = 0x30 + app_vars.state;
    
    app_vars.state = APP_STATE_RXDATAOFFSET;
    
    sctimer_scheduleActionIn(ACTION_RADIORX_ENABLE,app_vars.startOfSlotReference+DURATION_rt1);
    radio_rxPacket_prepare();
    // 2. schedule timer for starting 
    sctimer_scheduleActionIn(ACTION_SET_TIMEOUT,  app_vars.startOfSlotReference+DURATION_rt2);
}

void cb_action_timer(void){
    
    // time slot starts, use the register here
    app_vars.debug[app_vars.debug_index++] = 'C';
    // change {0-9} to char
    app_vars.debug[app_vars.debug_index++] = 0x30 + app_vars.state;
    
    debugpins_fsm_toggle();
    
    switch(app_vars.state){
        case APP_STATE_IDLE:
            // error
            leds_error_toggle();
            break;
        case APP_STATE_RXDATAOFFSET:
            app_vars.state = APP_STATE_RXDATAPREPARE;
            break;
        case APP_STATE_RXDATAPREPARE:
            app_vars.state = APP_STATE_RXDATAREADY;
            sctimer_scheduleActionIn(ACTION_SET_TIMEOUT, app_vars.startOfSlotReference+DURATION_rt3);
            app_vars.state = APP_STATE_RXDATALISTEN;
            break;
        case APP_STATE_RXDATAREADY:
            // error
            leds_error_toggle();
            break;
        case APP_STATE_RXDATALISTEN:
            app_vars.state = APP_STATE_IDLE;
            break;
    }
}
