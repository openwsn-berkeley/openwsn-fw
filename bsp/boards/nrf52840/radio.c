/**
 * brief Nordic nRF52840-specific definition of the "radio" bsp module. 
 *
 * Authors: Tamas Harczos (1, tamas.harczos@imms.de) and Adam Sedmak (2, adam.sedmak@gmail.com)
 * Company: (1) Institut fuer Mikroelektronik- und Mechatronik-Systeme gemeinnuetzige GmbH (IMMS GmbH)
 *          (2) Faculty of Electronics and Computing, Zagreb, Croatia
 *    Date: June 2018
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "sdk/components/boards/boards.h"
#include "sdk/components/drivers_nrf/radio_config/radio_config.h"
#include "sdk/modules/nrfx/drivers/include/nrfx_systick.h"
#include "sdk/modules/nrfx/mdk/nrf52840.h"

#include "sdk/integration/nrfx/legacy/nrf_drv_clock.h"

#include "nrf_delay.h"

#include "app_config.h"
#include "leds.h"
#include "radio.h"
#include "board.h"
#include "board_info.h"
#include "debugpins.h"
#include "sctimer.h"
#include "radio_ble.h"


//=========================== defines =========================================

#define RADIO_POWER_POWER_POS       0

#define STATE_DISABLED              0
#define STATE_RXRU                  1
#define STATE_RXIDLE                2
#define STATE_RX                    3
#define STATE_RXDISABLE             4
#define STATE_TXTU                  9
#define STATE_TXIDLE                10
#define STATE_TX                    11
#define STATE_TXDIABLE              12

/* For calculating frequency */
#define FREQUENCY_OFFSET  10
#define FREQUENCY_STEP    5

#define SFD_OCTET                 (0xA7)      ///< start of frame delimiter of IEEE 802.15.4
#define MAX_PACKET_SIZE           (127)       ///< maximal size of radio packet (one more byte at the beginning needed to store the length)
#define CRC_POLYNOMIAL            (0x11021)   ///< polynomial used for CRC calculation in 802.15.4 frames (x^16 + x^12 + x^5 + 1)

#define WAIT_FOR_RADIO_DISABLE    (0)         ///< whether the driver shall wait until the radio is disabled upon calling radio_rfOff()
#define WAIT_FOR_RADIO_ENABLE     (1)         ///< whether the driver shall wait until the radio is enabled upon calling radio_txEnable() or radio_rxEnable()

#define RADIO_CRCINIT_24BIT       0x555555
#define RADIO_CRCPOLY_24BIT       0x0000065B  /// ref: https://devzone.nordicsemi.com/f/nordic-q-a/44111/crc-register-values-for-a-24-bit-crc

#define MAX_PAYLOAD_LENGTH        (127)
#define INTERFRAM_SPACING         (150)       // in us

#define BLE_ACCESS_ADDR           0x8E89BED6  // the actual address is 0xD6, 0xBE, 0x89, 0x8E

//=========================== variables =======================================

typedef struct {
    radio_capture_cbt   startFrame_cb;
    radio_capture_cbt   endFrame_cb;
    radio_state_t       state; 
    uint8_t             payload[1+MAX_PACKET_SIZE] __attribute__ ((aligned));
    bool                hfc_started;
//  volatile bool event_ready;
} radio_vars_t;

static radio_vars_t radio_vars;

//=========================== prototypes ======================================

static uint32_t swap_bits(uint32_t inp);
static uint32_t bytewise_bitswap(uint32_t inp);
static uint8_t  ble_channel_to_frequency(channel);

//=========================== public ==========================================


