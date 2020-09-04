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
#include "radio_df.h"

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

#define MAX_PACKET_SIZE           (255)       ///< maximal size of radio packet (one more byte at the beginning needed to store the length)
#define CRC_POLYNOMIAL            (0x11021)   ///< polynomial used for CRC calculation in 802.15.4 frames (x^16 + x^12 + x^5 + 1)

#define WAIT_FOR_RADIO_DISABLE    (0)         ///< whether the driver shall wait until the radio is disabled upon calling radio_rfOff()
#define WAIT_FOR_RADIO_ENABLE     (1)         ///< whether the driver shall wait until the radio is enabled upon calling radio_txEnable() or radio_rxEnable()

#define RADIO_CRCINIT_24BIT       0x555555
#define RADIO_CRCPOLY_24BIT       0x0000065B  /// ref: https://devzone.nordicsemi.com/f/nordic-q-a/44111/crc-register-values-for-a-24-bit-crc

#define INTERFRAM_SPACING         (150)       // in us

#define BLE_ACCESS_ADDR           0x8E89BED6  // the actual address is 0xD6, 0xBE, 0x89, 0x8E

#define RADIO_TXPOWER             0 // in 2-compilant format

// the maxmium should be ((1<<14)-1), but need larger .bss size
#define MAX_IQSAMPLES            ((1<<8)-1) 

//=========================== variables =======================================

typedef struct {
    radio_capture_cbt         startFrame_cb;
    radio_capture_cbt         endFrame_cb;
    radio_state_t             state;
    uint8_t                   payload[MAX_PACKET_SIZE] __attribute__ ((aligned));
    uint32_t                  df_samples[MAX_IQSAMPLES];
} radio_vars_t;

radio_vars_t radio_vars;

//=========================== private =========================================

extern void nrf_gpio_cfg_output(uint8_t port_number, uint32_t pin_number);

uint32_t ble_channel_to_frequency(uint8_t channel);

//=========================== public ==========================================

//===== admin

