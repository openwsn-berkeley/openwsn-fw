/**
\brief nRF5340_network-specific definition of the "radio" bsp module.

\author: Tengfei Chang <tengfei.chang@inria.fr> August 2020
*/

#include "nRF5340_network.h"
#include "nrf5340_network_bitfields.h"
#include "board.h"
#include "radio.h"
#include "debugpins.h"
#include "leds.h"

//=========================== defines =========================================

#define MAX_PKT_LEN             128

#define RADIO_POWER_POWER_POS   0

#define RADIO_PCNF0_LFLEN       8
#define RADIO_PCNF0_LFLEN_POS   0
#define RADIO_PCNF0_S0LEN       0
#define RADIO_PCNF0_S0LEN_POS   8
#define RADIO_PCNF0_S1LEN       0
#define RADIO_PCNF0_S1LEN_POS   16

#define RADIO_PCNF1_MAXLEN          MAX_PKT_LEN
#define RADIO_PCNF1_MAXLEN_POS      0
#define RADIO_PCNF1_STATLEN         0
#define RADIO_PCNF1_STATLEN_POS     8
#define RADIO_PCNF1_BALEN           2
#define RADIO_PCNF1_BALEN_POS       16
#define RADIO_PCNF1_ENDIAN          0
#define RADIO_PCNF1_ENDIAN_POS      24
#define RADIO_PCNF1_WHITEEN         1   // enable data whiteen
#define RADIO_PCNF1_WHITEEN_POS     25

// CRC poly: x^24+x^10+x^9+x^6+x^4+x^3+x+1  
// Refer to: https://devzone.nordicsemi.com/f/nordic-q-a/44111/crc-register-values-for-a-24-bit-crc
#define RADIO_CRCPOLY_24BIT         0x0000065B

#define RADIO_CRCINIT_24BIT         0x555555

#define RADIO_CRCCNF_LEN            3
#define RADIO_CRCCNF_LEN_POS        0
#define RADIO_CRCCNF_SKIPADDR       1
#define RADIO_CRCCNF_SKIPADDR_POS   8

#define RADIO_TXADDRESS_ADDR0       0
#define RADIO_TXADDRESS_ADDR0_POS   0

#define RADIO_RXADDRESSES_ADDR0     1
#define RADIO_RXADDRESSES_ADDR0_POS 0

// RADIO mode
#define Nrf_1Mbit                   0
#define Nrf_2Mbit                   1
#define Ble_1Mbit                   3
#define Ble_2Mbit                   4
#define Ble_LR125Kbit               5
#define Ble_LR500Kbit               6
#define Ieeee802154_250Kbit         15
#define RADIO_MODE_MODE_POS         0

#define RADIO_TIFS_TIFS             150 // interframe spacing in us

#define RADIO_INTENSET_ADDRESS_POS  1
#define RADIO_INTENSET_END_POS      3

#define RADIO_TXPOWER_TXPOWER       0 // 2 = 2dbm, 0xFC = -4dbm

#define STATE_DISABLED              0
#define STATE_RXRU                  1
#define STATE_RXIDLE                2
#define STATE_RX                    3
#define STATE_RXDISABLE             4
#define STATE_TXTU                  9
#define STATE_TXIDLE                10
#define STATE_TX                    11
#define STATE_TXDIABLE              12

#define DEFAULT_FREQUENCY           4 // frequency = 2400MHz+DEFAULT_FREQUENCY
#define DEFAULT_CHANNEL             0 // default channel 0

#define BASEADDRESS_0               0xca
#define BASEADDRESS_1               0xfe
#define BASEADDRESS_PREFIX          0xff

//=========================== variables =======================================

// BLE advertising accesses address
const static uint8_t ble_access_addr[4] = { 0xD6, 0xBE, 0x89, 0x8E };


typedef struct {
    radio_capture_cbt         startFrame_cb;
    radio_capture_cbt         endFrame_cb;
    radio_state_t             state;
    uint8_t                   trx_frame_buffer[MAX_PKT_LEN];
} radio_vars_t;

