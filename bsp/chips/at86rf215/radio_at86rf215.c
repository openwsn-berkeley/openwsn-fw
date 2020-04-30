/**
\brief at86rf215-specific definition of the "radio" bsp module.

\author Jonathan Munoz <jonathan.munoz@inria.fr>, July 2016.
Modified by Mina Rady <mina1.rady@orange.com>

*/
#include "board.h"

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

#include "radio.h"
#include "radio_at86rf215.h"
#include "at86rf215.h"


#define AT86RF215_IRQ_BASE      ( GPIO_D_BASE )
#define AT86RF215_IRQ_PIN       ( GPIO_PIN_0 )
#define AT86RF215_IRQ_IOC       ( IOC_OVERRIDE_DIS )
#define AT86RF215_IRQ_EDGE      ( GPIO_RISING_EDGE )

//=========================== defines =========================================

#define DEFAULT_CHANNEL_SPACING_FSK_OPTION_1    200     // kHz
#define DEFAULT_CENTER_FREQUENCY_0_FSK_OPTION_1 863125  // Hz


#define DEFAULT_CHANNEL_SPACING_OQPSK_24GHZ    5000    // kHz
#define DEFAULT_CENTER_FREQUENCY_0_OQPSK_24GHZ 850000  // Hz

#ifdef ATMEL_24GHZ
    #define ATMEL_FREQUENCY_TYPE FREQ_24GHZ
#else
    #define ATMEL_FREQUENCY_TYPE FREQ_SUGHZ
#endif

//=========================== variables ==========================================
radio_vars_at86rf215_t radio_vars_at86rf215;

// open radio register mapping: an array of pointers to registersettings arrays

radio_config_t radio_api;
//=========================== public ==========================================

static void radio_read_isr_at86rf215(void);
static void radio_clear_isr_at86rf215(void);

//===== admin

