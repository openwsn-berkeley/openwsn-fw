/**
\brief at86rf215-specific definition of the "radio" bsp module.

\author Jonathan Munoz <jonathan.munoz@inria.fr>, July 2016.
*/

#include "board.h"
#include "radio.h"

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

#define DEFAULT_CHANNEL_SPACING_FSK_OPTION_1    200     // kHz
#define DEFAULT_CENTER_FREQUENCY_0_FSK_OPTION_1 863125  // Hz


#define DEFAULT_CHANNEL_SPACING_OQPSK_24GHZ    5000    // kHz
#define DEFAULT_CENTER_FREQUENCY_0_OQPSK_24GHZ 850000  // Hz

#ifdef ATMEL_24GHZ
    #define ATMEL_FREQUENCY_TYPE FREQ_24GHZ
#else
    #define ATMEL_FREQUENCY_TYPE FREQ_SUGHZ
#endif

//=========================== variables =======================================

typedef struct {
    radio_capture_cbt           startFrame_cb;
    radio_capture_cbt           endFrame_cb;
    radio_state_t               state;
    uint8_t                     rf09_isr;
    uint8_t                     rf24_isr;
    uint8_t                     bb0_isr;
    uint8_t                     bb1_isr;
} radio_vars_t;

radio_vars_t radio_vars;

//=========================== public ==========================================

static void radio_read_isr(void);
static void radio_clear_isr(void);

//===== admin