radio_vars_t radio_vars;

//=========================== private =========================================

uint32_t ble_freq_calculate(uint8_t channel);

//=========================== public ==========================================

//===== admin

void radio_init(void) {
    // clear variables
    memset(&radio_vars,0,sizeof(radio_vars_t));

    // set priority and disable interrupt in NVIC
    NVIC->IPR[((uint32_t)RADIO_IRQn)] = 
        (uint8_t)(
            (
                RADIO_PRIORITY << (8 - __NVIC_PRIO_BITS)
            ) & (uint32_t)0xff
        );
    NVIC->ICER[((uint32_t)RADIO_IRQn)>>5] = 
       ((uint32_t)1) << ( ((uint32_t)RADIO_IRQn) & 0x1f);

    radio_reset();

    // configure packet
    NRF_RADIO_NS->PCNF0 =   
          (((uint32_t)RADIO_PCNF0_LFLEN) << RADIO_PCNF0_LFLEN_POS)
        | (((uint32_t)RADIO_PCNF0_S0LEN) << RADIO_PCNF0_S0LEN_POS)
        | (((uint32_t)RADIO_PCNF0_S1LEN) << RADIO_PCNF0_S1LEN_POS);

    NRF_RADIO_NS->PCNF1 =   
           (((uint32_t)RADIO_PCNF1_MAXLEN)  << RADIO_PCNF1_MAXLEN_POS)
         | (((uint32_t)RADIO_PCNF1_STATLEN) << RADIO_PCNF1_STATLEN_POS)
         | (((uint32_t)RADIO_PCNF1_BALEN)   << RADIO_PCNF1_BALEN_POS)
         | (((uint32_t)RADIO_PCNF1_ENDIAN)  << RADIO_PCNF1_ENDIAN_POS)
         | (((uint32_t)RADIO_PCNF1_WHITEEN) << RADIO_PCNF1_WHITEEN_POS);

        
    NRF_RADIO_NS->CRCPOLY  = RADIO_CRCPOLY_24BIT;
    NRF_RADIO_NS->CRCCNF   = 
           (((uint32_t)RADIO_CRCCNF_LEN)  << RADIO_CRCCNF_LEN_POS)
         | (((uint32_t)RADIO_CRCCNF_SKIPADDR) << RADIO_CRCCNF_SKIPADDR_POS);

    NRF_RADIO_NS->TXADDRESS = 
        (((uint32_t)RADIO_TXADDRESS_ADDR0)  << RADIO_TXADDRESS_ADDR0_POS);

    NRF_RADIO_NS->RXADDRESSES = 
        (((uint32_t)RADIO_RXADDRESSES_ADDR0)  << RADIO_RXADDRESSES_ADDR0_POS);
        

    NRF_RADIO_NS->MODE = 
        (((uint32_t)Ble_2Mbit)  << RADIO_MODE_MODE_POS);

    NRF_RADIO_NS->TIFS = (uint32_t)RADIO_TIFS_TIFS;

    NRF_RADIO_NS->PREFIX0 = ble_access_addr[3];
    NRF_RADIO_NS->BASE0   = ( (((uint32_t)ble_access_addr[2]) << 24) 
                         | (((uint32_t)ble_access_addr[1]) << 16)
                         | (((uint32_t)ble_access_addr[0]) << 8) );

    NRF_RADIO_NS->CRCINIT = (uint32_t)RADIO_CRCINIT_24BIT;

    NRF_RADIO_NS->TXPOWER = (uint32_t)RADIO_TXPOWER_TXPOWER;

    NRF_RADIO_NS->PACKETPTR = (uint32_t)(radio_vars.trx_frame_buffer);

    NRF_RADIO_NS->SHORTS = RADIO_SHORTS_END_DISABLE_Msk;

    // enable address and payload interrupts 
    NRF_RADIO_NS->INTENSET =
           (((uint32_t)1)   << RADIO_INTENSET_ADDRESS_POS)
         | (((uint32_t)1)   << RADIO_INTENSET_END_POS);
    
    NVIC->ICPR[((uint32_t)RADIO_IRQn)>>5] = 
       ((uint32_t)1) << ( ((uint32_t)RADIO_IRQn) & 0x1f);
    NVIC->ISER[((uint32_t)RADIO_IRQn)>>5] = 
       ((uint32_t)1) << ( ((uint32_t)RADIO_IRQn) & 0x1f);

    radio_vars.state  = RADIOSTATE_STOPPED;
}

