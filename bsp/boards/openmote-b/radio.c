/**
    \brief Definition of the Open Radio interface for openmote-b
    \author Mina Rady <mina1.rady@orange.com>, March 2020.
*/
#include "board.h"

#include "spi.h"
#include "debugpins.h"
#include "leds.h"
#include "sctimer.h"


#include <headers/hw_ioc.h>
#include <headers/hw_memmap.h>
#include <headers/hw_ssi.h>
#include <headers/hw_sys_ctrl.h>
#include <headers/hw_types.h>

#include <source/interrupt.h>
#include <source/ioc.h>
#include <source/gpio.h>
#include <source/sys_ctrl.h>

// Drivers of radios attached to openmote-b
#include "radio_at86rf215.h"
#include "radio_cc2538rf.h"

// The generic radio driver header file
#include "radio.h"

//=========================== defines =========================================


// open radio functions callback types declaration

typedef void                (*radio_reset_cb_t)(void);
typedef void                (*radio_init_cb_t)(void);
typedef void                (*radio_set_modulation_cb_t)(radioSetting_t selected_radio);

typedef void                (*radio_setStartFrameCb_cb_t)(radio_capture_cbt cb);
typedef void                (*radio_setEndFrameCb_cb_t)(radio_capture_cbt cb);
typedef void                (*radio_setFrequency_cb_t)(uint16_t channel, radio_freq_t tx_or_rx);
typedef void                (*radio_rfOn_cb_t)(void);
typedef void                (*radio_rfOff_cb_t)(void);
typedef int8_t              (*radio_getFrequencyOffset_cb_t)(void);
typedef void                (*radio_loadPacket_cb_t)(uint8_t* packet, uint16_t len);
typedef radio_state_t       (*radio_getState_cb_t)(void);
typedef void                (*radio_txEnable_cb_t)(void);
typedef void                (*radio_txNow_cb_t)(void);
typedef void                (*radio_rxEnable_cb_t)(void);
typedef void                (*radio_rxNow_cb_t)(void);
typedef void                (*radio_getReceivedFrame_cb_t)(
                                                    uint8_t* bufRead,
                                                    uint16_t* lenRead,
                                                    uint16_t  maxBufLen,
                                                    int8_t*  rssi,
                                                    uint8_t* lqi,
                                                    bool*    crc
                                                    );
typedef void                (*radio_isr_cb_t)(void);




// the template for radio function callbacks
typedef struct {
    radio_reset_cb_t                    radio_reset;
    radio_init_cb_t                     radio_init;
    radio_set_modulation_cb_t           radio_set_modulation;
    radio_setStartFrameCb_cb_t          radio_setStartFrameCb;
    radio_setEndFrameCb_cb_t            radio_setEndFrameCb;
    radio_setFrequency_cb_t             radio_setFrequency;
    radio_rfOn_cb_t                     radio_rfOn;
    radio_rfOff_cb_t                    radio_rfOff;
    radio_getFrequencyOffset_cb_t       radio_getFrequencyOffset;
    radio_loadPacket_cb_t               radio_loadPacket;
    radio_txEnable_cb_t                 radio_txEnable;
    radio_txNow_cb_t                    radio_txNow;
    radio_rxEnable_cb_t                 radio_rxEnable;
    radio_rxNow_cb_t                    radio_rxNow;
    radio_getReceivedFrame_cb_t         radio_getReceivedFrame;
    radio_isr_cb_t                      radio_isr;
} radio_functions_t;


// global radio selection, will use the slowest by default at initialization. 
uint8_t SELECTED_RADIO      =       FSK_OPTION1_FEC;

//=========================== variables =======================================

//function call back matrix
radio_functions_t dyn_funcs [MAX_RADIOS];

// ================ Bootstrapping ==========

