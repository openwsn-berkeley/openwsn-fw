/**
\brief nRF5340_network-specific definition of the "radio" bsp module.

\author: Tengfei Chang <tengfei.chang@inria.fr> August 2020
*/

#include "nRF52833.h"
#include "nrf52833_bitfields.h"
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
    uint8_t                   array_to_use;
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
    NRF_RADIO->TXPOWER     = (uint32_t)RADIO_TXPOWER;

    // configure packet
    NRF_RADIO->PCNF0       = 
        (((1UL) << RADIO_PCNF0_S0LEN_Pos) & RADIO_PCNF0_S0LEN_Msk) | 
        (((0UL) << RADIO_PCNF0_S1LEN_Pos) & RADIO_PCNF0_S1LEN_Msk) |
        (((8UL) << RADIO_PCNF0_LFLEN_Pos) & RADIO_PCNF0_LFLEN_Msk);

    NRF_RADIO->PCNF1       = 
        (((RADIO_PCNF1_ENDIAN_Little)    << RADIO_PCNF1_ENDIAN_Pos)  & RADIO_PCNF1_ENDIAN_Msk)  |
        (((3UL)                          << RADIO_PCNF1_BALEN_Pos)   & RADIO_PCNF1_BALEN_Msk)   |
        (((0UL)                          << RADIO_PCNF1_STATLEN_Pos) & RADIO_PCNF1_STATLEN_Msk) |
        ((((uint32_t)MAX_PACKET_SIZE)    << RADIO_PCNF1_MAXLEN_Pos)  & RADIO_PCNF1_MAXLEN_Msk)  |
        ((RADIO_PCNF1_WHITEEN_Enabled    << RADIO_PCNF1_WHITEEN_Pos) & RADIO_PCNF1_WHITEEN_Msk);

        
    NRF_RADIO->CRCPOLY     = RADIO_CRCPOLY_24BIT;
    NRF_RADIO->CRCCNF      = 
        (((RADIO_CRCCNF_SKIPADDR_Skip) << RADIO_CRCCNF_SKIPADDR_Pos) & RADIO_CRCCNF_SKIPADDR_Msk) |
        (((RADIO_CRCCNF_LEN_Three)     << RADIO_CRCCNF_LEN_Pos)      & RADIO_CRCCNF_LEN_Msk);

    NRF_RADIO->CRCINIT     = RADIO_CRCINIT_24BIT;

    NRF_RADIO->TXADDRESS   = 0;
    NRF_RADIO->RXADDRESSES = 1;

    NRF_RADIO->MODE        = ((RADIO_MODE_MODE_Ble_1Mbit) << RADIO_MODE_MODE_Pos) & RADIO_MODE_MODE_Msk;
    NRF_RADIO->TIFS        = INTERFRAM_SPACING;
    NRF_RADIO->PREFIX0     = ((BLE_ACCESS_ADDR & 0xff000000) >> 24);
    NRF_RADIO->BASE0       = ((BLE_ACCESS_ADDR & 0x00ffffff) << 8 );

    NRF_RADIO->PACKETPTR   = (uint32_t)(radio_vars.payload);
    NRF_RADIO->SHORTS      = RADIO_SHORTS_PHYEND_DISABLE_Enabled << RADIO_SHORTS_PHYEND_DISABLE_Pos;

    // set priority and disable interrupt in NVIC
    NVIC->IP[((uint32_t)RADIO_IRQn)]     = 
        (uint8_t)(
            (RADIO_PRIORITY << (8 - __NVIC_PRIO_BITS)) & (uint32_t)0xff
        );
    NVIC->ICER[((uint32_t)RADIO_IRQn)>>5] = 
       ((uint32_t)1) << ( ((uint32_t)RADIO_IRQn) & 0x1f);

    // enable address and payload interrupts 
    NRF_RADIO->INTENSET   = 
        RADIO_INTENSET_ADDRESS_Set    << RADIO_INTENSET_ADDRESS_Pos |
        RADIO_INTENSET_END_Set        << RADIO_INTENSET_END_Pos     |
        RADIO_INTENSET_PHYEND_Set     << RADIO_INTENSET_PHYEND_Pos  |
        RADIO_INTENSET_CTEPRESENT_Set << RADIO_INTENSET_CTEPRESENT_Pos;
    
    NVIC->ICPR[((uint32_t)RADIO_IRQn)>>5] = 
       ((uint32_t)1) << ( ((uint32_t)RADIO_IRQn) & 0x1f);
    NVIC->ISER[((uint32_t)RADIO_IRQn)>>5] = 
       ((uint32_t)1) << ( ((uint32_t)RADIO_IRQn) & 0x1f);

    radio_vars.state        = RADIOSTATE_STOPPED;

    radio_vars.array_to_use = 1;
}

