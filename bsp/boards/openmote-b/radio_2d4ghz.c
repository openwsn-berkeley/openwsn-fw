/**
 * Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Date:   July 2013
 * Description: CC2538-specific definition of the "radio" bsp module.
 */

#include <stdio.h>
#include <string.h>

#include <headers/hw_ana_regs.h>
#include <headers/hw_ints.h>
#include <headers/hw_rfcore_sfr.h>
#include <headers/hw_rfcore_sfr.h>
#include <headers/hw_rfcore_xreg.h>
#include <headers/hw_types.h>

#include <source/interrupt.h>
#include <source/sys_ctrl.h>

#include "board.h"
#include "cc2538rf.h"
#include "debugpins.h"
#include "leds.h"
#include "radio_2d4ghz.h"
#include "sctimer.h"

//=========================== defines =========================================

/* Bit Masks for the last byte in the RX FIFO */
#define CRC_BIT_MASK 0x80
#define LQI_BIT_MASK 0x7F

/* RSSI Offset */
#define RSSI_OFFSET 73
#define CHECKSUM_LEN 2

//=========================== variables =======================================

typedef struct {
    radio_capture_cbt         startFrame_cb;
    radio_capture_cbt         endFrame_cb;
    radio_state_t             state;
} radio_2d4ghz_vars_t;

radio_2d4ghz_vars_t radio_2d4ghz_vars;

//=========================== prototypes ======================================

void     enable_radio_2d4ghz_interrupts(void);
void     disable_radio_2d4ghz_interrupts(void);

void     radio_2d4ghz_on(void);
void     radio_2d4ghz_off(void);

void     radio_2d4ghz_error_isr(void);
void     radio_2d4ghz_isr_internal(void);

//=========================== public ==========================================

//===== admin

void radio_2d4ghz_setFunctions(radio_functions_t* funcs){
    funcs->radio_powerOn_cb            = radio_2d4ghz_powerOn;
    // RF admin
    funcs->radio_init_cb               = radio_2d4ghz_init;
    funcs->radio_setStartFrameCb_cb    = radio_2d4ghz_setStartFrameCb;
    funcs->radio_setEndFrameCb_cb      = radio_2d4ghz_setEndFrameCb;
    // RF admin
    funcs->radio_rfOn_cb               = radio_2d4ghz_rfOn;
    funcs->radio_rfOff_cb              = radio_2d4ghz_rfOff;
    funcs->radio_setFrequency_cb       = radio_2d4ghz_setFrequency;
    funcs->radio_change_modulation_cb  = radio_2d4ghz_change_modulation;
    // reset
    funcs->radio_reset_cb              = radio_2d4ghz_reset;
    // TX
    funcs->radio_loadPacket_prepare_cb = radio_2d4ghz_loadPacket_prepare;
    funcs->radio_txEnable_cb           = radio_2d4ghz_txEnable;
    funcs->radio_txNow_cb              = radio_2d4ghz_txNow;
    funcs->radio_loadPacket_cb         = radio_2d4ghz_loadPacket;
    // RX
    funcs->radio_rxPacket_prepare_cb   = radio_2d4ghz_rxPacket_prepare;
    funcs->radio_rxEnable_cb           = radio_2d4ghz_rxEnable;
    funcs->radio_rxEnable_scum_cb      = radio_2d4ghz_rxEnable_scum;
    funcs->radio_rxNow_cb              = radio_2d4ghz_rxNow;
    funcs->radio_getReceivedFrame_cb   = radio_2d4ghz_getReceivedFrame;
    funcs->radio_getCRCLen_cb          = radio_2d4ghz_getCRCLen;
    funcs->radio_calculateFrequency_cb = radio_2d4ghz_calculateFrequency;
    funcs->radio_getDelayTx_cb         = radio_2d4ghz_getDelayTx;
    funcs->radio_getDelayRx_cb         = radio_2d4ghz_getDelayRx;
}