void radio_setStartFrameCb(radio_capture_cbt cb) {
    
    radio_vars.startFrame_cb = cb;
}

void radio_setEndFrameCb(radio_capture_cbt cb) {

    radio_vars.endFrame_cb = cb;
}

//===== reset

void radio_reset(void) {

    // reset is implemented by power off and power radio
    NRF_RADIO_NS->POWER = ((uint32_t)(0)) << RADIO_POWER_POWER_POS;
    NRF_RADIO_NS->POWER = ((uint32_t)(1)) << RADIO_POWER_POWER_POS;

    radio_vars.state  = RADIOSTATE_STOPPED;
}

//===== RF admin

void radio_setFrequency(uint8_t channel, radio_freq_t tx_or_rx) {


    uint8_t channel_to_set;
    radio_vars.state  = RADIOSTATE_SETTING_FREQUENCY;

    if (channel>39){
        channel_to_set = DEFAULT_CHANNEL;
    } else {
        channel_to_set = channel;
    }

    // frequency is calculated as: Frequency = 2400 + FREQUENCY (MHz)
    NRF_RADIO_NS->DATAWHITEIV = (uint32_t)(channel_to_set);
    NRF_RADIO_NS->FREQUENCY   = ble_freq_calculate(channel_to_set);
     
    radio_vars.state  = RADIOSTATE_FREQUENCY_SET;
}

void radio_rfOn(void) {

    // power on radio
    NRF_RADIO_NS->POWER = ((uint32_t)(1)) << RADIO_POWER_POWER_POS;

    radio_vars.state  = RADIOSTATE_STOPPED;
}

void radio_rfOff(void) {

    radio_vars.state  = RADIOSTATE_TURNING_OFF;

    NRF_RADIO_NS->EVENTS_DISABLED = 0;

    // stop radio
    NRF_RADIO_NS->TASKS_DISABLE = (uint32_t)(1);

    while(NRF_RADIO_NS->EVENTS_DISABLED==0);

    // wiggle debug pin
    debugpins_radio_clr();
    leds_radio_off();

    radio_vars.state  = RADIOSTATE_RFOFF;
}

int8_t radio_getFrequencyOffset(void){

    // not supported
    return 0;
}

//===== TX

void radio_loadPacket(uint8_t* packet, uint16_t len) {

    radio_vars.state  = RADIOSTATE_LOADING_PACKET;

    // base address: 2 byte BA + 1 byte prefix
    radio_vars.trx_frame_buffer[0] = BASEADDRESS_0;
    radio_vars.trx_frame_buffer[1] = BASEADDRESS_1;
    radio_vars.trx_frame_buffer[2] = BASEADDRESS_PREFIX;
    
    // length field
    radio_vars.trx_frame_buffer[3] = (uint8_t)len;

    memcpy(&radio_vars.trx_frame_buffer[4], packet, len);

    radio_vars.state  = RADIOSTATE_PACKET_LOADED;
}

void radio_txEnable(void){

    radio_vars.state  = RADIOSTATE_ENABLING_TX;

    NRF_RADIO_NS->EVENTS_READY = (uint32_t)0;

    NRF_RADIO_NS->TASKS_TXEN = (uint32_t)1;
    while(NRF_RADIO_NS->EVENTS_READY==0);

    // wiggle debug pin
    debugpins_radio_set();
    leds_radio_on();

    radio_vars.state  = RADIOSTATE_TX_ENABLED;
}