void radio_powerOn(void) {
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

void radio_reset(void) {
    at86rf215_spiWriteReg( RG_RF_RST, CMD_RF_RESET);
}

void radio_init(void) {

    uint16_t i;
    //power it on and configure pins
    radio_powerOn();

    spi_init();

    // clear variables
    memset(&radio_vars,0,sizeof(radio_vars_t));

    // change state
    radio_vars.state          = RADIOSTATE_STOPPED;

    // reset radio
    radio_reset();

    at86rf215_spiStrobe(CMD_RF_TRXOFF, ATMEL_FREQUENCY_TYPE);
    while(at86rf215_status(ATMEL_FREQUENCY_TYPE) != RF_STATE_TRXOFF);

    // change state
    radio_vars.state          = RADIOSTATE_RFOFF;

    //configure external radio interrupt in pin D0
    GPIOPinIntDisable(AT86RF215_IRQ_BASE, AT86RF215_IRQ_PIN);

    /* The gpio is an input GPIO on rising edge */
    GPIOPinTypeGPIOInput(AT86RF215_IRQ_BASE, AT86RF215_IRQ_PIN);

    GPIOIntTypeSet(AT86RF215_IRQ_BASE, AT86RF215_IRQ_PIN, GPIO_RISING_EDGE);

    GPIOPinIntClear(AT86RF215_IRQ_BASE, AT86RF215_IRQ_PIN);
    /* Register the interrupt */
    GPIOPortIntRegister(AT86RF215_IRQ_BASE, radio_isr);

    /* Clear and enable the interrupt */
    GPIOPinIntEnable(AT86RF215_IRQ_BASE, AT86RF215_IRQ_PIN);

    //check part number and version
    if ((at86rf215_spiReadReg(RG_RF_PN) != 0x34) | (at86rf215_spiReadReg(RG_RF_VN) != 0x03)) {
        while(1); //UNKNOWN DEVICE, FINISH
    }
    // Write registers to radio -- configuration 2-FSK-50kbps
    if (ATMEL_FREQUENCY_TYPE==FREQ_SUGHZ){
        for( i = 0; i < (sizeof(basic_settings_fsk_option1)/sizeof(registerSetting_t)); i++) {
            at86rf215_spiWriteReg( basic_settings_fsk_option1[i].addr, basic_settings_fsk_option1[i].data);
        };
    } else {
        for( i = 0; i < (sizeof(basic_settings_oqpsk_250kbps)/sizeof(registerSetting_t)); i++) {
            at86rf215_spiWriteReg( basic_settings_oqpsk_250kbps[i].addr, basic_settings_oqpsk_250kbps[i].data);
        };
    }


    radio_read_isr();
}

void radio_change_size(uint16_t* size){
    static int i = 0;
    *size = sizes[i%4];
    i++;
}

void radio_change_modulation(registerSetting_t * mod){
    static int mod_list = 1;
    uint16_t i;

    at86rf215_spiStrobe(CMD_RF_TRXOFF, ATMEL_FREQUENCY_TYPE);
    while(at86rf215_status(ATMEL_FREQUENCY_TYPE) != RF_STATE_TRXOFF);

    for( i = 0; i < (sizeof(*mod)/sizeof(registerSetting_t)); i++) {
        at86rf215_spiWriteReg( mod[i].addr, mod[i].data);
    };
    radio_read_isr();
    mod_list++;
}

void radio_setStartFrameCb(radio_capture_cbt cb) {
    radio_vars.startFrame_cb  = cb;
}

void radio_setEndFrameCb(radio_capture_cbt cb) {
    radio_vars.endFrame_cb    = cb;
}

//===== RF admin
//channel spacing in KHz
//frequency_0 in kHz
//frequency_nb integer
void radio_setFrequency(uint16_t channel, radio_freq_t tx_or_rx) {

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
    radio_vars.state = RADIOSTATE_FREQUENCY_SET;
}

void radio_rfOn(void) {
    //put the radio in the TRXPREP state
    at86rf215_spiStrobe(CMD_RF_TRXOFF, ATMEL_FREQUENCY_TYPE);
    while(at86rf215_status(ATMEL_FREQUENCY_TYPE) != RF_STATE_TRXOFF);
}

void radio_rfOff(void) {

    // change state
    radio_vars.state = RADIOSTATE_TURNING_OFF;

    at86rf215_spiStrobe(CMD_RF_TRXOFF, ATMEL_FREQUENCY_TYPE);
    while(at86rf215_status(ATMEL_FREQUENCY_TYPE) != RF_STATE_TRXOFF);
    // wiggle debug pin
    debugpins_radio_clr();
    leds_radio_off();

    // change state
    radio_vars.state = RADIOSTATE_RFOFF;
}

int8_t radio_getFrequencyOffset(void){

    // not available
    return 0;
}

//===== TX

void radio_loadPacket(uint8_t* packet, uint16_t len) {

    radio_vars.state = RADIOSTATE_LOADING_PACKET;
    at86rf215_spiWriteFifo(packet, len, ATMEL_FREQUENCY_TYPE);
    // change state
    radio_vars.state = RADIOSTATE_PACKET_LOADED;
    //at86rf215_readBurst(0x0306, packet, len);
}

radio_state_t radio_getState(void){
    return radio_vars.state;
}

void radio_txEnable(void) {

    // change state
    radio_vars.state = RADIOSTATE_ENABLING_TX;

    at86rf215_spiStrobe(CMD_RF_TXPREP, ATMEL_FREQUENCY_TYPE);
    while(at86rf215_status(ATMEL_FREQUENCY_TYPE) != RF_STATE_TXPREP);

    // wiggle debug pin
    debugpins_radio_set();
    leds_radio_on();

    radio_vars.state = RADIOSTATE_TX_ENABLED;
}

void radio_txNow(void) {

    PORT_TIMER_WIDTH capturedTime;

    // change state
    radio_vars.state = RADIOSTATE_TRANSMITTING;

    at86rf215_spiStrobe(CMD_RF_TX, ATMEL_FREQUENCY_TYPE);

    if (radio_vars.startFrame_cb!=NULL) {
        // capture the time
        capturedTime = sctimer_readCounter();
        // call the callback
        radio_vars.startFrame_cb(capturedTime);
    }
}

//===== RX

void radio_rxEnable(void) {
    // change state
    radio_vars.state = RADIOSTATE_ENABLING_RX;
    // wiggle debug pin
    debugpins_radio_set();
    leds_radio_on();
    at86rf215_spiStrobe(CMD_RF_RX, ATMEL_FREQUENCY_TYPE);

    // change state
    radio_vars.state = RADIOSTATE_LISTENING;
}

void radio_rxNow(void) {
    //nothing to do
    if(at86rf215_status(ATMEL_FREQUENCY_TYPE) != RF_STATE_RX){
        leds_error_toggle();
        return;
    }
}

void radio_getReceivedFrame(
    uint8_t* bufRead,
    uint16_t* lenRead,
    uint16_t  maxBufLen,
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
    at86rf215_spiReadRxFifo(bufRead, lenRead, ATMEL_FREQUENCY_TYPE, maxBufLen);

    *rssi   = at86rf215_spiReadReg(register_edv);
    *crc    = (at86rf215_spiReadReg(register_bbc_pc)>>5);
}

//=========================== private =========================================

void radio_read_isr(void){
    uint8_t flags[4];
    at86rf215_read_isr(flags, ATMEL_FREQUENCY_TYPE);

    radio_vars.rf09_isr = flags[0];
    radio_vars.rf24_isr = flags[1];
    radio_vars.bb0_isr = flags[2];
    radio_vars.bb1_isr = flags[3];
}

//=========================== callbacks =======================================

//=========================== interrupt handlers ==============================
void radio_isr(void) {

    PORT_TIMER_WIDTH capturedTime;
    // kick_scheduler_t result = DO_NOT_KICK_SCHEDULER;

    debugpins_isr_set();

    GPIOPinIntClear(AT86RF215_IRQ_BASE, AT86RF215_IRQ_PIN);

    // capture the time
    capturedTime = sctimer_readCounter();
    //get isr that happened from radio
    radio_read_isr();

    if (radio_vars.bb0_isr & IRQS_RXFS_MASK){
        radio_vars.state = RADIOSTATE_RECEIVING;
        if (radio_vars.startFrame_cb!=NULL) {
            // call the callback
            radio_vars.startFrame_cb(capturedTime);
            // kick the OS
            // result = KICK_SCHEDULER;
        } else {
            //while(1);
        }
    } else {
        if ((radio_vars.bb0_isr & IRQS_TXFE_MASK)){
            radio_vars.state = RADIOSTATE_TXRX_DONE;
            if (radio_vars.endFrame_cb!=NULL) {
                // call the callback
                radio_vars.endFrame_cb(capturedTime);
                // kick the OS
                // result = KICK_SCHEDULER;
            } else {
                //while(1);
            }
        }  else {
            if ((radio_vars.bb0_isr & IRQS_RXFE_MASK)){
                radio_vars.state = RADIOSTATE_TXRX_DONE;
                if (radio_vars.endFrame_cb!=NULL) {
                    // call the callback
                    radio_vars.endFrame_cb(capturedTime);
                    // kick the OS
                    //result = KICK_SCHEDULER;
                } else {
                    // while(1);
                }
            }
        }
    }
    radio_clear_isr();
    debugpins_isr_clr();
}

port_INLINE void radio_clear_isr(){
    radio_vars.rf09_isr = 0;
    radio_vars.rf24_isr = 0;
    radio_vars.bb0_isr = 0;
    radio_vars.bb1_isr = 0;
}