void radio_2d4ghz_init() {

    // clear variables
    memset(&radio_2d4ghz_vars,0,sizeof(radio_2d4ghz_vars_t));

    // change state
    radio_2d4ghz_vars.state          = RADIOSTATE_STOPPED;
    //flush fifos
    CC2538_RF_CSP_ISFLUSHRX();
    CC2538_RF_CSP_ISFLUSHTX();

    radio_2d4ghz_off();

    //disable radio interrupts
    disable_radio_2d4ghz_interrupts();

    /*
    This CORR_THR value should be changed to 0x14 before attempting RX. Testing has shown that
    too many false frames are received if the reset value is used. Make it more likely to detect
    sync by removing the requirement that both symbols in the SFD must have a correlation value
    above the correlation threshold, and make sync word detection less likely by raising the
    correlation threshold.
    */
    HWREG(RFCORE_XREG_MDMCTRL1)    = 0x14;
    /* tuning adjustments for optimal radio performance; details available in datasheet */

    HWREG(RFCORE_XREG_RXCTRL)      = 0x3F;
    /* Adjust current in synthesizer; details available in datasheet. */
    HWREG(RFCORE_XREG_FSCTRL)      = 0x55;

     /* Makes sync word detection less likely by requiring two zero symbols before the sync word.
      * details available in datasheet.
      */
    HWREG(RFCORE_XREG_MDMCTRL0)    = 0x85;

    /* Adjust current in VCO; details available in datasheet. */
    HWREG(RFCORE_XREG_FSCAL1)      = 0x01;
    /* Adjust target value for AGC control loop; details available in datasheet. */
    HWREG(RFCORE_XREG_AGCCTRL1)    = 0x15;

    /* Tune ADC performance, details available in datasheet. */
    HWREG(RFCORE_XREG_ADCTEST0)    = 0x10;
    HWREG(RFCORE_XREG_ADCTEST1)    = 0x0E;
    HWREG(RFCORE_XREG_ADCTEST2)    = 0x03;

    //update CCA register to -81db as indicated by manual.. won't be used..
    HWREG(RFCORE_XREG_CCACTRL0)    = 0xF8;
    /*
    * Changes from default values
    * See User Guide, section "Register Settings Update"
    */
    HWREG(RFCORE_XREG_TXFILTCFG)   = 0x09;    /** TX anti-aliasing filter bandwidth */
    HWREG(RFCORE_XREG_AGCCTRL1)    = 0x15;     /** AGC target value */
    HWREG(ANA_REGS_O_IVCTRL)       = 0x0B;        /** Bias currents */

    /* disable the CSPT register compare function */
    HWREG(RFCORE_XREG_CSPT)        = 0xFFUL;
    /*
    * Defaults:
    * Auto CRC; Append RSSI, CRC-OK and Corr. Val.; CRC calculation;
    * RX and TX modes with FIFOs
    */
    HWREG(RFCORE_XREG_FRMCTRL0)    = RFCORE_XREG_FRMCTRL0_AUTOCRC;

    //poipoi disable frame filtering by now.. sniffer mode.
    HWREG(RFCORE_XREG_FRMFILT0)   &= ~RFCORE_XREG_FRMFILT0_FRAME_FILTER_EN;

    /* Disable source address matching and autopend */
    HWREG(RFCORE_XREG_SRCMATCH)    = 0;

    /* MAX FIFOP threshold */
    HWREG(RFCORE_XREG_FIFOPCTRL)   = CC2538_RF_MAX_PACKET_LEN;

    HWREG(RFCORE_XREG_TXPOWER)     = CC2538_RF_TX_POWER;
    HWREG(RFCORE_XREG_FREQCTRL)    = CC2538_RF_CHANNEL_MIN;

    /* Enable RF interrupts  see page 751  */
    // enable_radio_2d4ghz_interrupts();

    //register interrupt
    IntRegister(INT_RFCORERTX, radio_2d4ghz_isr_internal);
    IntRegister(INT_RFCOREERR, radio_2d4ghz_error_isr);

    IntEnable(INT_RFCORERTX);

    /* Enable all RF Error interrupts */
    HWREG(RFCORE_XREG_RFERRM)      = RFCORE_XREG_RFERRM_RFERRM_M; //all errors
    IntEnable(INT_RFCOREERR);
    //radio_2d4ghz_on();

    // change state
    radio_2d4ghz_vars.state               = RADIOSTATE_RFOFF;
}

