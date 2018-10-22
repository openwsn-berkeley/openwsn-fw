/**
\brief at86rf215-specific definition of the "radio" bsp module.

\author Jonathan Munoz <jonathan.munoz@inria.fr>, July 2016.
*/

#include "board.h"
#include "radio_subghz.h"

#include "at86rf215.h"
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

#define AT86RF215_IRQ_BASE      ( GPIO_D_BASE )
#define AT86RF215_IRQ_PIN       ( GPIO_PIN_0 )
#define AT86RF215_IRQ_IOC       ( IOC_OVERRIDE_DIS )
#define AT86RF215_IRQ_EDGE      ( GPIO_RISING_EDGE )


//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
    radio_capture_cbt           startFrame_cb;
    radio_capture_cbt           endFrame_cb;
    radio_state_t               state;
    uint8_t                     rf09_isr;
    uint8_t                     rf24_isr;
    uint8_t                     bb0_isr;
    uint8_t                     bb1_isr;
    phy_tsch_config_t           phy_tsch_config_current;
    phy_tsch_config_t           phy_list[MAX_NUM_RADIOS];
    //uint8_t                     delayTx;
    //uint16_t                    channel_spacing;
    //uint32_t                    center_freq_0;
    //uint16_t                    num_channels;
    //uint16_t                    chTemplate;
} radio_subghz_vars_t;

radio_subghz_vars_t radio_subghz_vars;

phy_tsch_config_t              phy_tsch_config_2fsk_50_subGHz;
phy_tsch_config_t              phy_tsch_config_ofdm_1_800_subGHz;

//=========================== public ==========================================
static void radio_subghz_read_isr(void);
static void radio_subghz_clear_isr(void);
void config_ofdm_1_800_subGHz(void);
void config_2fsk_50_subGHz(void);

//isr handler for the radio
//static void radio_subghz_isr(void);

//===== admin

void  radio_subghz_setFunctions(radio_functions_t * funcs) {
    funcs->radio_powerOn_cb            = radio_subghz_powerOn;
    // RF admin
    funcs->radio_init_cb               = radio_subghz_init;
    funcs->radio_setStartFrameCb_cb    = radio_subghz_setStartFrameCb;
    funcs->radio_setEndFrameCb_cb      = radio_subghz_setEndFrameCb;
    // RF admin
    funcs->radio_rfOn_cb               = radio_subghz_rfOn;
    funcs->radio_rfOff_cb              = radio_subghz_rfOff;
    funcs->radio_setFrequency_cb       = radio_subghz_setFrequency;
    funcs->radio_load_phy_cb           = radio_subghz_load_phy;
    // reset
    funcs->radio_reset_cb              = radio_subghz_reset;
    // TX
    funcs->radio_loadPacket_prepare_cb = radio_subghz_loadPacket_prepare;
    funcs->radio_txEnable_cb           = radio_subghz_txEnable;
    funcs->radio_txNow_cb              = radio_subghz_txNow;
    funcs->radio_loadPacket_cb         = radio_subghz_loadPacket;
    // RX
    funcs->radio_rxPacket_prepare_cb   = radio_subghz_rxPacket_prepare;
    funcs->radio_rxEnable_cb           = radio_subghz_rxEnable;
    funcs->radio_rxEnable_scum_cb      = radio_subghz_rxEnable_scum;
    funcs->radio_rxNow_cb              = radio_subghz_rxNow;
    funcs->radio_getReceivedFrame_cb   = radio_subghz_getReceivedFrame;
    funcs->radio_getCRCLen_cb          = radio_subghz_getCRCLen;
    funcs->radio_calculateFrequency_cb = radio_subghz_calculateFrequency;
    funcs->radio_getDelayTx_cb         = radio_subghz_getDelayTx;
    funcs->radio_getDelayRx_cb         = radio_subghz_getDelayRx;
    funcs->radio_getChInitOffset_cb    = radio_subghz_getChInitOffset;
    funcs->radio_getCh_spacing_cb      = radio_subghz_getCh_spacing_cb;
    funcs->radio_getNumOfChannels_cb   = radio_subghz_getNumOfChannels_cb;
    funcs->radio_getCenterFreq_cb      = radio_subghz_getCenterFreq_cb;
}