// initializing the lookup table for radio function callbacks
void radio_bootstrap (void)
{   
    // FSK_OPTION1_FEC
    dyn_funcs [FSK_OPTION1_FEC].radio_reset                 =   radio_reset_at86rf215; 
    dyn_funcs [FSK_OPTION1_FEC].radio_init                  =   radio_init_at86rf215; 
    dyn_funcs [FSK_OPTION1_FEC].radio_set_modulation        =   radio_set_modulation_at86rf215; 
    dyn_funcs [FSK_OPTION1_FEC].radio_setStartFrameCb       =   radio_setStartFrameCb_at86rf215; 
    dyn_funcs [FSK_OPTION1_FEC].radio_setEndFrameCb         =   radio_setEndFrameCb_at86rf215; 
    dyn_funcs [FSK_OPTION1_FEC].radio_setFrequency          =   radio_setFrequency_at86rf215; 
    dyn_funcs [FSK_OPTION1_FEC].radio_rfOn                  =   radio_rfOn_at86rf215; 
    dyn_funcs [FSK_OPTION1_FEC].radio_rfOff                 =   radio_rfOff_at86rf215; 
    dyn_funcs [FSK_OPTION1_FEC].radio_getFrequencyOffset    =   radio_getFrequencyOffset_at86rf215; 
    dyn_funcs [FSK_OPTION1_FEC].radio_loadPacket            =   radio_loadPacket_at86rf215; 
    dyn_funcs [FSK_OPTION1_FEC].radio_txEnable              =   radio_txEnable_at86rf215; 
    dyn_funcs [FSK_OPTION1_FEC].radio_txNow                 =   radio_txNow_at86rf215; 
    dyn_funcs [FSK_OPTION1_FEC].radio_rxEnable              =   radio_rxEnable_at86rf215; 
    dyn_funcs [FSK_OPTION1_FEC].radio_rxNow                 =   radio_rxNow_at86rf215; 
    dyn_funcs [FSK_OPTION1_FEC].radio_getReceivedFrame      =   radio_getReceivedFrame_at86rf215; 
    dyn_funcs [FSK_OPTION1_FEC].radio_isr                   =   radio_isr_at86rf215; 

  
 //OQPSK_RATE3
    dyn_funcs [OQPSK_RATE3].radio_reset                 =   radio_reset_at86rf215; 
    dyn_funcs [OQPSK_RATE3].radio_init                  =   radio_init_at86rf215; 
    dyn_funcs [OQPSK_RATE3].radio_set_modulation     =   radio_set_modulation_at86rf215; 
    dyn_funcs [OQPSK_RATE3].radio_setStartFrameCb       =   radio_setStartFrameCb_at86rf215; 
    dyn_funcs [OQPSK_RATE3].radio_setEndFrameCb         =   radio_setEndFrameCb_at86rf215; 
    dyn_funcs [OQPSK_RATE3].radio_setFrequency          =   radio_setFrequency_at86rf215; 
    dyn_funcs [OQPSK_RATE3].radio_rfOn                  =   radio_rfOn_at86rf215; 
    dyn_funcs [OQPSK_RATE3].radio_rfOff                 =   radio_rfOff_at86rf215; 
    dyn_funcs [OQPSK_RATE3].radio_getFrequencyOffset    =   radio_getFrequencyOffset_at86rf215; 
    dyn_funcs [OQPSK_RATE3].radio_loadPacket            =   radio_loadPacket_at86rf215; 
    dyn_funcs [OQPSK_RATE3].radio_txEnable              =   radio_txEnable_at86rf215; 
    dyn_funcs [OQPSK_RATE3].radio_txNow                 =   radio_txNow_at86rf215; 
    dyn_funcs [OQPSK_RATE3].radio_rxEnable              =   radio_rxEnable_at86rf215; 
    dyn_funcs [OQPSK_RATE3].radio_rxNow                 =   radio_rxNow_at86rf215; 
    dyn_funcs [OQPSK_RATE3].radio_getReceivedFrame      =   radio_getReceivedFrame_at86rf215;
    dyn_funcs [OQPSK_RATE3].radio_isr                   =   radio_isr_at86rf215; 
 
    //OFDM_OPTION_1_MCS0
    dyn_funcs [OFDM_OPTION_1_MCS0].radio_reset                 =   radio_reset_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS0].radio_init                  =   radio_init_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS0].radio_set_modulation         =   radio_set_modulation_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS0].radio_setStartFrameCb       =   radio_setStartFrameCb_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS0].radio_setEndFrameCb         =   radio_setEndFrameCb_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS0].radio_setFrequency          =   radio_setFrequency_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS0].radio_rfOn                  =   radio_rfOn_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS0].radio_rfOff                 =   radio_rfOff_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS0].radio_getFrequencyOffset    =   radio_getFrequencyOffset_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS0].radio_loadPacket            =   radio_loadPacket_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS0].radio_txEnable              =   radio_txEnable_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS0].radio_txNow                 =   radio_txNow_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS0].radio_rxEnable              =   radio_rxEnable_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS0].radio_rxNow                 =   radio_rxNow_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS0].radio_getReceivedFrame      =   radio_getReceivedFrame_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS0].radio_isr                   =   radio_isr_at86rf215; 
 
 //OFDM_OPTION_1_MCS1
    dyn_funcs [OFDM_OPTION_1_MCS1].radio_reset                 =   radio_reset_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS1].radio_init                  =   radio_init_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS1].radio_set_modulation     =   radio_set_modulation_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS1].radio_setStartFrameCb       =   radio_setStartFrameCb_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS1].radio_setEndFrameCb         =   radio_setEndFrameCb_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS1].radio_setFrequency          =   radio_setFrequency_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS1].radio_rfOn                  =   radio_rfOn_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS1].radio_rfOff                 =   radio_rfOff_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS1].radio_getFrequencyOffset    =   radio_getFrequencyOffset_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS1].radio_loadPacket            =   radio_loadPacket_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS1].radio_txEnable              =   radio_txEnable_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS1].radio_txNow                 =   radio_txNow_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS1].radio_rxEnable              =   radio_rxEnable_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS1].radio_rxNow                 =   radio_rxNow_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS1].radio_getReceivedFrame      =   radio_getReceivedFrame_at86rf215;
    dyn_funcs [OFDM_OPTION_1_MCS1].radio_isr                   =   radio_isr_at86rf215; 
  
 //OFDM_OPTION_1_MCS2
    dyn_funcs [OFDM_OPTION_1_MCS2].radio_reset                 =   radio_reset_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS2].radio_init                  =   radio_init_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS2].radio_set_modulation     =   radio_set_modulation_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS2].radio_setStartFrameCb       =   radio_setStartFrameCb_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS2].radio_setEndFrameCb         =   radio_setEndFrameCb_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS2].radio_setFrequency          =   radio_setFrequency_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS2].radio_rfOn                  =   radio_rfOn_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS2].radio_rfOff                 =   radio_rfOff_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS2].radio_getFrequencyOffset    =   radio_getFrequencyOffset_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS2].radio_loadPacket            =   radio_loadPacket_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS2].radio_txEnable              =   radio_txEnable_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS2].radio_txNow                 =   radio_txNow_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS2].radio_rxEnable              =   radio_rxEnable_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS2].radio_rxNow                 =   radio_rxNow_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS2].radio_getReceivedFrame      =   radio_getReceivedFrame_at86rf215;
    dyn_funcs [OFDM_OPTION_1_MCS2].radio_isr                   =   radio_isr_at86rf215; 
  
 //OFDM_OPTION_1_MCS3
    dyn_funcs [OFDM_OPTION_1_MCS3].radio_reset                 =   radio_reset_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS3].radio_init                  =   radio_init_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS3].radio_set_modulation     =   radio_set_modulation_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS3].radio_setStartFrameCb       =   radio_setStartFrameCb_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS3].radio_setEndFrameCb         =   radio_setEndFrameCb_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS3].radio_setFrequency          =   radio_setFrequency_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS3].radio_rfOn                  =   radio_rfOn_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS3].radio_rfOff                 =   radio_rfOff_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS3].radio_getFrequencyOffset    =   radio_getFrequencyOffset_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS3].radio_loadPacket            =   radio_loadPacket_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS3].radio_txEnable              =   radio_txEnable_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS3].radio_txNow                 =   radio_txNow_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS3].radio_rxEnable              =   radio_rxEnable_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS3].radio_rxNow                 =   radio_rxNow_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS3].radio_getReceivedFrame      =   radio_getReceivedFrame_at86rf215;
    dyn_funcs [OFDM_OPTION_1_MCS3].radio_isr                   =   radio_isr_at86rf215; 
 
 
 //CC2538RF_24GHZ
    dyn_funcs [CC2538RF_24GHZ].radio_reset                 =   radio_reset_cc2538rf; 
    dyn_funcs [CC2538RF_24GHZ].radio_init                  =   radio_init_cc2538rf; 
    dyn_funcs [CC2538RF_24GHZ].radio_set_modulation     =   radio_set_modulation_cc2538rf; 
    dyn_funcs [CC2538RF_24GHZ].radio_setStartFrameCb       =   radio_setStartFrameCb_cc2538rf; 
    dyn_funcs [CC2538RF_24GHZ].radio_setEndFrameCb         =   radio_setEndFrameCb_cc2538rf; 
    dyn_funcs [CC2538RF_24GHZ].radio_setFrequency          =   radio_setFrequency_cc2538rf; 
    dyn_funcs [CC2538RF_24GHZ].radio_rfOn                  =   radio_rfOn_cc2538rf; 
    dyn_funcs [CC2538RF_24GHZ].radio_rfOff                 =   radio_rfOff_cc2538rf; 
    dyn_funcs [CC2538RF_24GHZ].radio_getFrequencyOffset    =   radio_getFrequencyOffset_cc2538rf; 
    dyn_funcs [CC2538RF_24GHZ].radio_loadPacket            =   radio_loadPacket_cc2538rf; 
    dyn_funcs [CC2538RF_24GHZ].radio_txEnable              =   radio_txEnable_cc2538rf; 
    dyn_funcs [CC2538RF_24GHZ].radio_txNow                 =   radio_txNow_cc2538rf; 
    dyn_funcs [CC2538RF_24GHZ].radio_rxEnable              =   radio_rxEnable_cc2538rf; 
    dyn_funcs [CC2538RF_24GHZ].radio_rxNow                 =   radio_rxNow_cc2538rf; 
    dyn_funcs [CC2538RF_24GHZ].radio_getReceivedFrame      =   radio_getReceivedFrame_cc2538rf;
    dyn_funcs [CC2538RF_24GHZ].radio_isr                   =   radio_isr_cc2538rf; 
}