void radio_setStartFrameCb(radio_capture_cbt cb) {
    
    radio_vars.startFrame_cb  = cb;
}

void radio_setEndFrameCb(radio_capture_cbt cb) {

    radio_vars.endFrame_cb    = cb;
}

//===== reset

void radio_reset(void) {

    // reset is implemented by power off and power radio
    NRF_RADIO->POWER = ((uint32_t)(0)) << RADIO_POWER_POWER_POS;
    NRF_RADIO->POWER = ((uint32_t)(1)) << RADIO_POWER_POWER_POS;

    radio_vars.state    = RADIOSTATE_STOPPED;
}

//===== RF admin

void radio_setFrequency(uint8_t channel, radio_freq_t tx_or_rx) {

    NRF_RADIO->FREQUENCY     = ble_channel_to_frequency(channel);
    NRF_RADIO->DATAWHITEIV   = channel; 

    radio_vars.state            = RADIOSTATE_FREQUENCY_SET;
}


uint32_t radio_get_frequency(void) {

    // return value, in MHz
    return (2400 + (NRF_RADIO->FREQUENCY & 0x0000007F));
}

void radio_rfOn(void) {

    // power on radio
    NRF_RADIO->POWER = ((uint32_t)(1)) << RADIO_POWER_POWER_POS;

    radio_vars.state    = RADIOSTATE_STOPPED;
}

void radio_rfOff(void) {

    radio_vars.state  = RADIOSTATE_TURNING_OFF;

    NRF_RADIO->EVENTS_DISABLED = 0;

    // stop radio
    NRF_RADIO->TASKS_DISABLE = (uint32_t)(1);

    while(NRF_RADIO->EVENTS_DISABLED==0);

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

    radio_vars.state  = RADIOSTATE_TRANSMITTING;
}

//===== RX

void radio_rxEnable(void) {

    radio_vars.state  = RADIOSTATE_ENABLING_RX;

    if (NRF_RADIO->STATE != STATE_RX){

       // turn off radio first
       radio_rfOff();
        
        NRF_RADIO->EVENTS_READY = (uint32_t)0;

        NRF_RADIO->TASKS_RXEN  = (uint32_t)1;

        while(NRF_RADIO->EVENTS_READY==0);
    }

    // wiggle debug pin
    debugpins_radio_set();
    leds_radio_on();
}

void radio_rxNow(void) {

    NRF_RADIO->TASKS_START = (uint32_t)1;

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

    uint8_t len;

    len = radio_vars.payload[1];

    if (len > MAX_PACKET_SIZE) {
        len = MAX_PACKET_SIZE; 
    }

    if (len > maxBufLen) {
        len = maxBufLen; 
    }

    // store other parameters
    // 2 bytes header. Add one byte if s1len is set to 8
    *lenRead = len+2;

    // copy payload
    memcpy(bufRead, &radio_vars.payload[0], *lenRead);

    *rssi = (int8_t)(0-NRF_RADIO->RSSISAMPLE); 

    *crc = (NRF_RADIO->CRCSTATUS == 1U);
}

void radio_get_crc(uint8_t* crc24){

    uint32_t crc;
    crc = NRF_RADIO->RXCRC;

    crc24[0] = (uint8_t)((crc & 0x00ff0000) >> 16);
    crc24[1] = (uint8_t)((crc & 0x0000ff00) >> 8);
    crc24[2] = (uint8_t)((crc & 0x000000ff) >> 0);
}

//=========================== private =========================================

// TI BOOSTXL-AOA antenna pin mapping

//  VALUE   DIO27   DIO28   DIO29   DIO30   Antenna
//  0x2     0       1       0       0       A2.1
//  0x4     0       0       1       0       A2.2
//  0x8     0       0       0       1       A2.3
//  0x3     1       1       0       0       A1.1
//  0x5     1       0       1       0       A1.2
//  0x9     1       0       0       1       A1.3

//  0: 0.0V to 0.2V, 1: 2.5V to 5.0V

#define ANT_SWITCH_PORT           1
#define ANT_SWITCH_PIN0           6 // DIO27
#define ANT_SWITCH_PIN1           7 // DIO28
#define ANT_SWITCH_PIN2           8 // DIO29
#define ANT_SWITCH_PIN3           9 // DIO30

#define PATTERN_A2_1              0x2
#define PATTERN_A2_2              0x4
#define PATTERN_A2_3              0x8
#define PATTERN_A1_1              0x3
#define PATTERN_A1_2              0x5
#define PATTERN_A1_3              0x9

#define LED_PORT                  0

