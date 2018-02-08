/**
\brief at86rf215-specific definition of the "radio" bsp module.

\author Jonathan Munoz <jonathan.munoz@inria.fr>, July 2016.
*/

#include "board.h"
#include "radio.h"
#include "at86rf215.h"
#include "spi.h"
#include "debugpins.h"
#include "leds.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
    radiotimer_capture_cbt      startFrame_cb;
    radiotimer_capture_cbt      endFrame_cb;
    radio_state_t               state;
    uint8_t                     rf09_isr;
    uint8_t                     rf24_isr;
    uint8_t                     bb0_isr;
    uint8_t                     bb1_isr;    
} radio_vars_t;

radio_vars_t radio_vars;

//=========================== public ==========================================
void radio_read_isr(uint8_t* rf09_isr);
//===== admin

void radio_init(void) {

    // clear variables
    memset(&radio_vars,0,sizeof(radio_vars_t));
   
    // change state
    radio_vars.state          = RADIOSTATE_STOPPED;
   
    // reset radio
    radio_reset();
    at86rf215_spiStrobe(CMD_RF_TRXOFF);
    while(at86rf215_status() != RF_STATE_TRXOFF);
    // change state
    radio_vars.state          = RADIOSTATE_RFOFF;
   
    P1SEL &= (~BIT4); // Set P1.4 SEL as GPIO
    P1DIR &= (~BIT4); // Set P1.4 SEL as Input
    P1IES &= (~BIT4); // low to high edge
    P1IFG &= (~BIT4); // Clear interrupt flag for P1.4
    P1IE |= (BIT4); // Enable interrupt for P1.4
    //check part number and version
    if ((at86rf215_spiReadReg(RG_RF_PN) != 0x34) | (at86rf215_spiReadReg(RG_RF_VN) != 0x03)) {
      while(1); //UNKNOWN DEVICE, FINISH
    }
    // Write registers to radio
    for(uint16_t i = 0; i < (sizeof(basic_settings_fsk_option2)/sizeof(registerSetting_t)); i++) {
        at86rf215_spiWriteReg( basic_settings_fsk_option2[i].addr, basic_settings_fsk_option2[i].data);
    };
    radio_read_isr(&radio_vars.rf09_isr);
}

void radio_change_size(uint16_t* size){
    static int i = 0;
    *size = sizes[i%4];
    i++;
}
void radio_change_modulation(void) {
    static int mod_list = 1;
    at86rf215_spiStrobe(CMD_RF_TRXOFF);
    while(at86rf215_status() != RF_STATE_TRXOFF);
    for(uint16_t i = 0; i < (sizeof(basic_settings_fsk_option1)/sizeof(registerSetting_t)); i++) {
        at86rf215_spiWriteReg( modulation_list[mod_list%5][i].addr, modulation_list[mod_list%5][i].data);
        //at86rf215_spiWriteReg( basic_settings_fsk_option1[i].addr, basic_settings_fsk_option1[i].data);
        };
    radio_read_isr(&radio_vars.rf09_isr);
    mod_list++;
}

void radio_setOverflowCb(radiotimer_compare_cbt cb) {
    radiotimer_setOverflowCb(cb);
}

void radio_setCompareCb(radiotimer_compare_cbt cb) {
    radiotimer_setCompareCb(cb);
}

void radio_setStartFrameCb(radiotimer_capture_cbt cb) {
    radio_vars.startFrame_cb  = cb;
}

void radio_setEndFrameCb(radiotimer_capture_cbt cb) {
    radio_vars.endFrame_cb    = cb;
}

//===== reset

void radio_reset(void) {
   
    at86rf215_spiWriteReg( RG_RF_RST, CMD_RF_RESET); 
}

//===== timer

void radio_startTimer(uint16_t period) {
    radiotimer_start(period);
}

uint16_t radio_getTimerValue(void) {
    return radiotimer_getValue();
}

void radio_setTimerPeriod(uint16_t period) {
    radiotimer_setPeriod(period);
}

uint16_t radio_getTimerPeriod(void) {
    return radiotimer_getPeriod();
}

//===== RF admin
//channel spacing in KHz
//frequency_0 in kHz
//frequency_nb integer
void radio_setFrequency(uint16_t channel_spacing, uint32_t frequency_0, uint16_t channel) {
    
    frequency_0 = (frequency_0/25);
    at86rf215_spiWriteReg(RG_RF09_CS, (uint8_t)(channel_spacing/25));
    at86rf215_spiWriteReg(RG_RF09_CCF0L, (uint8_t)(frequency_0%256));
    at86rf215_spiWriteReg(RG_RF09_CCF0H, (uint8_t)(frequency_0/256));
    at86rf215_spiWriteReg(RG_RF09_CNL, (uint8_t)(channel%256));
    at86rf215_spiWriteReg(RG_RF09_CNM, (uint8_t)(channel/256));
    // change state
    radio_vars.state = RADIOSTATE_FREQUENCY_SET;
    
}

void radio_rfOn(void) {
  
    //put the radio in the TRXPREP state
    at86rf215_spiStrobe(CMD_RF_TRXOFF);
    //while(radio_vars.state != RADIOSTATE_TX_ENABLED);

}