void radio_subghz_powerOn(void) {
    volatile uint32_t delay;

    GPIOPinTypeGPIOOutput(GPIO_C_BASE, GPIO_PIN_0);
    GPIOPinTypeGPIOOutput(GPIO_D_BASE, GPIO_PIN_1);

    //set radio pwr off
    GPIOPinWrite(GPIO_C_BASE, GPIO_PIN_0, 0);
    GPIOPinWrite(GPIO_D_BASE, GPIO_PIN_1, 0);
    for(delay=0;delay<0xA2C2;delay++);

    //init the radio, pwr up the radio
    GPIOPinWrite(GPIO_C_BASE, GPIO_PIN_0, GPIO_PIN_0);
    for(delay=0;delay<0xA2C2;delay++);

    //reset the radio
    GPIOPinWrite(GPIO_D_BASE, GPIO_PIN_1, GPIO_PIN_1);

}

//===== reset
void radio_subghz_reset(void) {
    at86rf215_spiWriteReg( RG_RF_RST, CMD_RF_RESET);
}

void radio_subghz_init(void) {
    uint16_t i;
    //power it on and configure pins
    radio_subghz_powerOn();

    // clear variables
    memset(&radio_subghz_vars,0,sizeof(radio_subghz_vars_t));

    // change state
    radio_subghz_vars.state          = RADIOSTATE_STOPPED;

    // reset radio
    radio_subghz_reset();

    at86rf215_spiStrobe(CMD_RF_TRXOFF);
    while(at86rf215_status() != RF_STATE_TRXOFF);

    // change state
    radio_subghz_vars.state          = RADIOSTATE_RFOFF;

    //configure external radio interrupt in pin D0
    GPIOPinIntDisable(AT86RF215_IRQ_BASE, AT86RF215_IRQ_PIN);

    /* The gpio is an input GPIO on rising edge */
    GPIOPinTypeGPIOInput(AT86RF215_IRQ_BASE, AT86RF215_IRQ_PIN);

    GPIOIntTypeSet(AT86RF215_IRQ_BASE, AT86RF215_IRQ_PIN, GPIO_RISING_EDGE);

    GPIOPinIntClear(AT86RF215_IRQ_BASE, AT86RF215_IRQ_PIN);
    /* Register the interrupt */
    GPIOPortIntRegister(AT86RF215_IRQ_BASE, radio_subghz_isr);

    /* Clear and enable the interrupt */
    GPIOPinIntEnable(AT86RF215_IRQ_BASE, AT86RF215_IRQ_PIN);

    //check part number and version
    if ((at86rf215_spiReadReg(RG_RF_PN) != 0x34) | (at86rf215_spiReadReg(RG_RF_VN) != 0x03)) {
        while(1); //UNKNOWN DEVICE, FINISH
    }
    // Write registers to radio -- default configuration 2fsk-50
    for( i = 0; i < (sizeof(basic_settings_fsk_option1_subghz)/sizeof(registerSetting_t)); i++) {
        at86rf215_spiWriteReg( basic_settings_fsk_option1_subghz[i].addr, basic_settings_fsk_option1_subghz[i].data);
    };
    radio_subghz_read_isr();
    config_ofdm_1_800_subGHz();
    config_2fsk_50_subGHz();
    radio_subghz_vars.phy_list[0]           = phy_tsch_config_2fsk_50_subGHz;
    radio_subghz_vars.phy_list[1]           = phy_tsch_config_ofdm_1_800_subGHz;
    radio_subghz_vars.phy_list[2]           = phy_tsch_config_2fsk_50_subGHz;
}