void radio_txNow(void) {

    NRF_RADIO_NS->TASKS_START = (uint32_t)1;

    radio_vars.state  = RADIOSTATE_TRANSMITTING;
}

//===== RX

void radio_rxEnable(void) {

    
    radio_vars.state  = RADIOSTATE_ENABLING_RX;

    if (NRF_RADIO_NS->STATE != STATE_RX){

       // turn off radio first
       radio_rfOff();
        
        NRF_RADIO_NS->EVENTS_READY = (uint32_t)0;

        NRF_RADIO_NS->TASKS_RXEN  = (uint32_t)1;

        while(NRF_RADIO_NS->EVENTS_READY==0);
    }

    // wiggle debug pin
    debugpins_radio_set();
    leds_radio_on();
}

void radio_rxNow(void) {

    NRF_RADIO_NS->TASKS_START = (uint32_t)1;

    radio_vars.state  = RADIOSTATE_LISTENING;
}

void radio_getReceivedFrame(
      uint8_t* bufRead,
      uint8_t* lenRead,
      uint8_t  maxBufLen,
      int8_t*  rssi,
      uint8_t* lqi,
      bool*    crc
   ) {

    if (NRF_RADIO_NS->EVENTS_CRCERROR) {
        *crc        = 0;
    } else {
        *crc        = 1;
    }
    NRF_RADIO_NS->EVENTS_CRCERROR = (uint32_t)0;
    
    if (
        radio_vars.trx_frame_buffer[0] != BASEADDRESS_0 ||
        radio_vars.trx_frame_buffer[1] != BASEADDRESS_1 ||
        radio_vars.trx_frame_buffer[2] != BASEADDRESS_PREFIX
    ){
        return;
    }


    *lenRead        = radio_vars.trx_frame_buffer[3];
    *lqi            = radio_vars.trx_frame_buffer[*lenRead+1];
    *rssi           = (int8_t)(0-NRF_RADIO_NS->RSSISAMPLE); 

    if (*lenRead<=maxBufLen) {
        memcpy(bufRead,&(radio_vars.trx_frame_buffer[4]),*lenRead);
    }
}

//=========================== private =========================================

uint32_t ble_freq_calculate(uint8_t channel){
    
    if (channel<=10){
        return (uint32_t)(4+2*channel);
    } else {
        if(channel<=36){
            return (uint32_t)(28+2*(channel-11));
        } else {
            switch(channel){
            case 37:
                return (uint32_t)2;

            break;
            case 38:
                return (uint32_t)26;
            break;
            case 39:
                return (uint32_t)80;
            break;
            default:
                // invalid channel
                return  (uint32_t)DEFAULT_FREQUENCY;
            }
        }
    }
}

void RADIO_IRQHandler(void){

    debugpins_isr_set();
    
    radio_isr();

    debugpins_isr_clr();
}

//=========================== callbacks =======================================

kick_scheduler_t    radio_isr(void){

    uint32_t time_stampe;

    time_stampe = NRF_RTC0_NS->COUNTER;

    // start of frame (payload)
    if (NRF_RADIO_NS->EVENTS_ADDRESS){
        
        // start sampling rssi
        NRF_RADIO_NS->TASKS_RSSISTART = (uint32_t)1;

        if (radio_vars.startFrame_cb!=NULL){
            radio_vars.startFrame_cb(time_stampe);
        }
        
        NRF_RADIO_NS->EVENTS_ADDRESS = (uint32_t)0;
        return KICK_SCHEDULER;
    }

    // end of frame
    if (NRF_RADIO_NS->EVENTS_END){

        if (radio_vars.endFrame_cb!=NULL){
            radio_vars.endFrame_cb(time_stampe);
        }
        
        NRF_RADIO_NS->EVENTS_END = (uint32_t)0;
        return KICK_SCHEDULER;
    }
    return DO_NOT_KICK_SCHEDULER;
}