void radio_2d4ghz_setStartFrameCb(radio_capture_cbt cb) {
    radio_2d4ghz_vars.startFrame_cb  = cb;
}

void radio_2d4ghz_setEndFrameCb(radio_capture_cbt cb) {
    radio_2d4ghz_vars.endFrame_cb    = cb;
}

//===== reset

void radio_2d4ghz_reset() {
    /* Wait for ongoing TX to complete (e.g. this could be an outgoing ACK) */
    while(HWREG(RFCORE_XREG_FSMSTAT1) & RFCORE_XREG_FSMSTAT1_TX_ACTIVE);

    //flush fifos
    CC2538_RF_CSP_ISFLUSHRX();
    CC2538_RF_CSP_ISFLUSHTX();

    /* Don't turn off if we are off as this will trigger a Strobe Error */
    if(HWREG(RFCORE_XREG_RXENABLE) != 0) {
        CC2538_RF_CSP_ISRFOFF();
    }
    radio_2d4ghz_init();
}

//===== RF admin

void radio_2d4ghz_setFrequency(uint16_t channel_spacing, uint32_t frequency_0, uint16_t channel) {

    // change state
    radio_2d4ghz_vars.state = RADIOSTATE_SETTING_FREQUENCY;

    radio_2d4ghz_off();
    // configure the radio to the right frequecy
    if((channel < CC2538_RF_CHANNEL_MIN) || (channel > CC2538_RF_CHANNEL_MAX)) {
        while(1);
    }

    /* Changes to FREQCTRL take effect after the next recalibration */
    HWREG(RFCORE_XREG_FREQCTRL) = (CC2538_RF_CHANNEL_MIN
        + (channel - CC2538_RF_CHANNEL_MIN) * CC2538_RF_CHANNEL_SPACING);

    //radio_2d4ghz_on();

    // change state
    radio_2d4ghz_vars.state = RADIOSTATE_FREQUENCY_SET;
}

void radio_2d4ghz_rfOn() {
    //radio_2d4ghz_on();
}

void radio_2d4ghz_rfOff() {

    // change state
    radio_2d4ghz_vars.state = RADIOSTATE_TURNING_OFF;
    radio_2d4ghz_off();
    // wiggle debug pin
    debugpins_radio_clr();
    leds_radio_off();
    //enable radio interrupts
    disable_radio_2d4ghz_interrupts();

    // change state
   radio_2d4ghz_vars.state = RADIOSTATE_RFOFF;
}

//===== TX

void radio_2d4ghz_loadPacket(uint8_t* packet, uint16_t len) {
    uint8_t i=0;

    // change state
    radio_2d4ghz_vars.state = RADIOSTATE_LOADING_PACKET;

    // load packet in TXFIFO
    /*
    When we transmit in very quick bursts, make sure previous transmission
    is not still in progress before re-writing to the TX FIFO
    */
    while(HWREG(RFCORE_XREG_FSMSTAT1) & RFCORE_XREG_FSMSTAT1_TX_ACTIVE);

    CC2538_RF_CSP_ISFLUSHTX();

    /* Send the phy length byte first */
    HWREG(RFCORE_SFR_RFDATA) = len; //crc len is included

    for(i = 0; i < len; i++) {
      HWREG(RFCORE_SFR_RFDATA) = packet[i];
    }

    // change state
    radio_2d4ghz_vars.state = RADIOSTATE_PACKET_LOADED;
}

