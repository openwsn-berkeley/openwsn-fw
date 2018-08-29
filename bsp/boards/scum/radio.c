/**
\brief SCuM-specific definition of the "radio" bsp module.

\author Tengfei Chang <tengfei.chang@inria.fr>, August 2016.
*/


#include "board.h"
#include "radio.h"
#include "sctimer.h"
#include "debugpins.h"
#include "leds.h"
#include "memory_map.h"

//=========================== defines =========================================

#define MAXLENGTH_TRX_BUFFER    128 //// 1B length, 125B data, 2B CRC

// ==== default crc check result and rssi value

#define DEFAULT_CRC_CHECK       0x01    // this is an arbitrary value for now
#define DEFAULT_RSSI            -50     // this is an arbitrary value for now

//=========================== variables =======================================

typedef struct {
    radio_capture_cbt         startFrame_cb;
    radio_capture_cbt         endFrame_cb;
    uint8_t                   radio_tx_buffer[MAXLENGTH_TRX_BUFFER] __attribute__ ((aligned (4)));
    uint8_t                   radio_rx_buffer[MAXLENGTH_TRX_BUFFER] __attribute__ ((aligned (4)));
    radio_state_t             state; 
} radio_vars_t;

radio_vars_t radio_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

//===== admin

void radio_init(void) {

    // clear variables
    memset(&radio_vars,0,sizeof(radio_vars_t));
    
    // change state
    radio_vars.state                = RADIOSTATE_STOPPED;
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
    // enable sfd done and send done interruptions of tranmission
    // enable sfd done and receiving done interruptions of reception
    RFCONTROLLER_REG__INT_CONFIG    = TX_LOAD_DONE_INT_EN           |   \
                                      TX_SFD_DONE_INT_EN            |   \
                                      TX_SEND_DONE_INT_EN           |   \
                                      RX_SFD_DONE_INT_EN            |   \
                                      RX_DONE_INT_EN                |   \
                                      TX_SFD_DONE_RFTIMER_PULSE_EN  |   \
                                      TX_SEND_DONE_RFTIMER_PULSE_EN |   \
                                      RX_SFD_DONE_RFTIMER_PULSE_EN  |   \
                                      RX_DONE_RFTIMER_PULSE_EN;
#else
    // enable sfd done and send done interruptions of tranmission
    RFCONTROLLER_REG__INT_CONFIG    = TX_SFD_DONE_INT_EN            |   \
                                      TX_SEND_DONE_INT_EN           |   \
                                      RX_SFD_DONE_INT_EN            |   \
                                      RX_DONE_INT_EN;
#endif
    // Enable all errors
    RFCONTROLLER_REG__ERROR_CONFIG  = TX_OVERFLOW_ERROR_EN          |   \
                                      TX_CUTOFF_ERROR_EN            |   \
                                      RX_OVERFLOW_ERROR_EN          |   \
                                      RX_CRC_ERROR_EN               |   \
                                      RX_CUTOFF_ERROR_EN;
    
    // change state
    radio_vars.state                = RADIOSTATE_RFOFF;
}

void radio_setStartFrameCb(radio_capture_cbt cb) {
    radio_vars.startFrame_cb    = cb;
}

void radio_setEndFrameCb(radio_capture_cbt cb) {
    radio_vars.endFrame_cb      = cb;
}

//===== reset

void radio_reset(void) {
    // reset SCuM radio module
    PORT_PIN_RADIO_RESET_LOW();
}

//===== RF admin

void radio_setFrequency(uint8_t frequency) {
    // change state
    radio_vars.state = RADIOSTATE_SETTING_FREQUENCY;
    
    // not support by SCuM yet
    
    
    // change state
    radio_vars.state = RADIOSTATE_FREQUENCY_SET;
}

void radio_rfOn(void) {
    // clear reset pin
    RFCONTROLLER_REG__CONTROL   &= ~RX_RESET;
}

void radio_rfOff(void) {
    // change state
    radio_vars.state            = RADIOSTATE_TURNING_OFF;

    // turn SCuM radio off
    RFCONTROLLER_REG__CONTROL   = RX_STOP;
    
    // wiggle debug pin
    debugpins_radio_clr();
    leds_radio_off();

    // change state
    radio_vars.state            = RADIOSTATE_RFOFF;
}

//===== TX

#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
void radio_loadPacket_prepare(uint8_t* packet, uint16_t len){
    
    radio_vars.state = RADIOSTATE_LOADING_PACKET;
    
    memcpy(&radio_vars.radio_tx_buffer[0],packet,len);
    
    RFCONTROLLER_REG__TX_DATA_ADDR  = &(radio_vars.radio_tx_buffer[0]);
    RFCONTROLLER_REG__TX_PACK_LEN   = len;
    
    // will be loaded when load timer fired, change the state in advance
    radio_vars.state = RADIOSTATE_PACKET_LOADED;
}
#endif

void radio_loadPacket(uint8_t* packet, uint16_t len) {
    uint8_t i;
    // change state
    radio_vars.state = RADIOSTATE_LOADING_PACKET;
    
    memcpy(&radio_vars.radio_tx_buffer[0],packet,len);

    // load packet in TXFIFO
    RFCONTROLLER_REG__TX_DATA_ADDR  = &(radio_vars.radio_tx_buffer[0]);
    RFCONTROLLER_REG__TX_PACK_LEN   = len;

    RFCONTROLLER_REG__CONTROL       = TX_LOAD;

    // add some delay for loading
    for (i=0;i<0xff;i++);
    
    radio_vars.state = RADIOSTATE_PACKET_LOADED;

}