void radio_rfOff(void) {
   
    // change state
    radio_vars.state = RADIOSTATE_TURNING_OFF;
   
    at86rf215_spiStrobe(CMD_RF_TRXOFF);
    // wiggle debug pin
    debugpins_radio_clr();
    leds_radio_off();
   
    // change state
    radio_vars.state = RADIOSTATE_RFOFF;
}

//===== TX

void radio_loadPacket(uint8_t* packet, uint16_t len) {
   
    radio_vars.state = RADIOSTATE_LOADING_PACKET;
    at86rf215_spiWriteFifo(packet, len);
    // change state
    radio_vars.state = RADIOSTATE_PACKET_LOADED;
    //at86rf215_readBurst(0x0306, packet, len);
}

void radio_txEnable(void) {
 
    // change state
    radio_vars.state = RADIOSTATE_ENABLING_TX;
    at86rf215_spiStrobe(CMD_RF_TXPREP);
    // wiggle debug pin
    debugpins_radio_set();
    leds_radio_on();
    at86rf215_spiReadReg(0);
    while(radio_vars.state != RADIOSTATE_TX_ENABLED); 
    // change state
    
}

void radio_txNow(void) {
    // change state
    radio_vars.state = RADIOSTATE_TRANSMITTING;

    at86rf215_spiStrobe(CMD_RF_TX);
    while(radio_vars.state != RADIOSTATE_TXRX_DONE);
    at86rf215_spiStrobe(RF_TRXOFF);
}

//===== RX

void radio_rxEnable(void) {
    // change state
    radio_vars.state = RADIOSTATE_ENABLING_RX; 
    // wiggle debug pin
    debugpins_radio_set();
    leds_radio_on();
    at86rf215_spiStrobe(CMD_RF_RX);
    // change state
    radio_vars.state = RADIOSTATE_LISTENING;
}

void radio_rxNow(void) {
    //nothing to do
}

void radio_getReceivedFrame(
    uint8_t* bufRead,
    uint16_t* lenRead,
    uint16_t  maxBufLen,
    int8_t*  rssi,
    uint8_t* lqi,
    bool*    crc,
    uint8_t* mcs
    ) {
    // read the received packet from the RXFIFO
    at86rf215_spiReadRxFifo(bufRead, lenRead);
    *rssi   = at86rf215_spiReadReg(RG_RF09_EDV);
    *crc    = (at86rf215_spiReadReg(RG_BBC0_PC)>>5);
    *mcs    = (at86rf215_spiReadReg(RG_BBC0_OFDMPHRRX)&OFDMPHRRX_MCS_MASK);
}

//=========================== private ========================================= 

void radio_read_isr(uint8_t* rf09_isr){
    at86rf215_read_isr(rf09_isr);
}
//=========================== callbacks =======================================

//=========================== interrupt handlers ==============================
kick_scheduler_t radio_isr(void) {
    PORT_TIMER_WIDTH capturedTime;
    // capture the time
    capturedTime = radiotimer_getCapturedTime();
    switch(__even_in_range(P1IV,16)){
        case 0:
            break;
        case 2:
            break;
        case 4:
            break;
        case 6:
            break;
        case 8:
            break;
        case 10:
            P1IFG &= ~(BIT4);
            radio_read_isr(&radio_vars.rf09_isr);
            if (radio_vars.rf09_isr & IRQS_TRXRDY_MASK){
                memset(&radio_vars.rf09_isr, 0, 4);
                radio_vars.state = RADIOSTATE_TX_ENABLED;
            }
            
            else if (radio_vars.bb0_isr & IRQS_RXFS_MASK){
                memset(&radio_vars.rf09_isr, 0, 4);
                P4OUT ^= BIT2;
                radio_vars.state = RADIOSTATE_RECEIVING;
                if (radio_vars.startFrame_cb!=NULL) {
                    // call the callback
                    radio_vars.startFrame_cb(capturedTime);
                    // kick the OS
                    return KICK_SCHEDULER;
                } else {
                while(1);
                }
            }
            
            else if ((radio_vars.bb0_isr & IRQS_TXFE_MASK)){ 
                P4OUT ^= BIT0;
                memset(&radio_vars.rf09_isr, 0, 4);
                radio_vars.state = RADIOSTATE_TXRX_DONE;
                if (radio_vars.endFrame_cb!=NULL) {
                    // call the callback
                    radio_vars.endFrame_cb(capturedTime);
                    // kick the OS
                    return KICK_SCHEDULER;
                } else {
                while(1);
                }                
            }            
            else if ((radio_vars.bb0_isr & IRQS_RXFE_MASK)){ 
                P4OUT ^= BIT0;
                memset(&radio_vars.rf09_isr, 0, 4);
                radio_vars.state = RADIOSTATE_TXRX_DONE;
                if (radio_vars.endFrame_cb!=NULL) {
                    // call the callback
                    radio_vars.endFrame_cb(capturedTime);
                    // kick the OS

                    return KICK_SCHEDULER;

                } else {
                while(1);
                }                
            }
            break;
        case 12:
            break;
        case 14:
            break;
        case 16:
            break;
    }
       
   return DO_NOT_KICK_SCHEDULER;
}
