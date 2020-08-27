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

#define DFEOPMODE_DISABLE         0
#define DFEOPMODE_AOD             2
#define DFEOPMODE_AOA             3


#define ANT_SWITCH_PORT   0
#define ANT_SWITCH_PIN0   4    // p0.04
#define ANT_SWITCH_PIN1   5    // p0.05
#define ANT_SWITCH_PIN2   6    // p0.06
#define ANT_SWITCH_PIN3   7    // p0.07
#define ANT_SWITCH_PIN4   23   // p0.23
#define ANT_SWITCH_PIN5   24   // p0.24
#define ANT_SWITCH_PIN6   25   // p0.25
#define ANT_SWITCH_PIN7   26   // p0.26

// the maxmium should be ((1<<14)-1), but need larger .bss size
#define MAX_IQSAMPLES     ((1<<8)-1) 
//=========================== variables =======================================

typedef struct {
    radio_capture_cbt         startFrame_cb;
    radio_capture_cbt         endFrame_cb;
    radio_state_t             state;
    uint8_t                   payload[MAX_PACKET_SIZE] __attribute__ ((aligned));
    uint32_t                  bf_samples[MAX_IQSAMPLES];
} radio_vars_t;

radio_vars_t radio_vars;

//=========================== private =========================================

uint32_t ble_channel_to_frequency(uint8_t channel);
void     radio_configure_direction_finding(void);

//=========================== public ==========================================

//===== admin

void radio_init(void) {

    // clear variables
    memset(&radio_vars,0,sizeof(radio_vars_t));

    // set radio configuration parameters
    NRF_RADIO_NS->TXPOWER  = (uint32_t)RADIO_TXPOWER;

    // configure packet
    NRF_RADIO_NS->PCNF0    =   (((1UL) << RADIO_PCNF0_S0LEN_Pos) & RADIO_PCNF0_S0LEN_Msk) | 
                         (((0UL) << RADIO_PCNF0_S1LEN_Pos) & RADIO_PCNF0_S1LEN_Msk) |
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

    NRF_RADIO_NS->SHORTS = RADIO_SHORTS_END_DISABLE_Msk;

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
    NRF_RADIO_NS->INTENSET = RADIO_INTENSET_ADDRESS_Enabled << RADIO_INTENSET_ADDRESS_Pos |
                          RADIO_INTENSET_END_Enabled     << RADIO_INTENSET_END_Pos;
    
    NVIC->ICPR[((uint32_t)RADIO_IRQn)>>5] = 
       ((uint32_t)1) << ( ((uint32_t)RADIO_IRQn) & 0x1f);
    NVIC->ISER[((uint32_t)RADIO_IRQn)>>5] = 
       ((uint32_t)1) << ( ((uint32_t)RADIO_IRQn) & 0x1f);

    radio_vars.state  = RADIOSTATE_STOPPED;

    radio_configure_direction_finding();
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
    memcpy(bufRead, &radio_vars.payload[0], len+2);

    // store other parameters
    *bufRead = len+2;

    *crc = (NRF_RADIO_NS->CRCSTATUS == 1U);
}

//=========================== private =========================================

#define NUM_SWITCH_PINS     8
#define NUM_SAMPLES         80

// DFECTRL1 register values
#define LEN_CTE             20 // in unit of 8 us
#define DFEINEXTENSION      1  // 1:crc  0:payload
#define TSWITCHSPACING      2  // 1:4us 2:2us 3: 1us
#define TSAMPLESPACINGREF   2  // 1:4us 2:2us 3: 1us 4:500ns 5:250ns 6:125ns
#define SAMPLETYPE          0  // 0: IQ  1: magPhase
#define TSAMPLESPACING      2  // 1:4us 2:2us 3: 1us 4:500ns 5:250ns 6:125ns 

void radio_configure_direction_finding(void) {

    uint8_t i;

    // enable direction finding in AoA mode
    NRF_RADIO_NS->DFEMODE = DFEOPMODE_AOA;

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
    NRF_RADIO_NS->PSEL.DFEGPIO[5] = (uint32_t)(
                                        (ANT_SWITCH_PORT << 5)      |
                                        (ANT_SWITCH_PIN5 << 0)      |
                                        (0 << 31)
                                    );
    NRF_RADIO_NS->PSEL.DFEGPIO[6] = (uint32_t)(
                                        (ANT_SWITCH_PORT << 5)      |
                                        (ANT_SWITCH_PIN6 << 0)      |
                                        (0 << 31)
                                    );
    NRF_RADIO_NS->PSEL.DFEGPIO[7] = (uint32_t)(
                                        (ANT_SWITCH_PORT << 5)      |
                                        (ANT_SWITCH_PIN7 << 0)      |
                                        (0 << 31)
                                    );

    // write switch pattern
    NRF_RADIO_NS->CLEARPATTERN    = 1;
    for (i=0; i<NUM_SWITCH_PINS; i++) {
        NRF_RADIO_NS->SWITCHPATTERN   = (uint32_t)(1<<i);
    }

    NRF_RADIO_NS->DFECTRL1          = (uint32_t)(LEN_CTE << 0)            | 
                                      (uint32_t)(DFEINEXTENSION << 7)     |
                                      (uint32_t)(TSWITCHSPACING << 8)     |
                                      (uint32_t)(TSAMPLESPACINGREF << 12) |
                                      (uint32_t)(SAMPLETYPE << 15)        |
                                      (uint32_t)(TSAMPLESPACING << 16);

    NRF_RADIO_NS->DFEPACKET.MAXCNT  = NUM_SAMPLES;
    NRF_RADIO_NS->DFEPACKET.PTR     = (uint32_t)(&radio_vars.bf_samples[0]);

    // configure 
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