void radio_2d4ghz_txEnable() {

    // change state
    radio_2d4ghz_vars.state = RADIOSTATE_ENABLING_TX;

    // wiggle debug pin
    debugpins_radio_set();
    leds_radio_on();

    //do nothing -- radio is activated by the strobe on rx or tx
    //radio_2d4ghz_rfOn();

    // change state
    radio_2d4ghz_vars.state = RADIOSTATE_TX_ENABLED;
}

void radio_2d4ghz_txNow() {

    PORT_TIMER_WIDTH count;

    // change state
    radio_2d4ghz_vars.state = RADIOSTATE_TRANSMITTING;

    //enable radio interrupts
    enable_radio_2d4ghz_interrupts();

    //make sure we are not transmitting already
    while(HWREG(RFCORE_XREG_FSMSTAT1) & RFCORE_XREG_FSMSTAT1_TX_ACTIVE);

    // send packet by STON strobe see pag 669

    CC2538_RF_CSP_ISTXON();
    //wait 192uS
    count=0;
    while(!((HWREG(RFCORE_XREG_FSMSTAT1) & RFCORE_XREG_FSMSTAT1_TX_ACTIVE))){
        count++; //debug
    }
}

//===== RX

void radio_2d4ghz_rxEnable() {

    // change state
    radio_2d4ghz_vars.state = RADIOSTATE_ENABLING_RX;

    //enable radio interrupts

    // do nothing as we do not want to receive anything yet.
    // wiggle debug pin
    debugpins_radio_set();
    leds_radio_on();

    // change state
    radio_2d4ghz_vars.state = RADIOSTATE_LISTENING;
}

void radio_2d4ghz_rxNow() {
    //empty buffer before receiving
    //CC2538_RF_CSP_ISFLUSHRX();

    //enable radio interrupts
    CC2538_RF_CSP_ISFLUSHRX();
    enable_radio_2d4ghz_interrupts();

    CC2538_RF_CSP_ISRXON();
    // busy wait until radio really listening
    while(!((HWREG(RFCORE_XREG_FSMSTAT1) & RFCORE_XREG_FSMSTAT1_RX_ACTIVE)));
}


void radio_2d4ghz_getReceivedFrame(
    uint8_t* pBufRead,
    uint16_t* pLenRead,
    uint16_t  maxBufLen,
    int8_t* pRssi,
    uint8_t* pLqi,
    bool* pCrc,
    uint8_t*  mcs
) {
    uint8_t crc_corr,i;

    uint8_t len=0;

    /* Check the length */
    len = HWREG(RFCORE_SFR_RFDATA); //first byte is len


    /* Check for validity */
    if(len > CC2538_RF_MAX_PACKET_LEN) {
        /* wrong len */
        CC2538_RF_CSP_ISFLUSHRX();
        return;
    }


    if(len <= CC2538_RF_MIN_PACKET_LEN) {
        //too short
        CC2538_RF_CSP_ISFLUSHRX();
        return;
    }

    //check if this fits to the buffer
    if(len > maxBufLen) {
        CC2538_RF_CSP_ISFLUSHRX();
        return;
    }

    // when reading the packet from the RX buffer, you get the following:
    // - *[1B]     length byte
    // -  [0-125B] packet (excluding CRC)
    // -  [1B]     RSSI
    // - *[2B]     CRC

    //skip first byte is len
    for(i = 0; i < len - 2; i++) {
        pBufRead[i] = HWREG(RFCORE_SFR_RFDATA);
    }

    *pRssi     = ((int8_t)(HWREG(RFCORE_SFR_RFDATA)) - RSSI_OFFSET);
    crc_corr   = HWREG(RFCORE_SFR_RFDATA);
    *pCrc      = crc_corr & CRC_BIT_MASK;
    *pLenRead  = len;

    //flush it
    CC2538_RF_CSP_ISFLUSHRX();
}

//=========================== private =========================================