void radio_init(void) {

    // clear internal variables
    memset(&radio_vars, 0, sizeof(radio_vars));

    // set radio configuration parameters
    NRF_RADIO->TXPOWER   = (RADIO_TXPOWER_TXPOWER_Pos4dBm << RADIO_TXPOWER_TXPOWER_Pos);

    // set radio mode to IEEE 802.15.4
    NRF_RADIO->MODE      = (RADIO_MODE_MODE_Ieee802154_250Kbit << RADIO_MODE_MODE_Pos);

    // set config field length to 8
    NRF_RADIO->PCNF0 &= (~RADIO_PCNF0_LFLEN_Msk);
    NRF_RADIO->PCNF0 |= (((uint32_t)8) << RADIO_PCNF0_LFLEN_Pos);

    // set 32-bit zero preamble
    NRF_RADIO->PCNF0 &= (~RADIO_PCNF0_PLEN_Msk);
    NRF_RADIO->PCNF0 |= ((uint32_t) RADIO_PCNF0_PLEN_32bitZero << RADIO_PCNF0_PLEN_Pos);

    // set max packet size
    NRF_RADIO->PCNF1 &= (~RADIO_PCNF1_MAXLEN_Msk);
    NRF_RADIO->PCNF1 |= ((uint32_t) MAX_PACKET_SIZE << RADIO_PCNF1_MAXLEN_Pos);

    // set start of frame delimiter
    NRF_RADIO->SFD = (SFD_OCTET << RADIO_SFD_SFD_Pos) & RADIO_SFD_SFD_Msk;

    // set CRC to be included
    NRF_RADIO->PCNF0 &= (~RADIO_PCNF0_CRCINC_Msk);
    NRF_RADIO->PCNF0 |= ((uint32_t) RADIO_PCNF0_CRCINC_Include << RADIO_PCNF0_CRCINC_Pos);

    // set CRC length
    NRF_RADIO->CRCCNF &= (~RADIO_CRCCNF_LEN_Msk);
    NRF_RADIO->CRCCNF |= ((uint32_t) LENGTH_CRC << RADIO_CRCCNF_LEN_Pos);

    // configure CRC (CRC calculation as per 802.15.4 standard. Starting at first byte after length field.)
    NRF_RADIO->CRCCNF &= (~RADIO_CRCCNF_SKIPADDR_Msk);
    NRF_RADIO->CRCCNF |= ((uint32_t) RADIO_CRCCNF_SKIPADDR_Ieee802154 << RADIO_CRCCNF_SKIPADDR_Pos);

    // set CRC polynomial
    NRF_RADIO->CRCPOLY = (CRC_POLYNOMIAL << RADIO_CRCPOLY_CRCPOLY_Pos);
    NRF_RADIO->CRCINIT = 0x0UL;

    // set payload pointer
    NRF_RADIO->PACKETPTR = (uint32_t)(radio_vars.payload);

    // set up interrupts
    // disable radio interrupt
    NVIC_DisableIRQ(RADIO_IRQn);
    NRF_RADIO->INTENSET = // RADIO_INTENSET_READY_Enabled << RADIO_INTENSET_READY_Pos | 
                        RADIO_INTENSET_ADDRESS_Enabled << RADIO_INTENSET_ADDRESS_Pos |
                        RADIO_INTENSET_END_Enabled << RADIO_INTENSET_END_Pos; 
    NVIC_SetPriority(RADIO_IRQn, NRFX_RADIO_CONFIG_IRQ_PRIORITY);

    NVIC_ClearPendingIRQ(RADIO_IRQn);
    NVIC_EnableIRQ(RADIO_IRQn);
}

void radio_ble_init(void){
    
    NRF_RADIO->PCNF0 =   (((1UL) << RADIO_PCNF0_S0LEN_Pos) & RADIO_PCNF0_S0LEN_Msk) | 
                         (((0UL) << RADIO_PCNF0_S1LEN_Pos) & RADIO_PCNF0_S1LEN_Msk) |
                         (((8UL) << RADIO_PCNF0_LFLEN_Pos) & RADIO_PCNF0_LFLEN_Msk);

    NRF_RADIO->PCNF1 =   (((RADIO_PCNF1_ENDIAN_Little)    << RADIO_PCNF1_ENDIAN_Pos)  & RADIO_PCNF1_ENDIAN_Msk)  |
                         (((3UL)                          << RADIO_PCNF1_BALEN_Pos)   & RADIO_PCNF1_BALEN_Msk)   |
                         (((0UL)                          << RADIO_PCNF1_STATLEN_Pos) & RADIO_PCNF1_STATLEN_Msk) |
                         ((((uint32_t)MAX_PAYLOAD_LENGTH) << RADIO_PCNF1_MAXLEN_Pos)  & RADIO_PCNF1_MAXLEN_Msk)  |
                         ((RADIO_PCNF1_WHITEEN_Enabled    << RADIO_PCNF1_WHITEEN_Pos) & RADIO_PCNF1_WHITEEN_Msk);
    
    NRF_RADIO->CRCPOLY      = RADIO_CRCPOLY_24BIT;
    NRF_RADIO->CRCCNF       = (
                                ((RADIO_CRCCNF_SKIPADDR_Skip) << RADIO_CRCCNF_SKIPADDR_Pos) & RADIO_CRCCNF_SKIPADDR_Msk) |
                                (((RADIO_CRCCNF_LEN_Three)    << RADIO_CRCCNF_LEN_Pos)      & RADIO_CRCCNF_LEN_Msk
                              );
    NRF_RADIO->CRCINIT      = RADIO_CRCINIT_24BIT;

    NRF_RADIO->TXADDRESS    = 0;
    NRF_RADIO->RXADDRESSES  = 1;

    NRF_RADIO->MODE         = ((RADIO_MODE_MODE_Ble_1Mbit) << RADIO_MODE_MODE_Pos) & RADIO_MODE_MODE_Msk;
    NRF_RADIO->TIFS         = INTERFRAM_SPACING;
    NRF_RADIO->PREFIX0      = ((BLE_ACCESS_ADDR & 0xff000000) >> 24);
    NRF_RADIO->BASE0        = ((BLE_ACCESS_ADDR & 0x00ffffff) << 8 );
}