//void radio_subghz_load_phy(registerSetting_t * mod, uint8_t size, uint8_t phy_index){
void radio_subghz_load_phy(uint8_t phy_index){
    uint8_t i;
    at86rf215_spiStrobe(CMD_RF_TRXOFF);
    while(at86rf215_status() != RF_STATE_TRXOFF);

//    for( i = 0; i < size; i++) {
//        at86rf215_spiWriteReg( mod[i].addr, mod[i].data);
//    };
    for (i=0; i < radio_subghz_vars.phy_list[phy_index].size_config; i++){
        at86rf215_spiWriteReg(radio_subghz_vars.phy_list[phy_index].phy_conf[i].addr, radio_subghz_vars.phy_list[phy_index].phy_conf[i].data);
    };
    
    radio_subghz_read_isr();
    //radio_subghz_vars.delayTx = radio_subghz_vars.phy_list[phy_index].delayTX;
    radio_subghz_vars.phy_tsch_config_current = radio_subghz_vars.phy_list[phy_index];   
    // radio_subghz_setFrequency(radio_subghz_vars.phy_list[phy_index].channel_spacing, radio_subghz_vars.phy_list[phy_index].center_freq_0, radio_subghz_vars.phy_list[phy_index])
    
}

void radio_subghz_setStartFrameCb(radio_capture_cbt cb) {
    radio_subghz_vars.startFrame_cb  = cb;
}

void radio_subghz_setEndFrameCb(radio_capture_cbt cb) {
    radio_subghz_vars.endFrame_cb    = cb;
}

//===== RF admin
//channel spacing in KHz
//frequency_0 in kHz
//frequency_nb integer
void radio_subghz_setFrequency(uint16_t channel_spacing, uint32_t frequency_0, uint16_t channel) {

    frequency_0 = (frequency_0/25);
    at86rf215_spiWriteReg(RG_RF09_CS, (uint8_t)(channel_spacing/25));
    at86rf215_spiWriteReg(RG_RF09_CCF0L, (uint8_t)(frequency_0%256));
    at86rf215_spiWriteReg(RG_RF09_CCF0H, (uint8_t)(frequency_0/256));
    at86rf215_spiWriteReg(RG_RF09_CNL, (uint8_t)(channel%256));
    at86rf215_spiWriteReg(RG_RF09_CNM, (uint8_t)(channel/256));
    // change state
    radio_subghz_vars.state = RADIOSTATE_FREQUENCY_SET;

}

void radio_subghz_rfOn(void) {
    //put the radio in the TRXPREP state
    at86rf215_spiStrobe(CMD_RF_TRXOFF);
    //while(radio_subghz_vars.state != RADIOSTATE_TX_ENABLED);
}

void radio_subghz_rfOff(void) {

    // change state
    radio_subghz_vars.state = RADIOSTATE_TURNING_OFF;

    at86rf215_spiStrobe(CMD_RF_TRXOFF);
    // wiggle debug pin
    debugpins_radio_clr();
    leds_debug_off();

    // change state
    radio_subghz_vars.state = RADIOSTATE_RFOFF;
}

//===== TX

void radio_subghz_loadPacket(uint8_t* packet, uint16_t len) {

    radio_subghz_vars.state = RADIOSTATE_LOADING_PACKET;
    at86rf215_spiWriteFifo(packet, len);
    // change state
    radio_subghz_vars.state = RADIOSTATE_PACKET_LOADED;
    //at86rf215_readBurst(0x0306, packet, len);
    
}

radio_state_t radio_subghz_getState(void){
    return radio_subghz_vars.state;
}

void radio_subghz_txEnable(void) {

    // change state
    radio_subghz_vars.state = RADIOSTATE_ENABLING_TX;
    at86rf215_spiStrobe(CMD_RF_TXPREP);
    // wiggle debug pin
    debugpins_radio_set();
    leds_debug_on();
}