void radio_init(void) {

    uint8_t i;

    // clear variables
    memset(&radio_vars,0,sizeof(radio_vars_t));

    // set radio configuration parameters
    NRF_RADIO_NS->TXPOWER  = (uint32_t)RADIO_TXPOWER;

    // configure packet
    NRF_RADIO_NS->PCNF0    =   (((1UL) << RADIO_PCNF0_S0LEN_Pos) & RADIO_PCNF0_S0LEN_Msk) | 
                         (((8UL) << RADIO_PCNF0_S1LEN_Pos) & RADIO_PCNF0_S1LEN_Msk) |
                         (((8UL) << RADIO_PCNF0_LFLEN_Pos) & RADIO_PCNF0_LFLEN_Msk);

    NRF_RADIO_NS->PCNF1    =   (((RADIO_PCNF1_ENDIAN_Little)    << RADIO_PCNF1_ENDIAN_Pos)  & RADIO_PCNF1_ENDIAN_Msk)  |
                         (((3UL)                          << RADIO_PCNF1_BALEN_Pos)   & RADIO_PCNF1_BALEN_Msk)   |
                         (((0UL)                          << RADIO_PCNF1_STATLEN_Pos) & RADIO_PCNF1_STATLEN_Msk) |
                         ((((uint32_t)MAX_PACKET_SIZE)    << RADIO_PCNF1_MAXLEN_Pos)  & RADIO_PCNF1_MAXLEN_Msk)  |
                         ((RADIO_PCNF1_WHITEEN_Enabled    << RADIO_PCNF1_WHITEEN_Pos) & RADIO_PCNF1_WHITEEN_Msk);

        
    NRF_RADIO_NS->CRCPOLY  = RADIO_CRCPOLY_24BIT;
    NRF_RADIO_NS->CRCCNF   = (
                                ((RADIO_CRCCNF_SKIPADDR_Skip) << RADIO_CRCCNF_SKIPADDR_Pos) & RADIO_CRCCNF_SKIPADDR_Msk) |
                                (((RADIO_CRCCNF_LEN_Three)    << RADIO_CRCCNF_LEN_Pos)      & RADIO_CRCCNF_LEN_Msk
                          );
    NRF_RADIO_NS->CRCINIT      = RADIO_CRCINIT_24BIT;

    NRF_RADIO_NS->TXADDRESS    = 0;
    NRF_RADIO_NS->RXADDRESSES  = 1;

    NRF_RADIO_NS->MODE         = ((RADIO_MODE_MODE_Ble_1Mbit) << RADIO_MODE_MODE_Pos) & RADIO_MODE_MODE_Msk;
    NRF_RADIO_NS->TIFS         = INTERFRAM_SPACING;
    NRF_RADIO_NS->PREFIX0      = ((BLE_ACCESS_ADDR & 0xff000000) >> 24);
    NRF_RADIO_NS->BASE0        = ((BLE_ACCESS_ADDR & 0x00ffffff) << 8 );

    NRF_RADIO_NS->PACKETPTR = (uint32_t)(radio_vars.payload);

    NRF_RADIO_NS->SHORTS = RADIO_SHORTS_PHYEND_DISABLE_Enabled << RADIO_SHORTS_PHYEND_DISABLE_Pos;

    // set priority and disable interrupt in NVIC
    NVIC->IPR[((uint32_t)RADIO_IRQn)] = 
        (uint8_t)(
            (
                RADIO_PRIORITY << (8 - __NVIC_PRIO_BITS)
            ) & (uint32_t)0xff
        );
    NVIC->ICER[((uint32_t)RADIO_IRQn)>>5] = 
       ((uint32_t)1) << ( ((uint32_t)RADIO_IRQn) & 0x1f);

    // enable address and payload interrupts 
    NRF_RADIO_NS->INTENSET = RADIO_INTENSET_ADDRESS_Set    << RADIO_INTENSET_ADDRESS_Pos |
                             RADIO_INTENSET_END_Set        << RADIO_INTENSET_END_Pos     |
                             RADIO_INTENSET_PHYEND_Set     << RADIO_INTENSET_PHYEND_Pos  |
                             RADIO_INTENSET_CTEPRESENT_Set << RADIO_INTENSET_CTEPRESENT_Pos;
    
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

    NRF_RADIO_NS->FREQUENCY    = ble_channel_to_frequency(channel);
    NRF_RADIO_NS->DATAWHITEIV  = channel; 

    radio_vars.state        = RADIOSTATE_FREQUENCY_SET;
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

    if ((len != 0) && (len <= MAX_PACKET_SIZE)) {
        memcpy(&radio_vars.payload[0], packet, len);
    }

    // (re)set payload pointer
    NRF_RADIO_NS->PACKETPTR = (uint32_t)(radio_vars.payload);

    radio_vars.state  = RADIOSTATE_PACKET_LOADED;
}

void radio_txEnable(void) {

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

    // check for length parameter; if too long, payload won't fit into memory
    uint8_t len;

    len = radio_vars.payload[1];

    if (len > MAX_PACKET_SIZE) {
        len = MAX_PACKET_SIZE; 
    }

    if (len > maxBufLen) {
        len = maxBufLen; 
    }

    // copy payload
    memcpy(bufRead, &radio_vars.payload[0], len+2);

    // store other parameters
    *lenRead = len+2 + 1; // 2 bytes header and 1 byte s1 field

    *rssi = (int8_t)(0-NRF_RADIO_NS->RSSISAMPLE); 

    *crc = (NRF_RADIO_NS->CRCSTATUS == 1U);
}

void radio_get_crc(uint8_t* crc24){

    uint32_t crc;
    crc = NRF_RADIO_NS->RXCRC;

    crc24[0] = (uint8_t)((crc & 0x00ff0000) >> 16);
    crc24[1] = (uint8_t)((crc & 0x0000ff00) >> 8);
    crc24[2] = (uint8_t)((crc & 0x000000ff) >> 0);
}

//=========================== private =========================================

#define ANT_SWITCH_PORT           1

#define ANT_SWITCH_PIN0           6
#define ANT_SWITCH_PIN1           7
#define ANT_SWITCH_PIN2           8
#define ANT_SWITCH_PIN3           9
#define ANT_SWITCH_PIN4           10