//=========================== public ==========================================

//===== admin
void radio_init (void) {
    //bootstrapping the radio look-up matrix
    radio_bootstrap();
    
    // Initializing the atmel radio
    dyn_funcs [FSK_OPTION1_FEC].radio_init();
    
    // Initializing the ti radio
    dyn_funcs [CC2538RF_24GHZ].radio_init();
}

void radio_select (radioSetting_t radio){
    SELECTED_RADIO = radio;
}

void radio_reset(void) {
    dyn_funcs [SELECTED_RADIO].radio_reset();
}


void radio_set_modulation (radioSetting_t selected_radio){
    SELECTED_RADIO = selected_radio;
    dyn_funcs [SELECTED_RADIO].radio_set_modulation(selected_radio);
}


void radio_setStartFrameCb (radio_capture_cbt cb) {
    dyn_funcs [SELECTED_RADIO].radio_setStartFrameCb(cb);
}

void radio_setEndFrameCb (radio_capture_cbt cb) {
    dyn_funcs [SELECTED_RADIO].radio_setEndFrameCb(cb);
}

//===== RF admin

void radio_setFrequency (uint16_t channel, radio_freq_t tx_or_rx) {
    dyn_funcs [SELECTED_RADIO].radio_setFrequency(channel, tx_or_rx);
}