void enable_radio_2d4ghz_interrupts(void){
    /* Enable RF interrupts 0, RXPKTDONE,SFD,FIFOP only -- see page 751  */
    HWREG(RFCORE_XREG_RFIRQM0) |= ((0x06|0x02|0x01) << RFCORE_XREG_RFIRQM0_RFIRQM_S) & RFCORE_XREG_RFIRQM0_RFIRQM_M;

    /* Enable RF interrupts 1, TXDONE only */
    HWREG(RFCORE_XREG_RFIRQM1) |= ((0x02) << RFCORE_XREG_RFIRQM1_RFIRQM_S) & RFCORE_XREG_RFIRQM1_RFIRQM_M;
}

void disable_radio_2d4ghz_interrupts(void){
    /* Enable RF interrupts 0, RXPKTDONE,SFD,FIFOP only -- see page 751  */
    HWREG(RFCORE_XREG_RFIRQM0) = 0;
    /* Enable RF interrupts 1, TXDONE only */
    HWREG(RFCORE_XREG_RFIRQM1) = 0;
}

void radio_2d4ghz_on(void){
    // CC2538_RF_CSP_ISFLUSHRX();
    CC2538_RF_CSP_ISRXON();
}

void radio_2d4ghz_off(void){
    /* Wait for ongoing TX to complete (e.g. this could be an outgoing ACK) */
    while(HWREG(RFCORE_XREG_FSMSTAT1) & RFCORE_XREG_FSMSTAT1_TX_ACTIVE);
    //CC2538_RF_CSP_ISFLUSHRX();

    /* Don't turn off if we are off as this will trigger a Strobe Error */
    if(HWREG(RFCORE_XREG_RXENABLE) != 0) {
        CC2538_RF_CSP_ISRFOFF();
        //clear fifo isr flag
        HWREG(RFCORE_SFR_RFIRQF0) = ~(RFCORE_SFR_RFIRQF0_FIFOP|RFCORE_SFR_RFIRQF0_RXPKTDONE);
    }
}

//returns the crc len for this radio
uint8_t  radio_2d4ghz_getCRCLen(void){
    return LENGTH_CRC;
}

uint8_t radio_2d4ghz_getDelayTx(void){
    return delayTx_2D4GHZ;
}

uint8_t radio_2d4ghz_getDelayRx(void){
    return delayRx_2D4GHZ;
}

//=========================== callbacks =======================================

//=========================== interrupt handlers ==============================

/**
\brief Stub function for the CC2538.

In MSP430 platforms the CPU status after servicing an interrupt can be managed
toggling some bits in a special register, e.g. CPUOFF, LPM1, etc, within the
interrupt context itself. By default, after servicing an interrupt the CPU will
be off so it makes sense to return a value and enable it if something has
happened that needs the scheduler to run (a packet has been received that needs
to be processed). Otherwise, the CPU is kept in sleep mode without even
reaching the main loop.

In the CC2538, however, the default behaviour is the contrary. After servicing
an interrupt the CPU will be on by default and it is the responsability of the
main thread to put it back to sleep (which is already done). This means that
the scheduler will always be kicked in after servicing an interrupt. This
behaviour can be changed by modifying the SLEEPEXIT field in the SYSCTRL
regiser (see page 131 of the CC2538 manual).
*/
kick_scheduler_t radio_2d4ghz_isr() {
    return DO_NOT_KICK_SCHEDULER;
}