void radio_txEnable(void) {
    // change state
    radio_vars.state = RADIOSTATE_ENABLING_TX;

    // not support by SCuM
    
    // wiggle debug pin
    debugpins_radio_set();
    leds_radio_on();

    // change state
    radio_vars.state = RADIOSTATE_TX_ENABLED;
}

void radio_txNow(void) {

    // change state
    RFCONTROLLER_REG__CONTROL = TX_SEND;
}

//===== RX

void radio_rxPacket_prepare(void){
    DMA_REG__RF_RX_ADDR         = &(radio_vars.radio_rx_buffer[0]);
}

void radio_rxEnable(void) {
    
    // change state
    radio_vars.state            = RADIOSTATE_ENABLING_RX;
    DMA_REG__RF_RX_ADDR         = &(radio_vars.radio_rx_buffer[0]);
    // start to listen
    RFCONTROLLER_REG__CONTROL   = RX_START;
    // wiggle debug pin
    debugpins_radio_set();
    leds_radio_on();
    
    // change state
    radio_vars.state            = RADIOSTATE_LISTENING;

}

void radio_rxEnable_scum(void){
    // change state
    radio_vars.state            = RADIOSTATE_ENABLING_RX;
    
    // wiggle debug pin
    debugpins_radio_set();
    leds_radio_on();
    
    // change state
    radio_vars.state            = RADIOSTATE_LISTENING;
}

void radio_rxNow(void) {
    // nothing to do
}

void radio_getReceivedFrame(uint8_t* pBufRead,
                            uint8_t* pLenRead,
                            uint8_t  maxBufLen,
                             int8_t* pRssi,
                            uint8_t* pLqi,
                               bool* pCrc) {
    
    //===== crc
    *pCrc           = DEFAULT_CRC_CHECK;
   
    //===== rssi
    *pRssi          = DEFAULT_RSSI;
    
    //===== length
    *pLenRead       = radio_vars.radio_rx_buffer[0];
    
    //===== packet 
    memcpy(pBufRead,&(radio_vars.radio_rx_buffer[1]),*pLenRead);
}

//=========================== private =========================================

//=========================== callbacks =======================================

//=========================== interrupt handlers ==============================

kick_scheduler_t radio_isr(void) {
    
    PORT_TIMER_WIDTH capturedTime;
    
    PORT_TIMER_WIDTH irq_status = RFCONTROLLER_REG__INT;
    PORT_TIMER_WIDTH irq_error  = RFCONTROLLER_REG__ERROR;
    
    debugpins_isr_set();
    
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
#else
    capturedTime                = sctimer_readCounter();
#endif
    if (irq_status & TX_SFD_DONE_INT || irq_status & RX_SFD_DONE_INT){
        // SFD is just sent or received, check the specific interruption and 
        // change the radio state accordingly
        if (irq_status & TX_SFD_DONE_INT) {
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
            // get the capture Time from capture register
            capturedTime = TIMER_COUNTER_CONVERT_RFTIMER_CLK_TO_32K(RFTIMER_REG__CAPTURE0);
#endif
            RFCONTROLLER_REG__INT_CLEAR = TX_SFD_DONE_INT;
            // a SFD is just sent, update radio state
            radio_vars.state    = RADIOSTATE_TRANSMITTING;
        }
        if (irq_status & RX_SFD_DONE_INT) {
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
            // get the capture Time from capture register
            capturedTime = TIMER_COUNTER_CONVERT_RFTIMER_CLK_TO_32K(RFTIMER_REG__CAPTURE1);
#endif
            RFCONTROLLER_REG__INT_CLEAR = RX_SFD_DONE_INT;
            // a SFD is just received, update radio state
            radio_vars.state    = RADIOSTATE_RECEIVING;
        }
        if (radio_vars.startFrame_cb!=NULL) {
            // call the callback
            radio_vars.startFrame_cb(capturedTime);
            debugpins_isr_clr();
            // kick the OS
            return KICK_SCHEDULER;
        }
    }
    
    if (irq_status & TX_SEND_DONE_INT || irq_status & RX_DONE_INT){
        if (irq_status & TX_SEND_DONE_INT) {
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
            // get the capture Time from capture register
            capturedTime = TIMER_COUNTER_CONVERT_RFTIMER_CLK_TO_32K(RFTIMER_REG__CAPTURE2);
#endif
            RFCONTROLLER_REG__INT_CLEAR = TX_SEND_DONE_INT;
        }
        if (irq_status & RX_DONE_INT) {
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
            // get the capture Time from capture register
            capturedTime = TIMER_COUNTER_CONVERT_RFTIMER_CLK_TO_32K(RFTIMER_REG__CAPTURE3);
#endif
            RFCONTROLLER_REG__INT_CLEAR = RX_DONE_INT;
        }
        // the packet transmission or reception is done,
        // update the radio state
        radio_vars.state        = RADIOSTATE_TXRX_DONE;
        if (radio_vars.endFrame_cb!=NULL) {
            // call the callback
            radio_vars.endFrame_cb(capturedTime);
            debugpins_isr_clr();
            // kick the OS
            return KICK_SCHEDULER;
        } else {
            while(1);
        }
    }
    
    if (irq_status & TX_LOAD_DONE_INT){
        RFCONTROLLER_REG__INT_CLEAR = TX_LOAD_DONE_INT;
    }
    
    if (irq_error == 0) {
        // error happens during the operation of radio. Print out the error here. 
        // To Be Done. add error description deifinition for this type of errors.
        RFCONTROLLER_REG__ERROR_CLEAR = irq_error;
    }
    debugpins_isr_clr();
    return DO_NOT_KICK_SCHEDULER;
}