void radio_configure_direction_finding_antenna_switch(void) {
    
    uint8_t i;

    NRF_RADIO_NS->EVENTS_DISABLED = (uint32_t)0;
    NRF_RADIO_NS->TASKS_DISABLE = (uint32_t)1;
    while(NRF_RADIO_NS->EVENTS_DISABLED == 0);

    nrf_gpio_cfg_output(ANT_SWITCH_PORT,  ANT_SWITCH_PIN0);
    nrf_gpio_cfg_output(ANT_SWITCH_PORT,  ANT_SWITCH_PIN1);
    nrf_gpio_cfg_output(ANT_SWITCH_PORT,  ANT_SWITCH_PIN2);
    nrf_gpio_cfg_output(ANT_SWITCH_PORT,  ANT_SWITCH_PIN3);
    nrf_gpio_cfg_output(ANT_SWITCH_PORT,  ANT_SWITCH_PIN4);

    NRF_P1_NS->OUTCLR = 0x000007C0;
        
    // configure GPIO pins
    NRF_RADIO_NS->PSEL.DFEGPIO[0] = (uint32_t)(
                                        (ANT_SWITCH_PORT << 5)      |
                                        (ANT_SWITCH_PIN0 << 0)      |
                                        (0 << 31)
                                    );
    NRF_RADIO_NS->PSEL.DFEGPIO[1] = (uint32_t)(
                                        (ANT_SWITCH_PORT << 5)      |
                                        (ANT_SWITCH_PIN1 << 0)      |
                                        (0 << 31)
                                    );
    NRF_RADIO_NS->PSEL.DFEGPIO[2] = (uint32_t)(
                                        (ANT_SWITCH_PORT << 5)      |
                                        (ANT_SWITCH_PIN2 << 0)      |
                                        (0 << 31)
                                    );
    NRF_RADIO_NS->PSEL.DFEGPIO[3] = (uint32_t)(
                                        (ANT_SWITCH_PORT << 5)      |
                                        (ANT_SWITCH_PIN3 << 0)      |
                                        (0 << 31)
                                    );
    NRF_RADIO_NS->PSEL.DFEGPIO[4] = (uint32_t)(
                                        (ANT_SWITCH_PORT << 5)      |
                                        (ANT_SWITCH_PIN4 << 0)      |
                                        (0 << 31)
                                    );

    // write switch pattern
    NRF_RADIO_NS->CLEARPATTERN  = (uint32_t)1;

    NRF_RADIO_NS->SWITCHPATTERN = (uint32_t)(1<<0);
    NRF_RADIO_NS->SWITCHPATTERN = (uint32_t)(1<<1);
    NRF_RADIO_NS->SWITCHPATTERN = (uint32_t)(1<<2);
    NRF_RADIO_NS->SWITCHPATTERN = (uint32_t)(1<<3);
    NRF_RADIO_NS->SWITCHPATTERN = (uint32_t)(1<<4);
}


// DFECTRL1 register values

#define NUMBEROF8US         3 // in unit of 8 us
#define DFEINEXTENSION      1  // 1:crc  0:payload
#define TSWITCHSPACING      2  // 1:4us 2:2us 3: 1us
#define TSAMPLESPACINGREF   3  // 1:4us 2:2us 3: 1us 4:500ns 5:250ns 6:125ns
#define SAMPLETYPE          1  // 0: IQ  1: magPhase
#define TSAMPLESPACING      2  // 1:4us 2:2us 3: 1us 4:500ns 5:250ns 6:125ns 

// DFECTRL2 register values

#define TSWITCHOFFSET             0 // 
#define TSAMPLEOFFSET             3 //

// DFEMODE

#define DFEOPMODE_DISABLE         0 //
#define DFEOPMODE_AOD             2 //
#define DFEOPMODE_AOA             3 //

void radio_configure_direction_finding_manual(void) {

    radio_configure_direction_finding_antenna_switch();

    // enable direction finding in AoA mode
    NRF_RADIO_NS->DFEMODE = (uint32_t)DFEOPMODE_AOA;

    NRF_RADIO_NS->CTEINLINECONF     = (uint32_t)0;

    NRF_RADIO_NS->DFECTRL1          = (uint32_t)(NUMBEROF8US << 0)        | 
                                      (uint32_t)(DFEINEXTENSION << 7)     |
                                      (uint32_t)(TSWITCHSPACING << 8)     |
                                      (uint32_t)(TSAMPLESPACINGREF << 12) |
                                      (uint32_t)(SAMPLETYPE << 15)        |
                                      (uint32_t)(TSAMPLESPACING << 16);

    NRF_RADIO_NS->DFECTRL2          = (uint32_t)(TSWITCHOFFSET << 0) |
                                      (uint32_t)(TSAMPLEOFFSET << 0);
  

    NRF_RADIO_NS->DFEPACKET.MAXCNT  = SAMPLE_MAXCNT;
    NRF_RADIO_NS->DFEPACKET.PTR     = (uint32_t)(&radio_vars.df_samples[0]);
}