void radio_configure_direction_finding_antenna_switch(void) {
    
    uint8_t i;

    NRF_RADIO->EVENTS_DISABLED = (uint32_t)0;
    NRF_RADIO->TASKS_DISABLE = (uint32_t)1;
    while(NRF_RADIO->EVENTS_DISABLED == 0);

    nrf_gpio_cfg_output(ANT_SWITCH_PORT,  ANT_SWITCH_PIN0);
    nrf_gpio_cfg_output(ANT_SWITCH_PORT,  ANT_SWITCH_PIN1);
    nrf_gpio_cfg_output(ANT_SWITCH_PORT,  ANT_SWITCH_PIN2);
    nrf_gpio_cfg_output(ANT_SWITCH_PORT,  ANT_SWITCH_PIN3);

    NRF_P1->OUTCLR = 0x000003C0;
        
    // configure GPIO pins
    NRF_RADIO->PSEL.DFEGPIO[0] = (uint32_t)(
                                        (ANT_SWITCH_PORT << 5)      |
                                        (ANT_SWITCH_PIN0 << 0)      |
                                        (0 << 31)
                                    );
    NRF_RADIO->PSEL.DFEGPIO[1] = (uint32_t)(
                                        (ANT_SWITCH_PORT << 5)      |
                                        (ANT_SWITCH_PIN1 << 0)      |
                                        (0 << 31)
                                    );
    NRF_RADIO->PSEL.DFEGPIO[2] = (uint32_t)(
                                        (ANT_SWITCH_PORT << 5)      |
                                        (ANT_SWITCH_PIN2 << 0)      |
                                        (0 << 31)
                                    );
    NRF_RADIO->PSEL.DFEGPIO[3] = (uint32_t)(
                                        (ANT_SWITCH_PORT << 5)      |
                                        (ANT_SWITCH_PIN3 << 0)      |
                                        (0 << 31)
                                    );

    // write switch pattern

    NRF_RADIO->CLEARPATTERN  = (uint32_t)1;
    if (radio_vars.array_to_use == 2){
        // use antenna array 2
        NRF_RADIO->SWITCHPATTERN = (uint32_t)(PATTERN_A2_2);
        NRF_RADIO->SWITCHPATTERN = (uint32_t)(PATTERN_A2_2);
        NRF_RADIO->SWITCHPATTERN = (uint32_t)(PATTERN_A2_1);
        NRF_RADIO->SWITCHPATTERN = (uint32_t)(PATTERN_A2_3);
    } else {
        // use antenna array 1 by default
        NRF_RADIO->SWITCHPATTERN = (uint32_t)(PATTERN_A1_2);
        NRF_RADIO->SWITCHPATTERN = (uint32_t)(PATTERN_A1_2);
        NRF_RADIO->SWITCHPATTERN = (uint32_t)(PATTERN_A1_1);
        NRF_RADIO->SWITCHPATTERN = (uint32_t)(PATTERN_A1_3);
    }
}


// DFECTRL1 register values

#define NUMBEROF8US         3  // in unit of 8 us
#define DFEINEXTENSION      1  // 1:crc  0:payload
#define TSWITCHSPACING      2  // 1:4us 2:2us 3: 1us
#define TSAMPLESPACINGREF   6  // 1:4us 2:2us 3: 1us 4:500ns 5:250ns 6:125ns
#define SAMPLETYPE          1  // 0: IQ  1: magPhase
#define TSAMPLESPACING      6  // 1:4us 2:2us 3: 1us 4:500ns 5:250ns 6:125ns

// DFECTRL2 register values

#define TSWITCHOFFSET             0 // 
#define TSAMPLEOFFSET             3 //

// DFEMODE

#define DFEOPMODE_DISABLE         0 //
#define DFEOPMODE_AOD             2 //
#define DFEOPMODE_AOA             3 //