void radio_setStartFrameCb(radio_capture_cbt cb) {

    radio_vars.startFrame_cb  = cb;
}


void radio_setEndFrameCb(radio_capture_cbt cb) {

    radio_vars.endFrame_cb = cb;
}


void radio_reset(void) {

    // reset is implemented by power off and power radio
    NRF_RADIO->POWER = ((uint32_t)(0)) << RADIO_POWER_POWER_POS;
    NRF_RADIO->POWER = ((uint32_t)(1)) << RADIO_POWER_POWER_POS;

    radio_vars.state  = RADIOSTATE_STOPPED;
}

void radio_setConfig (radioSetting_t radioSetting){
    selected_radioSetting = radioSetting;
    //do nothing
}

void radio_setFrequency(uint8_t frequency, radio_freq_t tx_or_rx) {

    NRF_RADIO->FREQUENCY = FREQUENCY_STEP*(frequency-FREQUENCY_OFFSET);

    radio_vars.state     = RADIOSTATE_FREQUENCY_SET;
}

void radio_ble_setFrequency(uint8_t channel) {

    NRF_RADIO->FREQUENCY   = ble_channel_to_frequency(channel);
    NRF_RADIO->DATAWHITEIV = channel; 

    radio_vars.state  = RADIOSTATE_FREQUENCY_SET;
}

int8_t radio_getFrequencyOffset(void){
  
    return 0; 
}


void radio_rfOn(void) {
    // power on radio
    NRF_RADIO->POWER = ((uint32_t)(1)) << 0;

    radio_vars.state = RADIOSTATE_STOPPED;
}


void radio_rfOff(void) {

    radio_vars.state  = RADIOSTATE_TURNING_OFF;

    NRF_RADIO->EVENTS_DISABLED = 0;

    // stop radio
    NRF_RADIO->TASKS_DISABLE = (uint32_t)(1);

    while(NRF_RADIO->EVENTS_DISABLED==0);

    leds_radio_off();
    debugpins_radio_clr();

    radio_vars.state  = RADIOSTATE_RFOFF;
}


void radio_loadPacket(uint8_t* packet, uint16_t len) {

    radio_vars.state  = RADIOSTATE_LOADING_PACKET;

    ///< note: 1st byte should be the payload size (for Nordic), and
    ///   the two last bytes are used by the MAC layer for CRC
    if ((len > 0) && (len <= MAX_PACKET_SIZE)) {
        radio_vars.payload[0]= len;
        memcpy(&radio_vars.payload[1], packet, len);
    }

    // (re)set payload pointer
    NRF_RADIO->PACKETPTR = (uint32_t)(radio_vars.payload);

    radio_vars.state  = RADIOSTATE_PACKET_LOADED;
}

void radio_ble_loadPacket(uint8_t* packet, uint16_t len) {
    radio_vars.state  = RADIOSTATE_LOADING_PACKET;

    ///< note: 1st byte should be the payload size (for Nordic), and
    ///   the two last bytes are used by the MAC layer for CRC
    if ((len > 0) && (len <= MAX_PACKET_SIZE)) {
        memcpy(&radio_vars.payload[0], packet, len);
    }

    // (re)set payload pointer
    NRF_RADIO->PACKETPTR = (uint32_t)(radio_vars.payload);

    radio_vars.state  = RADIOSTATE_PACKET_LOADED;
}


void radio_txEnable(void) {

    radio_vars.state  = RADIOSTATE_ENABLING_TX;

    NRF_RADIO->EVENTS_READY = (uint32_t)0;

    NRF_RADIO->TASKS_TXEN = (uint32_t)1;
    while(NRF_RADIO->EVENTS_READY==0);


    // wiggle debug pin
    debugpins_radio_set();
    leds_radio_on();

    radio_vars.state  = RADIOSTATE_TX_ENABLED;
}


void radio_txNow(void) {

    NRF_RADIO->TASKS_START = (uint32_t)1;

    radio_vars.state = RADIOSTATE_TRANSMITTING;
}