#define CTEINLINECTRLEN     1 // 1: enabled 0: disabled
#define CTEINFOINS1         1 // 1: data channel PDU  0: advertising channel PDU
#define CTEERRORHANDLING    0 // 1: sampling and antenna switch when crc is not OK, 0: no sampling and antenna ...
#define CTETIMEVALIDRANGE   0 // 0: 20, 1: 31, 2: 63 (in uint of 8 us)
// sample spacing in switching period: 
#define CTEINLINERXMODE1US  2 // 1 4us, 2 2us, 3 1us, 4 0.5us, 5 0.25us, 6 0.125us (used when tswitchingspace is set to 2us)
#define CTEINLINERXMODE2US  2 // 1 4us, 2 2us, 3 1us, 4 0.5us, 5 0.25us, 6 0.125us (used when tswitchingspace is set to 4us)
#define S0CONF              0x20
#define S0MASK              0x20

void radio_configure_direction_finding_inline(void) {

    // enable direction finding in AoA mode
    NRF_RADIO_NS->DFEMODE           = (uint32_t)DFEOPMODE_AOA;

    NRF_RADIO_NS->CTEINLINECONF     = (uint32_t)(CTEINLINECTRLEN    << 0) |
                                      (uint32_t)(CTEINFOINS1        << 3) |
                                      (uint32_t)(CTEERRORHANDLING   << 4) |
                                      (uint32_t)(CTETIMEVALIDRANGE  << 6) |
                                      (uint32_t)(CTEINLINERXMODE1US << 10)|
                                      (uint32_t)(CTEINLINERXMODE2US << 13)|
                                      (uint32_t)(S0CONF             << 16)|
                                      (uint32_t)(S0MASK             << 24);

    NRF_RADIO_NS->DFECTRL1          = (uint32_t)(NUMBEROF8US        << 0) | 
                                      (uint32_t)(DFEINEXTENSION     << 7) |
                                      (uint32_t)(TSWITCHSPACING     << 8) |
                                      (uint32_t)(TSAMPLESPACINGREF  << 12)|
                                      (uint32_t)(SAMPLETYPE         << 15)|
                                      (uint32_t)(TSAMPLESPACING     << 16);

    NRF_RADIO_NS->DFECTRL2          = (uint32_t)(TSWITCHOFFSET      << 0) |
                                      (uint32_t)(TSAMPLEOFFSET      << 0);
  

    NRF_RADIO_NS->DFEPACKET.MAXCNT  = SAMPLE_MAXCNT;
    NRF_RADIO_NS->DFEPACKET.PTR     = (uint32_t)(&radio_vars.df_samples[0]);
}

void radio_get_df_samples(uint32_t* sample_buffer, uint16_t length) {

    uint16_t i;

    for (i=0;i<length;i++) {
        sample_buffer[i] = radio_vars.df_samples[i];        
    }

    // reset sample buffer once accessed
    memset((uint8_t*)&radio_vars.df_samples[0], 0, 4*MAX_IQSAMPLES);
}

uint32_t ble_channel_to_frequency(uint8_t channel) {

    uint32_t frequency;
    
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

     // CTE presence
    if (NRF_RADIO_NS->EVENTS_CTEPRESENT){
        
        NRF_RADIO_NS->EVENTS_CTEPRESENT = (uint32_t)0;
        return KICK_SCHEDULER;
    }

    // END 
    if (NRF_RADIO_NS->EVENTS_END) {
        
        NRF_RADIO_NS->EVENTS_END = (uint32_t)0;
        return KICK_SCHEDULER;
    }

    // end of frame
    if (NRF_RADIO_NS->EVENTS_PHYEND){

        debugpins_frame_toggle();
        
        if (radio_vars.endFrame_cb!=NULL){
            radio_vars.endFrame_cb(time_stampe);
        }
        
        NRF_RADIO_NS->EVENTS_PHYEND = (uint32_t)0;
        return KICK_SCHEDULER;
    }
    return DO_NOT_KICK_SCHEDULER;
}