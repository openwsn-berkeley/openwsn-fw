/**
\brief SCuM-specific definition of the "radio" bsp module.

\author Tengfei Chang <tengfei.chang@inria.fr>, August 2016.
*/


#include "board.h"
#include "radio.h"
#include "radiotimer.h"
#include "debugpins.h"
#include "leds.h"
#include "memory_map.h"

//=========================== defines =========================================

#define LENGTH_MAX_RX_BUFFER  130 //// 1B spi address, 1B length, 125B data, 2B CRC, 1B LQI

//=========================== variables =======================================

typedef struct {
    radiotimer_capture_cbt    startFrame_cb;
    radiotimer_capture_cbt    endFrame_cb;
    uint8_t                   radio_rx_buffer[LENGTH_MAX_RX_BUFFER];
    radio_state_t             state; 
} radio_vars_t;

radio_vars_t radio_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

//===== admin

void radio_init() {

    // clear variables
    memset(&radio_vars,0,sizeof(radio_vars_t));
    
    // change state
    radio_vars.state                = RADIOSTATE_STOPPED;
    
    // Enable all interrupts and pulses to radio timer: no loaddone interrupt
    RFCONTROLLER_REG__INT_CONFIG    = 0x3FE;
    // Enable all errors
    RFCONTROLLER_REG__ERROR_CONFIG  = 0x1F;
    
    // change state
    radio_vars.state                = RADIOSTATE_RFOFF;
}

void radio_setOverflowCb(radiotimer_compare_cbt cb) {
    radiotimer_setOverflowCb(cb);
}

void radio_setCompareCb(radiotimer_compare_cbt cb) {
    radiotimer_setCompareCb(cb);
}

void radio_setStartFrameCb(radiotimer_capture_cbt cb) {
    radio_vars.startFrame_cb  = cb;
}

void radio_setEndFrameCb(radiotimer_capture_cbt cb) {
    radio_vars.endFrame_cb    = cb;
}

//===== reset

void radio_reset() {
    // reset SCuM radio module
    PORT_PIN_RADIO_RESET_LOW();
}

//===== timer

void radio_startTimer(PORT_TIMER_WIDTH period) {
   radiotimer_start(period);
}

PORT_TIMER_WIDTH radio_getTimerValue() {
   return radiotimer_getValue();
}

void radio_setTimerPeriod(PORT_TIMER_WIDTH period) {
   radiotimer_setPeriod(period);
}

PORT_TIMER_WIDTH radio_getTimerPeriod() {
   return radiotimer_getPeriod();
}

//===== RF admin

void radio_setFrequency(uint8_t frequency) {
    // change state
    radio_vars.state = RADIOSTATE_SETTING_FREQUENCY;
    
    // not support by SCuM yet
    
    
    // change state
    radio_vars.state = RADIOSTATE_FREQUENCY_SET;
}

void radio_rfOn() {
    // clear reset pin
    RFCONTROLLER_REG__CONTROL &= ~0x10;
}

void radio_rfOff() {
    // change state
    radio_vars.state = RADIOSTATE_TURNING_OFF;

    // turn SCuM radio off
    RFCONTROLLER_REG__CONTROL = 0x08;

    // wiggle debug pin
    debugpins_radio_clr();
    leds_radio_off();

    // change state
    radio_vars.state = RADIOSTATE_RFOFF;
}

//===== TX

void radio_loadPacket(uint8_t* packet, uint8_t len) {
    uint8_t i;
    // change state
    radio_vars.state = RADIOSTATE_LOADING_PACKET;

    // load packet in TXFIFO
    RFCONTROLLER_REG__TX_DATA_ADDR  = &(packet[0]);
    RFCONTROLLER_REG__TX_PACK_LEN   = len;
    RFCONTROLLER_REG__CONTROL       = 0x01;
    
    // add some delay for loading
    for (i=0;i<0xff;i++);
    
    radio_vars.state = RADIOSTATE_PACKET_LOADED;
}

void radio_txEnable() {
    // change state
    radio_vars.state = RADIOSTATE_ENABLING_TX;

    // not support by SCuM
    
    // wiggle debug pin
    debugpins_radio_set();
    leds_radio_on();

    // change state
    radio_vars.state = RADIOSTATE_TX_ENABLED;
}

void radio_txNow() {

    // change state
    RFCONTROLLER_REG__CONTROL = 0x02;
}

//===== RX

void radio_rxEnable() {
    
    // change state
    radio_vars.state            = RADIOSTATE_ENABLING_RX;
    
    DMA_REG__RF_RX_ADDR         = &(radio_vars.radio_rx_buffer[0]);
    // start to listen
    RFCONTROLLER_REG__CONTROL   = 0x04;
    
    // wiggle debug pin
    debugpins_radio_set();
    leds_radio_on();
    
    // change state
    radio_vars.state            = RADIOSTATE_LISTENING;
}

void radio_rxNow() {
    // nothing to do
}

void radio_getReceivedFrame(uint8_t* pBufRead,
                            uint8_t* pLenRead,
                            uint8_t  maxBufLen,
                             int8_t* pRssi,
                            uint8_t* pLqi,
                               bool* pCrc) {
    
    //===== crc
    *pCrc           = 1;
   
    //===== rssi
    *pRssi          = -91;
    
    //===== length
    *pLenRead = radio_vars.radio_rx_buffer[0];
    
    //===== packet 
    memcpy(pBufRead,&(radio_vars.radio_rx_buffer[1]),*pLenRead);
}

//=========================== private =========================================

//=========================== callbacks =======================================

//=========================== interrupt handlers ==============================

kick_scheduler_t radio_isr() {
    
    PORT_TIMER_WIDTH capturedTime;
    PORT_TIMER_WIDTH irq_status = RFCONTROLLER_REG__INT;
    PORT_TIMER_WIDTH irq_error  = RFCONTROLLER_REG__ERROR;
    
    capturedTime                = radiotimer_getCapturedTime();
    
    if (irq_status & 0x00000002 || irq_status & 0x00000008){
        // change state
        if (irq_status & 0x00000002) {
            radio_vars.state    = RADIOSTATE_TRANSMITTING;
        }
        if (irq_status & 0x00000008) {
            radio_vars.state    = RADIOSTATE_RECEIVING;
        }
        if (radio_vars.startFrame_cb!=NULL) {
            // call the callback
            radio_vars.startFrame_cb(capturedTime);
            // clear interruption bit
            RFCONTROLLER_REG__INT_CLEAR = irq_status;
            // kick the OS
            return KICK_SCHEDULER;
        }
    }
    
    if (irq_status & 0x00000004 || irq_status & 0x00000010){
        // change state
        radio_vars.state = RADIOSTATE_TXRX_DONE;
        if (radio_vars.endFrame_cb!=NULL) {
            // call the callback
            radio_vars.endFrame_cb(capturedTime);
            // clear interruption bit
            RFCONTROLLER_REG__INT_CLEAR = irq_status;
            // kick the OS
            return KICK_SCHEDULER;
        } else {
            while(1);
        }
    }
    
    if (irq_error == 0) {
        
        // print out the error here. 
        
        RFCONTROLLER_REG__ERROR_CLEAR = irq_error;
    }
    
    return DO_NOT_KICK_SCHEDULER;
}