void radio_rfOn (void) {
    dyn_funcs [SELECTED_RADIO].radio_rfOn();
}

void radio_rfOff (void) {
    dyn_funcs [SELECTED_RADIO].radio_rfOff();
}

int8_t radio_getFrequencyOffset (void){
    return dyn_funcs [SELECTED_RADIO].radio_getFrequencyOffset();
}

//===== TX

void radio_loadPacket (uint8_t* packet, uint16_t len) {
    dyn_funcs [SELECTED_RADIO].radio_loadPacket(packet, len);
}


void radio_txEnable (void) {
    dyn_funcs [SELECTED_RADIO].radio_txEnable();
}

void radio_txNow (void) {
    dyn_funcs [SELECTED_RADIO].radio_txNow();
}

//===== RX

void radio_rxEnable (void) {
    dyn_funcs [SELECTED_RADIO].radio_rxEnable();
}

void radio_rxNow (void) {
    dyn_funcs [SELECTED_RADIO].radio_rxNow();
}

void radio_getReceivedFrame (
    uint8_t* bufRead,
    uint16_t* lenRead,
    uint16_t  maxBufLen,
    int8_t*  rssi,
    uint8_t* lqi,
    bool*    crc
) {

    dyn_funcs [SELECTED_RADIO].radio_getReceivedFrame (bufRead, lenRead, maxBufLen, rssi, lqi, crc );
}

//=========================== private =========================================

//=========================== callbacks =======================================

//=========================== interrupt handlers ==============================
void radio_isr(void) {
    dyn_funcs [SELECTED_RADIO].radio_isr();
}