void radio_2d4ghz_isr_internal(void) {
    volatile PORT_TIMER_WIDTH capturedTime;
    uint8_t  irq_status0,irq_status1;

    debugpins_isr_set();

    // capture the time
    capturedTime = sctimer_readCounter();

    // reading IRQ_STATUS
    irq_status0 = HWREG(RFCORE_SFR_RFIRQF0);
    irq_status1 = HWREG(RFCORE_SFR_RFIRQF1);

    IntPendClear(INT_RFCORERTX);

    //clear interrupt
    HWREG(RFCORE_SFR_RFIRQF0) = 0;
    HWREG(RFCORE_SFR_RFIRQF1) = 0;

    //STATUS0 Register
    // start of frame event
    if ((irq_status0 & RFCORE_SFR_RFIRQF0_SFD) == RFCORE_SFR_RFIRQF0_SFD) {
        // change state
        radio_2d4ghz_vars.state = RADIOSTATE_RECEIVING;
        if (radio_2d4ghz_vars.startFrame_cb!=NULL) {
            // call the callback
            radio_2d4ghz_vars.startFrame_cb(capturedTime);
            debugpins_isr_clr();
            // kick the OS
            return;
        } else {
            while(1);
        }
    }

    //or RXDONE is full -- we have a packet.
    if (((irq_status0 & RFCORE_SFR_RFIRQF0_RXPKTDONE) ==  RFCORE_SFR_RFIRQF0_RXPKTDONE)) {
        // change state
        radio_2d4ghz_vars.state = RADIOSTATE_TXRX_DONE;
        if (radio_2d4ghz_vars.endFrame_cb!=NULL) {
            // call the callback
            radio_2d4ghz_vars.endFrame_cb(capturedTime);
            debugpins_isr_clr();
            // kick the OS
            return;
        } else {
            while(1);
        }
    }

    // or FIFOP is full -- we have a packet.
    if (((irq_status0 & RFCORE_SFR_RFIRQF0_FIFOP) ==  RFCORE_SFR_RFIRQF0_FIFOP)) {
        // change state
        radio_2d4ghz_vars.state = RADIOSTATE_TXRX_DONE;
        if (radio_2d4ghz_vars.endFrame_cb!=NULL) {
            // call the callback
            radio_2d4ghz_vars.endFrame_cb(capturedTime);
            debugpins_isr_clr();
            // kick the OS
            return;
        } else {
            while(1);
        }
    }

    //STATUS1 Register
    // end of frame event --either end of tx .
    if (((irq_status1 & RFCORE_SFR_RFIRQF1_TXDONE) == RFCORE_SFR_RFIRQF1_TXDONE)) {
        // change state
        radio_2d4ghz_vars.state = RADIOSTATE_TXRX_DONE;
        if (radio_2d4ghz_vars.endFrame_cb!=NULL) {
            // call the callback
            radio_2d4ghz_vars.endFrame_cb(capturedTime);
            debugpins_isr_clr();
            // kick the OS
            return;
        } else {
            while(1);
        }
    }
    debugpins_isr_clr();

    return;
}

void radio_2d4ghz_error_isr(void){

    uint8_t rferrm;

    rferrm = (uint8_t)HWREG(RFCORE_XREG_RFERRM);

    if ((HWREG(RFCORE_XREG_RFERRM) & (((0x02)<<RFCORE_XREG_RFERRM_RFERRM_S)&RFCORE_XREG_RFERRM_RFERRM_M)) & ((uint32_t)rferrm)) {
        HWREG(RFCORE_XREG_RFERRM) = ~(((0x02)<<RFCORE_XREG_RFERRM_RFERRM_S)&RFCORE_XREG_RFERRM_RFERRM_M);
        //poipoi  -- todo handle error
    }
}

uint8_t radio_2d4ghz_calculateFrequency(uint8_t channelOffset, uint8_t asnOffset, uint8_t numChannels, uint8_t* hopSeq, bool singleChannel){

    if (singleChannel) {
        return channelOffset; // single channel
    } else {
        // channel hopping enabled, use the channel depending on hopping template
        return 11 + hopSeq[(asnOffset+channelOffset)%numChannels];
    }
}

// not used by 2.4ghz radio
void  radio_2d4ghz_powerOn(void){}
void  radio_2d4ghz_change_modulation(registerSetting_t * mod){}
void  radio_2d4ghz_change_size(uint16_t* size){}
void  radio_2d4ghz_loadPacket_prepare(uint8_t* packet, uint8_t len){}
void  radio_2d4ghz_rxPacket_prepare(void){}
void  radio_2d4ghz_rxEnable_scum(void){}
