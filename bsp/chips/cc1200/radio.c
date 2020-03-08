/**
\brief CC1200-specific definition of the "radio" bsp module.

\author Jonathan Munoz <jonathan.munoz@inria.fr>, July 2016.
*/

#include "board.h"
#include "radio.h"
#include "cc1200.h"
#include "spi.h"
#include "debugpins.h"
#include "leds.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
    radiotimer_capture_cbt      startFrame_cb;
    radiotimer_capture_cbt      endFrame_cb;
    cc1200_status_t             radioStatusByte;
    radio_state_t               state;
} radio_vars_t;

radio_vars_t radio_vars;

//=========================== public ==========================================

//===== admin

void radio_init(void) {

    // clear variables
    memset(&radio_vars,0,sizeof(radio_vars_t));
   
    // change state
    radio_vars.state          = RADIOSTATE_STOPPED;
   
    // reset radio
    radio_reset();
   
    // change state
    radio_vars.state          = RADIOSTATE_RFOFF;
   
    P1SEL &= (~BIT3); // Set P1.3 SEL as GPIO
    P1DIR &= (~BIT3); // Set P1.3 SEL as Input
    P1IES |= (BIT3); // Falling Edge
    P1IFG &= (~BIT3); // Clear interrupt flag for P1.3
    P1IE |= (BIT3); // Enable interrupt for P1.3
    P1SEL &= (~BIT7); // Set P1.7 SEL as GPIO
    P1DIR &= (~BIT7); // Set P1.7 SEL as Input
    P1IES &= (~BIT7); // Rising Edge //P1IES |= (BIT7); // Falling Edge
    P1IFG &= (~BIT7); // Clear interrupt flag for P1.7
    P1IE |= (BIT7); // Enable interrupt for P1.7
    // Write registers to radio
    for(uint16_t i = 0;
        i < (sizeof(preferredSettings)/sizeof(registerSetting_t)); i++) {
        cc1200_spiWriteReg( preferredSettings[i].addr, &radio_vars.radioStatusByte, preferredSettings[i].data);
    };
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
  
    cc1200_spiStrobe( CC1200_SRES, &radio_vars.radioStatusByte); 
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

void radio_setFrequency(uint8_t frequency, radio_freq_t tx_or_rx) {
    
    // change state
    radio_vars.state = RADIOSTATE_FREQUENCY_SET;
}

void radio_rfOn(void) {
    uint8_t marcState;
    // Calibrate radio
    cc1200_spiStrobe(CC1200_SCAL, &radio_vars.radioStatusByte);
    // Wait for calibration to be done (radio back in IDLE state)
    do {
        cc1200_spiReadReg(CC1200_MARCSTATE, &radio_vars.radioStatusByte, &marcState);
    } while (marcState != 0x41);

}

void radio_rfOff(void) {
   
    // change state
    radio_vars.state = RADIOSTATE_TURNING_OFF;
   
    cc1200_spiStrobe(CC1200_SPWD, &radio_vars.radioStatusByte);
    // poipoipoi wait until off
    cc1200_spiStrobe(CC1200_SXOFF, &radio_vars.radioStatusByte);
   
    // wiggle debug pin
    debugpins_radio_clr();
    leds_radio_off();
   
    // change state
    radio_vars.state = RADIOSTATE_RFOFF;
}

//===== TX

void radio_loadPacket(uint8_t* packet, uint16_t len) {
    //test 802.15.4g PHR. This has to be done by the MAC layer
    uint8_t PHR[2];
    uint8_t aux;    

    if (len<2048){
        PHR[0]      = len/256;
        PHR[1]      = len%256;
        PHR[0]     |= 0x10; //FCS set, size 2 bytes
        PHR[0]     &= 0x77; //no data whitening & no Mode Switch
        *packet     = PHR[0];
        *(packet+1) = PHR[1];
        // change state
        radio_vars.state = RADIOSTATE_LOADING_PACKET;
        cc1200_spiStrobe( CC1200_SFTX, &radio_vars.radioStatusByte);
        cc1200_spiWriteFifo(&radio_vars.radioStatusByte, packet, len, CC1200_FIFO_ADDR);
        cc1200_spiReadReg(CC1200_NUM_TXBYTES, &radio_vars.radioStatusByte, &aux);
        // change state
        radio_vars.state = RADIOSTATE_PACKET_LOADED;
    }else{
    //error
    }
    

}

void radio_txEnable(void) {
    // change state
    radio_vars.state = RADIOSTATE_ENABLING_TX;

    // wiggle debug pin
    debugpins_radio_set();
    leds_radio_on();
      
    // change state
    radio_vars.state = RADIOSTATE_TX_ENABLED;
}

void radio_txNow(void) {
    // change state
    radio_vars.state = RADIOSTATE_TRANSMITTING;
   
    cc1200_spiStrobe( CC1200_STX, &radio_vars.radioStatusByte);
    cc1200_spiStrobe( CC1200_SNOP, &radio_vars.radioStatusByte);
    while(radio_vars.state != RADIOSTATE_TXRX_DONE);
}

//===== RX

void radio_rxEnable(void) {
    uint8_t aux;
    // change state
    radio_vars.state = RADIOSTATE_ENABLING_RX;
   
    //calibrate ROCos
    cc1200_spiReadReg(CC1200_WOR_CFG0, &radio_vars.radioStatusByte, &aux);
    aux = (aux & 0xF9) | (0x02 << 1); 
    // Write new register value
    cc1200_spiWriteReg(CC1200_WOR_CFG0, &radio_vars.radioStatusByte, aux);
    // Strobe IDLE to calibrate the RCOSC
    cc1200_spiStrobe(CC1200_SIDLE, &radio_vars.radioStatusByte);
    // Disable RC calibration
    aux = (aux & 0xF9) | (0x00 << 1);
    cc1200_spiWriteReg(CC1200_WOR_CFG0, &radio_vars.radioStatusByte, aux);
   
    //put radio in reception mode
    cc1200_spiStrobe(CC1200_SWOR, &radio_vars.radioStatusByte); //sniffer mode
   
    // wiggle debug pin
    debugpins_radio_set();
    leds_radio_on();
     
    // change state
    radio_vars.state = RADIOSTATE_LISTENING;
}

void radio_rxNow(void) {
   // nothing to do, the radio is already listening.
}

void radio_getReceivedFrame(
    uint8_t* bufRead,
    uint8_t* lenRead,
    uint8_t  maxBufLen,
    int8_t*  rssi,
    uint8_t* lqi,
    bool*    crc
    ) {
    uint8_t marcStatus1;
    cc1200_spiReadReg( CC1200_MARC_STATUS1, &radio_vars.radioStatusByte, &marcStatus1);
     
    //read FIFO length 
    cc1200_spiReadReg(CC1200_NUM_RXBYTES, &radio_vars.radioStatusByte, lenRead);
    // read the received packet from the RXFIFO
    cc1200_spiReadRxFifo(&radio_vars.radioStatusByte, bufRead, lenRead, maxBufLen);
    
    //TODO
    // On reception, the CC1200 replaces the
    // received CRC by:
    // - [1B] RSSI
    // - [1B] whether CRC checked (bit 7) and LQI (bit 6-0)
    //*rssi  =  *(bufRead+*lenRead-1);
    //*crc   = ((*(bufRead+*lenRead))&0x80)>>7;
    //*lqi   =  (*(bufRead+*lenRead))&0x7f;
    //clean RX FIFO  
    cc1200_spiStrobe( CC1200_SFRX, &radio_vars.radioStatusByte);
    //put radio in reception mode
    cc1200_spiStrobe(CC1200_SWOR, &radio_vars.radioStatusByte);
}

//=========================== private =========================================

//=========================== callbacks =======================================

//=========================== interrupt handlers ==============================
kick_scheduler_t radio_isr(void) {
    PORT_TIMER_WIDTH capturedTime;
    //uint8_t  irq_status;
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
            P1IFG &= ~(BIT3);  //GPIO2 of the radio cc1200 falling edge
            P4OUT ^= BIT0;
            // change state
            radio_vars.state = RADIOSTATE_TXRX_DONE;
            if (radio_vars.endFrame_cb!=NULL) {
                // call the callback
                radio_vars.endFrame_cb(capturedTime);
                // kick the OS
                return KICK_SCHEDULER;
            } else {
            while(1);
            }
            break;
        case 10:
            break;
        case 12:
            break;
        case 14:
            break;
        case 16:
            P1IFG &= ~(BIT7);   //GPIO0 of the radio cc1200 rising edge
            P4OUT ^= BIT2;
            // change state
            radio_vars.state = RADIOSTATE_RECEIVING;
            if (radio_vars.startFrame_cb!=NULL) {
                // call the callback
                radio_vars.startFrame_cb(capturedTime);
                // kick the OS
                return KICK_SCHEDULER;
            } else {
            while(1);
            }
            break;
    }
       
   return DO_NOT_KICK_SCHEDULER;
}