void radio_configure_direction_finding_manual(void) {

    // enable direction finding in AoA mode
    NRF_RADIO->DFEMODE = (uint32_t)DFEOPMODE_AOA;

    NRF_RADIO->CTEINLINECONF     = (uint32_t)0;

    NRF_RADIO->DFECTRL1          = (uint32_t)(NUMBEROF8US        << 0)  | 
                                      (uint32_t)(DFEINEXTENSION     << 7)  |
                                      (uint32_t)(TSWITCHSPACING     << 8)  |
                                      (uint32_t)(TSAMPLESPACINGREF  << 12) |
                                      (uint32_t)(SAMPLETYPE         << 15) |
                                      (uint32_t)(TSAMPLESPACING     << 16);

    NRF_RADIO->DFECTRL2          = (uint32_t)(TSWITCHOFFSET      << 0)  |
                                      (uint32_t)(TSAMPLEOFFSET      << 0);
  

    NRF_RADIO->DFEPACKET.MAXCNT  = MAX_IQSAMPLES;
    NRF_RADIO->DFEPACKET.PTR     = (uint32_t)(&radio_vars.df_samples[0]);
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
    NRF_RADIO->DFEMODE           = (uint32_t)DFEOPMODE_AOA;

    NRF_RADIO->CTEINLINECONF     = (uint32_t)(CTEINLINECTRLEN    << 0) |
                                      (uint32_t)(CTEINFOINS1        << 3) |
                                      (uint32_t)(CTEERRORHANDLING   << 4) |
                                      (uint32_t)(CTETIMEVALIDRANGE  << 6) |
                                      (uint32_t)(CTEINLINERXMODE1US << 10)|
                                      (uint32_t)(CTEINLINERXMODE2US << 13)|
                                      (uint32_t)(S0CONF             << 16)|
                                      (uint32_t)(S0MASK             << 24);

    NRF_RADIO->DFECTRL1          = (uint32_t)(NUMBEROF8US        << 0) | 
                                      (uint32_t)(DFEINEXTENSION     << 7) |
                                      (uint32_t)(TSWITCHSPACING     << 8) |
                                      (uint32_t)(TSAMPLESPACINGREF  << 12)|
                                      (uint32_t)(SAMPLETYPE         << 15)|
                                      (uint32_t)(TSAMPLESPACING     << 16);

    NRF_RADIO->DFECTRL2          = (uint32_t)(TSWITCHOFFSET      << 0) |
                                      (uint32_t)(TSAMPLEOFFSET      << 0);
  

    NRF_RADIO->DFEPACKET.MAXCNT  = MAX_IQSAMPLES;
    NRF_RADIO->DFEPACKET.PTR     = (uint32_t)(&radio_vars.df_samples[0]);
}

uint16_t radio_get_df_samples(uint32_t* sample_buffer, uint16_t length) {

    uint16_t i;
    uint16_t num_transfered_samples;
    
    num_transfered_samples = NRF_RADIO->DFEPACKET.AMOUNT;
    if (length < num_transfered_samples) {
        // do not transfer the sample if the given buffer length doesn't fit
        return 0;
    }

    for (i=0;i<num_transfered_samples;i++) {
        sample_buffer[i] = radio_vars.df_samples[i];        
    }

    // reset sample buffer once accessed
    memset((uint8_t*)&radio_vars.df_samples[0], 0, 4*MAX_IQSAMPLES);

    return num_transfered_samples;
}
void     radio_configure_switch_antenna_array(void) {

    if (radio_vars.array_to_use == 2) {
        radio_vars.array_to_use = 1;
    } else {
        radio_vars.array_to_use = 2;
    }
}

uint8_t  radio_get_antenna_array_id(void) {

    return radio_vars.array_to_use;
}


uint32_t ble_channel_to_frequency(uint8_t channel) {

    uint32_t frequency;
    
    if (channel<=10) {

        frequency = 4+2*channel;
    } else {
        if (channel >=11 && channel <=36) {
            
            frequency = 28+2*(channel-11);
        } else {
            switch(channel) {
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

void RADIO_IRQHandler(void) {

    debugpins_isr_set();
    
    radio_isr();

    debugpins_isr_clr();
}

//=========================== callbacks =======================================

kick_scheduler_t    radio_isr(void){

    uint32_t time_stampe;

    time_stampe = NRF_RTC0->COUNTER;

    // start of frame (payload)
    if (NRF_RADIO->EVENTS_ADDRESS){

        // start sampling rssi
        NRF_RADIO->TASKS_RSSISTART = (uint32_t)1;

        if (radio_vars.startFrame_cb!=NULL){
            radio_vars.startFrame_cb(time_stampe);
        }
        
        NRF_RADIO->EVENTS_ADDRESS = (uint32_t)0;
        return KICK_SCHEDULER;
    }

     // CTE presence
    if (NRF_RADIO->EVENTS_CTEPRESENT){

        NRF_RADIO->EVENTS_CTEPRESENT = (uint32_t)0;
        return KICK_SCHEDULER;
    }

    // END 
    if (NRF_RADIO->EVENTS_END) {
        
        NRF_RADIO->EVENTS_END = (uint32_t)0;
        return KICK_SCHEDULER;
    }

    // end of frame
    if (NRF_RADIO->EVENTS_PHYEND) {
        
        if (radio_vars.endFrame_cb!=NULL){
            radio_vars.endFrame_cb(time_stampe);
        }
        
        NRF_RADIO->EVENTS_PHYEND = (uint32_t)0;
        return KICK_SCHEDULER;
    }
    return DO_NOT_KICK_SCHEDULER;
}