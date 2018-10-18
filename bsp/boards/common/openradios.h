#ifndef __OPENRADIOS_H
#define __OPENRADIOS_H

/**
\addtogroup BSP
\{
\addtogroup openradios
\{

\brief Cross-platform declaration "openradios" driver module.

\author Tengfei Chang <tengfei.chang@inria.fr>, March 2018.
*/

#include "stdint.h"
#include "board.h"

//=========================== define ==========================================

//=========================== typedef =========================================

/**
\brief Current state of the radio.

\note This radio driver is very minimal in that it does not follow a state machine.
      It is up to the MAC layer to ensure that the different radio operations
      are called in the righr order. The radio keeps a state for debugging purposes only.
*/

typedef enum {
   RADIOSTATE_STOPPED             = 0x00,   ///< Completely stopped.
   RADIOSTATE_RFOFF               = 0x01,   ///< Listening for commands, but RF chain is off.
   RADIOSTATE_SETTING_FREQUENCY   = 0x02,   ///< Configuring the frequency.
   RADIOSTATE_FREQUENCY_SET       = 0x03,   ///< Done configuring the frequency.
   RADIOSTATE_LOADING_PACKET      = 0x04,   ///< Loading packet into the radio's TX buffer.
   RADIOSTATE_PACKET_LOADED       = 0x05,   ///< Packet is fully loaded in the radio's TX buffer.
   RADIOSTATE_ENABLING_TX         = 0x06,   ///< The RF TX chaing is being enabled (includes locking the PLL).
   RADIOSTATE_TX_ENABLED          = 0x07,   ///< Radio ready to transmit.
   RADIOSTATE_TRANSMITTING        = 0x08,   ///< Busy transmitting bytes.
   RADIOSTATE_ENABLING_RX         = 0x09,   ///< The RF RX chain is being enabled (includes locking the PLL).
   RADIOSTATE_LISTENING           = 0x0a,   ///< RF chain is on, listening, but no packet received yet.
   RADIOSTATE_RECEIVING           = 0x0b,   ///< Busy receiving bytes.
   RADIOSTATE_TXRX_DONE           = 0x0c,   ///< Frame has been sent/received completely.
   RADIOSTATE_TURNING_OFF         = 0x0d,   ///< Turning the RF chain off.
} radio_state_t;

typedef void (*radio_capture_cbt)(PORT_TIMER_WIDTH timestamp);

//generic register placeholder for some spi radios
typedef struct {
    uint16_t  addr;
    uint8_t   data;
}registerSetting_t;

typedef void                (*radio_powerOn_cbt)(void);
      // RF admin
typedef void                (*radio_init_cbt)(void);
typedef void                (*radio_setStartFrameCb_cbt)(radio_capture_cbt cb);
typedef void                (*radio_setEndFrameCb_cbt)(radio_capture_cbt cb);
    // RF admin
typedef void                (*radio_rfOn_cbt)(void);
typedef void                (*radio_rfOff_cbt)(void);
typedef void                (*radio_setFrequency_cbt)(uint16_t channel_spacing, uint32_t frequency_0, uint16_t channel);
typedef void                (*radio_change_modulation_cbt)(registerSetting_t * mod);
typedef void                (*radio_change_size_cbt)(uint16_t* size);
    // reset
typedef void                (*radio_reset_cbt)(void);
    // TX
typedef void                (*radio_loadPacket_prepare_cbt)(uint8_t* packet, uint8_t len);
typedef void                (*radio_txEnable_cbt)(void);
typedef void                (*radio_txNow_cbt)(void);
typedef void                (*radio_loadPacket_cbt)(uint8_t* packet, uint16_t len);
    // RX
typedef void                (*radio_rxPacket_prepare_cbt)(void);
typedef void                (*radio_rxEnable_cbt)(void);
typedef void                (*radio_rxEnable_scum_cbt)(void);
typedef void                (*radio_rxNow_cbt)(void);
typedef void                (*radio_getReceivedFrame_cbt)(
    uint8_t*    bufRead,
    uint16_t*   lenRead,
    uint16_t    maxBufLen,
    int8_t*     rssi,
    uint8_t*    lqi,
    bool*       crc,
    uint8_t*    mcs
);
typedef uint8_t             (*radio_getCRCLen_cbt)(void);
typedef uint8_t             (*radio_calculateFrequency_cbt)(
    uint8_t channelOffset,
    uint8_t asnOffset,
    uint8_t numChannels,
    uint8_t* hopSeq,
    bool singleChannel
);
typedef uint8_t             (*radio_getDelayTx_cbt)(void);
typedef uint8_t             (*radio_getDelayRx_cbt)(void);
typedef uint8_t             (*radio_getChInitOffset_cbt)(void);

typedef struct {
    radio_powerOn_cbt             radio_powerOn_cb;
          // RF admin
    radio_init_cbt                radio_init_cb;
    radio_setStartFrameCb_cbt     radio_setStartFrameCb_cb;
    radio_setEndFrameCb_cbt       radio_setEndFrameCb_cb;
        // RF admin
    radio_rfOn_cbt                radio_rfOn_cb;
    radio_rfOff_cbt               radio_rfOff_cb;
    radio_setFrequency_cbt        radio_setFrequency_cb;
    radio_change_modulation_cbt   radio_change_modulation_cb;
    radio_change_size_cbt         radio_change_size_cb;
        // reset
    radio_reset_cbt               radio_reset_cb;
        // TX
    radio_loadPacket_prepare_cbt  radio_loadPacket_prepare_cb;
    radio_txEnable_cbt            radio_txEnable_cb;
    radio_txNow_cbt               radio_txNow_cb;
    radio_loadPacket_cbt          radio_loadPacket_cb;
        // RX
    radio_rxPacket_prepare_cbt    radio_rxPacket_prepare_cb;
    radio_rxEnable_cbt            radio_rxEnable_cb;
    radio_rxEnable_scum_cbt       radio_rxEnable_scum_cb;
    radio_rxNow_cbt               radio_rxNow_cb;
    radio_getReceivedFrame_cbt    radio_getReceivedFrame_cb;
    radio_getCRCLen_cbt           radio_getCRCLen_cb;
    radio_calculateFrequency_cbt  radio_calculateFrequency_cb;
    radio_getDelayTx_cbt          radio_getDelayTx_cb;
    radio_getDelayRx_cbt          radio_getDelayRx_cb;
    radio_getChInitOffset_cbt     radio_getChInitOffset_cb;
} radio_functions_t;

typedef struct {
    radio_functions_t radio_funct[MAX_NUM_MODEM];
} openradios_vars_t;

//=========================== variables =======================================

//=========================== prototypes ======================================

// admin
void                openradios_getFunctions(radio_functions_t** radio_funct);

// interrupt handlers, this should be implemented by specific radio
kick_scheduler_t    openradios_isr(void);

/**
\}
\}
*/

#endif