void radio_powerOn_at86rf215(void) {
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

void radio_reset_at86rf215(void) {
    at86rf215_spiWriteReg( RG_RF_RST, CMD_RF_RESET);
}

void radio_init_at86rf215(void) {
    
    //power it on and configure pins
    radio_powerOn_at86rf215();

    spi_init();

    // clear variables
    memset(&radio_vars_at86rf215,0,sizeof(radio_vars_at86rf215_t));

    // change state
    radio_vars_at86rf215.state          = RADIOSTATE_STOPPED;

    // reset radio
    radio_reset_at86rf215();

    at86rf215_spiStrobe(CMD_RF_TRXOFF, ATMEL_FREQUENCY_TYPE);
    while(at86rf215_status(ATMEL_FREQUENCY_TYPE) != RF_STATE_TRXOFF);

    // change state
    radio_vars_at86rf215.state          = RADIOSTATE_RFOFF;

    //configure external radio interrupt in pin D0
    GPIOPinIntDisable(AT86RF215_IRQ_BASE, AT86RF215_IRQ_PIN);

    /* The gpio is an input GPIO on rising edge */
    GPIOPinTypeGPIOInput(AT86RF215_IRQ_BASE, AT86RF215_IRQ_PIN);

    GPIOIntTypeSet(AT86RF215_IRQ_BASE, AT86RF215_IRQ_PIN, GPIO_RISING_EDGE);

    GPIOPinIntClear(AT86RF215_IRQ_BASE, AT86RF215_IRQ_PIN);
    /* Register the interrupt */
    GPIOPortIntRegister(AT86RF215_IRQ_BASE, radio_isr_internal_at86rf215);

    /* Clear and enable the interrupt */
    GPIOPinIntEnable(AT86RF215_IRQ_BASE, AT86RF215_IRQ_PIN);

    //check part number and version
    if ((at86rf215_spiReadReg(RG_RF_PN) != 0x34) | (at86rf215_spiReadReg(RG_RF_VN) != 0x03)) {
        while(1); //UNKNOWN DEVICE, FINISH
    }
    
    //Initialize the lookup table for register configurations
    
    radio_local_bootstrap_at86rf215();
    
    // select the radio configuration to use -- configuration 2-FSK-50kbps
    radio_setConfig_at86rf215 (RADIOSETTING_FSK_OPTION1_FEC);
    radio_read_isr_at86rf215();
}

void radio_change_size_at86rf215(uint16_t* size){
    static int i = 0;
    *size = sizes[i%4];
    i++;
}

// This function accepts one of the existing radio presets and writes them to the radio chip.
void radio_setConfig_at86rf215(radioSetting_t radioSetting){
    uint16_t _register;

    at86rf215_spiStrobe(CMD_RF_TRXOFF, ATMEL_FREQUENCY_TYPE);
    while(at86rf215_status(ATMEL_FREQUENCY_TYPE) != RF_STATE_TRXOFF);

    for( _register = 0; _register < radio_api.size[radioSetting]; _register++) {
        at86rf215_spiWriteReg(radio_api.radios[radioSetting][_register].addr, radio_api.radios[radioSetting][_register].data);
    };
    radio_read_isr_at86rf215();
}

// This function writes a specific set of register values to customize the configuration of the radio chip
void radio_change_modulation_at86rf215(registerSetting_t * mod){
    static int mod_list = 1;
    uint16_t i;

    at86rf215_spiStrobe(CMD_RF_TRXOFF, ATMEL_FREQUENCY_TYPE);
    while(at86rf215_status(ATMEL_FREQUENCY_TYPE) != RF_STATE_TRXOFF);

    for( i = 0; i < (sizeof(*mod)/sizeof(registerSetting_t)); i++) {
        at86rf215_spiWriteReg( mod[i].addr, mod[i].data);
    };
    radio_read_isr_at86rf215();
    mod_list++;
}

void radio_setStartFrameCb_at86rf215(radio_capture_cbt cb) {
    radio_vars_at86rf215.startFrame_cb  = cb;
}

void radio_setEndFrameCb_at86rf215(radio_capture_cbt cb) {
    radio_vars_at86rf215.endFrame_cb    = cb;
}

//===== RF admin
//channel spacing in KHz
//frequency_0 in kHz
//frequency_nb integer
void radio_setFrequency_at86rf215(uint16_t channel, radio_freq_t tx_or_rx) {

    uint16_t frequency_0;

    // frequency has to be updated in TRXOFF status (datatsheet: 6.3.2).
    at86rf215_spiStrobe(CMD_RF_TRXOFF, ATMEL_FREQUENCY_TYPE);
    while(at86rf215_status(ATMEL_FREQUENCY_TYPE) != RF_STATE_TRXOFF);

    if(ATMEL_FREQUENCY_TYPE==0){

        frequency_0 = (DEFAULT_CENTER_FREQUENCY_0_FSK_OPTION_1/25);
        at86rf215_spiWriteReg(RG_RF09_CS, (uint8_t)(DEFAULT_CHANNEL_SPACING_FSK_OPTION_1/25));
        at86rf215_spiWriteReg(RG_RF09_CCF0L, (uint8_t)(frequency_0%256));
        at86rf215_spiWriteReg(RG_RF09_CCF0H, (uint8_t)(frequency_0/256));
        at86rf215_spiWriteReg(RG_RF09_CNL, (uint8_t)(channel%256));
        at86rf215_spiWriteReg(RG_RF09_CNM, (uint8_t)(channel/256));
    } else {
        frequency_0 = (DEFAULT_CENTER_FREQUENCY_0_OQPSK_24GHZ/25);
        at86rf215_spiWriteReg(RG_RF24_CS, (uint8_t)(DEFAULT_CHANNEL_SPACING_OQPSK_24GHZ/25));
        at86rf215_spiWriteReg(RG_RF24_CCF0L, (uint8_t)(frequency_0%256));
        at86rf215_spiWriteReg(RG_RF24_CCF0H, (uint8_t)(frequency_0/256));
        at86rf215_spiWriteReg(RG_RF24_CNL, (uint8_t)(channel%256));
        at86rf215_spiWriteReg(RG_RF24_CNM, (uint8_t)(channel/256));
    }

    // change state
    radio_vars_at86rf215.state = RADIOSTATE_FREQUENCY_SET;
}

void radio_rfOn_at86rf215(void) {
    //put the radio in the TRXPREP state
    at86rf215_spiStrobe(CMD_RF_TRXOFF, ATMEL_FREQUENCY_TYPE);
    while(at86rf215_status(ATMEL_FREQUENCY_TYPE) != RF_STATE_TRXOFF);
}

void radio_rfOff_at86rf215(void) {

    // change state
    radio_vars_at86rf215.state = RADIOSTATE_TURNING_OFF;

    at86rf215_spiStrobe(CMD_RF_TRXOFF, ATMEL_FREQUENCY_TYPE);
    while(at86rf215_status(ATMEL_FREQUENCY_TYPE) != RF_STATE_TRXOFF);
    // wiggle debug pin
    debugpins_radio_clr();
    leds_radio_off();

    // change state
    radio_vars_at86rf215.state = RADIOSTATE_RFOFF;
}

int8_t radio_getFrequencyOffset_at86rf215(void){

    // not available
    return 0;
}

//===== TX

void radio_loadPacket_at86rf215(uint8_t* packet, uint16_t len) {

    radio_vars_at86rf215.state = RADIOSTATE_LOADING_PACKET;
    at86rf215_spiWriteFifo(packet, len, ATMEL_FREQUENCY_TYPE);
    // change state
    radio_vars_at86rf215.state = RADIOSTATE_PACKET_LOADED;
    //at86rf215_readBurst(0x0306, packet, len);
}

radio_state_t radio_getState_at86rf215(void){
    return radio_vars_at86rf215.state;
}

void radio_txEnable_at86rf215(void) {

    // change state
    radio_vars_at86rf215.state = RADIOSTATE_ENABLING_TX;

    at86rf215_spiStrobe(CMD_RF_TXPREP, ATMEL_FREQUENCY_TYPE);
    while(at86rf215_status(ATMEL_FREQUENCY_TYPE) != RF_STATE_TXPREP);

    // wiggle debug pin
    debugpins_radio_set();
    leds_radio_on();

    radio_vars_at86rf215.state = RADIOSTATE_TX_ENABLED;
}

void radio_txNow_at86rf215(void) {

    PORT_TIMER_WIDTH capturedTime;

    // change state
    radio_vars_at86rf215.state = RADIOSTATE_TRANSMITTING;

    at86rf215_spiStrobe(CMD_RF_TX, ATMEL_FREQUENCY_TYPE);

    if (radio_vars_at86rf215.startFrame_cb!=NULL) {
        // capture the time
        capturedTime = sctimer_readCounter();
        // call the callback
        radio_vars_at86rf215.startFrame_cb(capturedTime);
    }
}

//===== RX

void radio_rxEnable_at86rf215(void) {
    // change state
    radio_vars_at86rf215.state = RADIOSTATE_ENABLING_RX;
    // wiggle debug pin
    debugpins_radio_set();
    leds_radio_on();
    at86rf215_spiStrobe(CMD_RF_RX, ATMEL_FREQUENCY_TYPE);

    // change state
    radio_vars_at86rf215.state = RADIOSTATE_LISTENING;
}

void radio_rxNow_at86rf215(void) {
    //nothing to do
    if(at86rf215_status(ATMEL_FREQUENCY_TYPE) != RF_STATE_RX){
        leds_error_toggle();
        return;
    }
}

void radio_getReceivedFrame_at86rf215(
    uint8_t* bufRead,
    uint8_t* lenRead,
    uint8_t  maxBufLen,
    int8_t*  rssi,
    uint8_t* lqi,
    bool*    crc
) {

    uint16_t register_edv;
    uint16_t register_bbc_pc;

    if (ATMEL_FREQUENCY_TYPE==FREQ_SUGHZ){
        // subghz
        register_edv     = RG_RF09_EDV;
        register_bbc_pc  = RG_BBC0_PC;
    } else {
        register_edv     = RG_RF24_EDV;
        register_bbc_pc  = RG_BBC1_PC;
    }

    // read the received packet from the RXFIFO
    at86rf215_spiReadRxFifo(bufRead, (uint16_t*)lenRead, ATMEL_FREQUENCY_TYPE, maxBufLen);

    *rssi   = at86rf215_spiReadReg(register_edv);
    *crc    = (at86rf215_spiReadReg(register_bbc_pc)>>5);
}

//=========================== private =========================================

//This function creates a matrix of register configurations of each modulation supported by the readio chip. It is used for dynamic modulation change inside the chip.
void radio_local_bootstrap_at86rf215(void){
    
    radio_api.radios [RADIOSETTING_FSK_OPTION1_FEC]        = basic_settings_fsk_option1;
    radio_api.radios [RADIOSETTING_OQPSK_RATE3]            = basic_settings_oqpsk_rate3;
    radio_api.radios [RADIOSETTING_OFDM_OPTION_1_MCS0]     = basic_settings_ofdm_1_mcs0;
    radio_api.radios [RADIOSETTING_OFDM_OPTION_1_MCS1]     = basic_settings_ofdm_1_mcs1;
    radio_api.radios [RADIOSETTING_OFDM_OPTION_1_MCS2]     = basic_settings_ofdm_1_mcs2;
    radio_api.radios [RADIOSETTING_OFDM_OPTION_1_MCS3]     = basic_settings_ofdm_1_mcs3;
    
    radio_api.size [RADIOSETTING_FSK_OPTION1_FEC]        = sizeof(basic_settings_fsk_option1)/sizeof(registerSetting_t);
    radio_api.size [RADIOSETTING_OQPSK_RATE3]            = sizeof(basic_settings_oqpsk_rate3)/sizeof(registerSetting_t);
    radio_api.size [RADIOSETTING_OFDM_OPTION_1_MCS0]     = sizeof(basic_settings_ofdm_1_mcs0)/sizeof(registerSetting_t);
    radio_api.size [RADIOSETTING_OFDM_OPTION_1_MCS1]     = sizeof(basic_settings_ofdm_1_mcs1)/sizeof(registerSetting_t);    
    radio_api.size [RADIOSETTING_OFDM_OPTION_1_MCS2]     = sizeof(basic_settings_ofdm_1_mcs2)/sizeof(registerSetting_t);    
    radio_api.size [RADIOSETTING_OFDM_OPTION_1_MCS3]     = sizeof(basic_settings_ofdm_1_mcs3)/sizeof(registerSetting_t);    
}
void radio_read_isr_at86rf215(void){
    uint8_t flags[4];
    at86rf215_read_isr(flags, ATMEL_FREQUENCY_TYPE);

    radio_vars_at86rf215.rf09_isr = flags[0];
    radio_vars_at86rf215.rf24_isr = flags[1];
    radio_vars_at86rf215.bb0_isr = flags[2];
    radio_vars_at86rf215.bb1_isr = flags[3];
}


//=========================== callbacks =======================================

//=========================== interrupt handlers ==============================

// Wrapper function for radio_isr_at86rf215 to be called by the radio interrupt
void radio_isr_internal_at86rf215(void) {
    
    kick_scheduler_t ks = radio_isr_at86rf215();
    
}
kick_scheduler_t radio_isr_at86rf215(void) {

    PORT_TIMER_WIDTH capturedTime;
    kick_scheduler_t result = DO_NOT_KICK_SCHEDULER;

    debugpins_isr_set();

    GPIOPinIntClear(AT86RF215_IRQ_BASE, AT86RF215_IRQ_PIN);

    // capture the time
    capturedTime = sctimer_readCounter();
    //get isr that happened from radio
    radio_read_isr_at86rf215();

    if (radio_vars_at86rf215.bb0_isr & IRQS_RXFS_MASK){
        radio_vars_at86rf215.state = RADIOSTATE_RECEIVING;
        if (radio_vars_at86rf215.startFrame_cb!=NULL) {
            // call the callback
            radio_vars_at86rf215.startFrame_cb(capturedTime);
            // kick the OS
             result = KICK_SCHEDULER;
        } else {
            //while(1);
        }
    } else {
        if ((radio_vars_at86rf215.bb0_isr & IRQS_TXFE_MASK)){
            radio_vars_at86rf215.state = RADIOSTATE_TXRX_DONE;
            if (radio_vars_at86rf215.endFrame_cb!=NULL) {
                // call the callback
                radio_vars_at86rf215.endFrame_cb(capturedTime);
                // kick the OS
                result = KICK_SCHEDULER;
            } else {
                //while(1);
            }
        }  else {
            if ((radio_vars_at86rf215.bb0_isr & IRQS_RXFE_MASK)){
                radio_vars_at86rf215.state = RADIOSTATE_TXRX_DONE;
                if (radio_vars_at86rf215.endFrame_cb!=NULL) {
                    // call the callback
                    radio_vars_at86rf215.endFrame_cb(capturedTime);
                    
                    // kick the OS
                    result = KICK_SCHEDULER;
                } else {
                    // while(1);
                }
            }
        }
    }
    radio_clear_isr_at86rf215();
    debugpins_isr_clr();
    return result;
}

port_INLINE void radio_clear_isr_at86rf215(){
    radio_vars_at86rf215.rf09_isr = 0;
    radio_vars_at86rf215.rf24_isr = 0;
    radio_vars_at86rf215.bb0_isr = 0;
    radio_vars_at86rf215.bb1_isr = 0;
}