void radio_subghz_txNow(void) {

    PORT_TIMER_WIDTH capturedTime;

    // check radio state transit to TRX PREPARE
    if (radio_subghz_vars.state != RADIOSTATE_TX_ENABLED){
        // return directly
        return;
    }

    // change state
    radio_subghz_vars.state = RADIOSTATE_TRANSMITTING;

    at86rf215_spiStrobe(CMD_RF_TX);

    if (radio_subghz_vars.startFrame_cb!=NULL) {
        // capture the time
        capturedTime = sctimer_readCounter();
        // call the callback
        radio_subghz_vars.startFrame_cb(capturedTime);
    }
}

//===== RX

void radio_subghz_rxEnable(void) {
    // change state
    radio_subghz_vars.state = RADIOSTATE_ENABLING_RX;
    // wiggle debug pin
    debugpins_radio_set();
    leds_debug_on();
    at86rf215_spiStrobe(CMD_RF_RX);

    // change state
    radio_subghz_vars.state = RADIOSTATE_LISTENING;
}

void radio_subghz_rxNow(void) {
    //nothing to do
    if(at86rf215_status() != RF_STATE_RX){
        leds_error_toggle();
        return;
    }


}

void radio_subghz_getReceivedFrame(
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

//returns the crc len for this radio
uint8_t  radio_subghz_getCRCLen(void){
    return LENGTH_CRC_SUBGHZ;
}

uint8_t radio_subghz_getDelayTx(void){
    //return delayTx_SUBGHZ;
    return radio_subghz_vars.phy_tsch_config_current.delayTX;
}

uint8_t radio_subghz_getDelayRx(void){
    //return delayRx_SUBGHZ;
    return radio_subghz_vars.phy_tsch_config_current.delayRX;
}

uint8_t radio_subghz_getChInitOffset(void){
    //return ChInitOffset;
    return radio_subghz_vars.phy_tsch_config_current.chInitOffset;
}

uint16_t radio_subghz_getNumOfChannels_cb(void){
    return radio_subghz_vars.phy_tsch_config_current.num_channels;
}

uint16_t radio_subghz_getCh_spacing_cb(void){
    return radio_subghz_vars.phy_tsch_config_current.channel_spacing;
}

uint32_t radio_subghz_getCenterFreq_cb(void){
    return radio_subghz_vars.phy_tsch_config_current.center_freq_0;
}
//=========================== private =========================================

void radio_subghz_read_isr(){
    uint8_t flags[4];
    at86rf215_read_isr(flags);

    radio_subghz_vars.rf09_isr = flags[0];
    radio_subghz_vars.rf24_isr = flags[1];
    radio_subghz_vars.bb0_isr = flags[2];
    radio_subghz_vars.bb1_isr = flags[3];
}
//=========================== callbacks =======================================

//=========================== interrupt handlers ==============================
void radio_subghz_isr() {

    PORT_TIMER_WIDTH capturedTime;
    // kick_scheduler_t result = DO_NOT_KICK_SCHEDULER;

    debugpins_isr_set();

    GPIOPinIntClear(AT86RF215_IRQ_BASE, AT86RF215_IRQ_PIN);

    // capture the time
    capturedTime = sctimer_readCounter();
    //get isr that happened from radio
    radio_subghz_read_isr();

    if (radio_subghz_vars.rf09_isr & IRQS_TRXRDY_MASK){
        radio_subghz_vars.state = RADIOSTATE_TX_ENABLED;
        // result = DO_NOT_KICK_SCHEDULER;
    }

    if (radio_subghz_vars.bb0_isr & IRQS_RXFS_MASK){
        radio_subghz_vars.state = RADIOSTATE_RECEIVING;
        if (radio_subghz_vars.startFrame_cb!=NULL) {
            // call the callback
            radio_subghz_vars.startFrame_cb(capturedTime);
            // kick the OS
            // result = KICK_SCHEDULER;
        } else {
            //while(1);
        }
    } else {
        if ((radio_subghz_vars.bb0_isr & IRQS_TXFE_MASK)){
            radio_subghz_vars.state = RADIOSTATE_TXRX_DONE;
            if (radio_subghz_vars.endFrame_cb!=NULL) {
                // call the callback
                radio_subghz_vars.endFrame_cb(capturedTime);
                // kick the OS
                // result = KICK_SCHEDULER;
            } else {
                //while(1);
            }
        }  else {
            if ((radio_subghz_vars.bb0_isr & IRQS_RXFE_MASK)){
                radio_subghz_vars.state = RADIOSTATE_TXRX_DONE;
                if (radio_subghz_vars.endFrame_cb!=NULL) {
                    // call the callback
                    radio_subghz_vars.endFrame_cb(capturedTime);
                    // kick the OS
                    //result = KICK_SCHEDULER;
                } else {
                    // while(1);
                }
            }
        }
    }
    radio_subghz_clear_isr();
    debugpins_isr_clr();
}

port_INLINE void radio_subghz_clear_isr(){
    radio_subghz_vars.rf09_isr = 0;
    radio_subghz_vars.rf24_isr = 0;
    radio_subghz_vars.bb0_isr = 0;
    radio_subghz_vars.bb1_isr = 0;
}


uint8_t radio_subghz_calculateFrequency(uint8_t channelOffset, uint8_t asnOffset, uint8_t numChannels, uint8_t* hopSeq, bool singleChannel){
    if (singleChannel) {
        return 0; // single channel
    } else {
        // channel hopping enabled, use the channel depending on hopping template modulo the available channels for this platform.
        return hopSeq[(asnOffset+channelOffset)%numChannels] % NUM_CHANNELS_SUBGHZ;
    }
}

void config_2fsk_50_subGHz(){
    phy_tsch_config_2fsk_50_subGHz.phy_conf        = basic_settings_fsk_option1_subghz;
    phy_tsch_config_2fsk_50_subGHz.channel_spacing = 200;
    phy_tsch_config_2fsk_50_subGHz.center_freq_0   = 863125;
    phy_tsch_config_2fsk_50_subGHz.size_config     = (sizeof(basic_settings_fsk_option1_subghz)/sizeof(registerSetting_t));
    phy_tsch_config_2fsk_50_subGHz.delayTX         = delayTx_2FSK_50;
    phy_tsch_config_2fsk_50_subGHz.delayRX         = delayRx_SUBGHZ;
    phy_tsch_config_2fsk_50_subGHz.chTemplate      = chTemplate_default_2fsk50[0]; //
    phy_tsch_config_2fsk_50_subGHz.num_channels    = 34;
    phy_tsch_config_2fsk_50_subGHz.chInitOffset    = 0;
}

void config_ofdm_1_800_subGHz(){

    phy_tsch_config_ofdm_1_800_subGHz.phy_conf        = basic_settings_ofdm_1_mcs3_subghz;
    phy_tsch_config_ofdm_1_800_subGHz.channel_spacing = 1200;
    phy_tsch_config_ofdm_1_800_subGHz.center_freq_0   = 863625;
    phy_tsch_config_ofdm_1_800_subGHz.size_config     = (sizeof(basic_settings_ofdm_1_mcs3_subghz)/sizeof(registerSetting_t));
    phy_tsch_config_ofdm_1_800_subGHz.delayTX         = delayTx_OFDM1;
    phy_tsch_config_ofdm_1_800_subGHz.delayRX         = delayRx_SUBGHZ;
    phy_tsch_config_ofdm_1_800_subGHz.chTemplate      = chTemplate_default_OFDM1[0];
    phy_tsch_config_ofdm_1_800_subGHz.num_channels    = 5;
    phy_tsch_config_ofdm_1_800_subGHz.chInitOffset    = 0;
}

// ==== not used for subghz radio
void radio_subghz_loadPacket_prepare(uint8_t* packet, uint8_t len){}
void radio_subghz_rxPacket_prepare(void){}
void radio_subghz_rxEnable_scum(void){}
