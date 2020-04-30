#ifndef __RADIO_AT86RF215_H
#define __RADIO_AT86RF215_H

/**
\addtogroup BSP
\{
\addtogroup radio
\{

\brief Cross-platform declaration "radio" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

/*
\modifications to implement the IEEE 802.15.4-SUN
\done by Jonathan Munoz <jonathan.munoz@inria.fr> 
*/

#include "radio.h"
#include "at86rf215.h"

//=========================== define ==========================================

// Number of modulations available on this radio chip. This is different from MAC_RADIOS which is for all the board and defined in board_info.h
#define MAX_MODULATIONS 6

//=========================== typedef =========================================


typedef struct {
    radio_capture_cbt           startFrame_cb;
    radio_capture_cbt           endFrame_cb;
    radio_state_t               state;
    uint8_t                     rf09_isr;
    uint8_t                     rf24_isr;
    uint8_t                     bb0_isr;
    uint8_t                     bb1_isr;
} radio_vars_at86rf215_t;




// open radio register mapping: an array of pointers to registersettings arrays

typedef struct {
    registerSetting_t * radios [MAX_MODULATIONS];
    size_t size [MAX_MODULATIONS];
}   radio_config_t;


//=========================== variables =======================================

//=========================== prototypes ======================================

// admin
void     radio_powerOn_at86rf215(void);
void     radio_init_at86rf215(void);
void     radio_setStartFrameCb_at86rf215(radio_capture_cbt cb);
void     radio_setEndFrameCb_at86rf215(radio_capture_cbt cb);
// reset
void     radio_reset_at86rf215(void);
// RF admin
void     radio_setFrequency_at86rf215(uint16_t channel, radio_freq_t tx_or_rx);
int8_t  radio_getFrequencyOffset_at86rf215(void);
void     radio_rfOn_at86rf215(void);
void     radio_rfOff_at86rf215(void);
void     radio_setConfig_at86rf215 (radioSetting_t radioSetting);
void     radio_change_modulation_at86rf215(registerSetting_t * mod);
void     radio_change_size_at86rf215(uint16_t* size);
// TX
void     radio_loadPacket_at86rf215(uint8_t* packet, uint16_t len);
radio_state_t    radio_getState_at86rf215(void);
void     radio_txEnable_at86rf215(void);
void     radio_txNow_at86rf215(void);
// RX
void     radio_rxEnable_at86rf215(void);
void     radio_rxNow_at86rf215(void);
void     radio_getReceivedFrame_at86rf215(uint8_t* bufRead,
                                uint8_t* lenRead,
                                uint8_t  maxBufLen,
                                 int8_t* rssi,
                                uint8_t* lqi,
                                   bool* crc);

// interrupt handlers
kick_scheduler_t     radio_isr_at86rf215(void);
// private
void     radio_isr_internal_at86rf215(void);
void     radio_local_bootstrap_at86rf215(void);
/**
\}
\}
*/

#endif