void radio_rxEnable(void) {

    radio_vars.state = RADIOSTATE_ENABLING_RX;

    if (NRF_RADIO->STATE != STATE_RX) {

        // turn off radio first
        radio_rfOff();

        NRF_RADIO->EVENTS_READY = (uint32_t)0;

        NRF_RADIO->TASKS_RXEN  = (uint32_t)1;

        while(NRF_RADIO->EVENTS_READY==0);
    }
}


void radio_rxNow(void) {

    NRF_RADIO->TASKS_START = (uint32_t)1;

    debugpins_radio_set();
    leds_radio_on();

    radio_vars.state  = RADIOSTATE_LISTENING;
}


void radio_getReceivedFrame(uint8_t* pBufRead,
                            uint8_t* pLenRead,
                            uint8_t  maxBufLen,
                             int8_t* pRssi,
                            uint8_t* pLqi,
                               bool* pCrc)
{
    // check for length parameter; if too long, payload won't fit into memory
    uint8_t len;

    len = radio_vars.payload[0];

    if (len == 0) {
        return; 
    }

    if (len > MAX_PACKET_SIZE) { 
        len = MAX_PACKET_SIZE; 
    }

    if (len > maxBufLen) { 
        len = maxBufLen; 
    }

    // copy payload
    memcpy(pBufRead, &radio_vars.payload[1], len);

    // store other parameters
    *pLenRead = len;
    *pLqi = radio_vars.payload[radio_vars.payload[0]-1];

    // For the RSSI calculation, see 
    //
    // - http://infocenter.nordicsemi.com/topic/com.nordic.infocenter.nrf52840.ps/radio.html?cp=2_0_0_5_19_11_6#ieee802154_rx and
    // - https://www.metageek.com/training/resources/understanding-rssi.html
    //
    // Our RSSI will be in the range -91 dB (worst) to 0 dB (best)
    *pRssi = (*pLqi > 91)?(0):(((int8_t) *pLqi) - 91);

    *pCrc = (NRF_RADIO->CRCSTATUS == 1U);
}

void                radio_ble_getReceivedFrame(uint8_t* pBufRead,
                            uint8_t* pLenRead,
                            uint8_t  maxBufLen,
                             int8_t* pRssi,
                            uint8_t* pLqi,
                               bool* pCrc) 
{
    // check for length parameter; if too long, payload won't fit into memory
    uint8_t len;

    len = radio_vars.payload[1];

    if (len == 0) {
        return; 
    }

    if (len > MAX_PACKET_SIZE) {
        len = MAX_PACKET_SIZE; 
    }

    if (len > maxBufLen) {
        len = maxBufLen; 
    }

    // copy payload
    memcpy(pBufRead, &radio_vars.payload[0], len+2);

    // store other parameters
    *pLenRead = len+2;

    *pCrc = (NRF_RADIO->CRCSTATUS == 1U);

}


//=========================== private =========================================

static uint32_t swap_bits(uint32_t inp) {

    uint32_t i;
    uint32_t retval = 0;

    inp = (inp & 0x000000FFUL);

    for (i = 0; i < 8; i++) {
        retval |= ((inp >> i) & 0x01) << (7 - i);
    }

    return retval;
}


static uint32_t bytewise_bitswap(uint32_t inp) {

    return (swap_bits(inp >> 24) << 24)
          | (swap_bits(inp >> 16) << 16)
          | (swap_bits(inp >> 8) << 8)
          | (swap_bits(inp));
}

static uint8_t ble_channel_to_frequency(channel) {

    uint8_t frequency;
    
    if (channel<=10) {

        frequency = 4+2*channel;
    } else {
        if (channel >=11 && channel <=36) {
            
            frequency = 28+2*(channel-11);
        } else {
            switch(channel){
                case 37:
                    frequency = 2;
                break;
                case 38:
                    frequency = 26;
                break;
                case 39:
                    frequency = 80;
                break;
                default:
                    // something goes wrong
                    frequency = 2;

            }
        }
    }

    return frequency;
}


//=========================== callbacks =======================================

//=========================== interrupt handlers ==============================

void RADIO_IRQHandler(void) {

    if (NRF_RADIO->EVENTS_ADDRESS) {

        NRF_RADIO->EVENTS_ADDRESS = 0;

        if (radio_vars.startFrame_cb) {
            radio_vars.startFrame_cb(sctimer_readCounter());
        }
    }

    if (NRF_RADIO->EVENTS_END) {

        NRF_RADIO->EVENTS_END = 0;

        if (radio_vars.endFrame_cb) {
            radio_vars.endFrame_cb(sctimer_readCounter());
        }
    }
}
