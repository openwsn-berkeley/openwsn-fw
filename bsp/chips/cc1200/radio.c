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
   radiotimer_capture_cbt    startFrame_cb;
   radiotimer_capture_cbt    endFrame_cb;
   cc1200_status_t radioStatusByte;
   radio_state_t   state;
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
        CC1200_spiWriteReg( preferredSettings[i].addr, &radio_vars.radioStatusByte, preferredSettings[i].data);
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
  
   CC1200_spiStrobe( CC1200_SRES, &radio_vars.radioStatusByte); 
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

void radio_setFrequency(uint8_t frequency) {
    
   // change state
   radio_vars.state = RADIOSTATE_FREQUENCY_SET;
}

void radio_rfOn(void) {
  uint8_t marcState;
  // Calibrate radio
  CC1200_spiStrobe(CC1200_SCAL, &radio_vars.radioStatusByte);
  // Wait for calibration to be done (radio back in IDLE state)
  do {
      CC1200_spiReadReg(CC1200_MARCSTATE, &radio_vars.radioStatusByte, &marcState);
  } while (marcState != 0x41);
   //CC1200_spiStrobe(CC1200_SFSTXON, &radio_vars.radioStatusByte);
   //while (radio_vars.radioStatusByte.chip_rdyn==1) {
    //  CC1200_spiStrobe(CC1200_SNOP, &radio_vars.radioStatusByte);
   //}
  //calibrate ROCos
  CC1200_spiReadReg(CC1200_WOR_CFG0, &radio_vars.radioStatusByte, &marcState);
  marcState = (marcState & 0xF9) | (0x02 << 1); 
  // Write new register value
  CC1200_spiWriteReg(CC1200_WOR_CFG0, &radio_vars.radioStatusByte, marcState);
  // Strobe IDLE to calibrate the RCOSC
  CC1200_spiStrobe(CC1200_SIDLE, &radio_vars.radioStatusByte);
  // Disable RC calibration
  marcState = (marcState & 0xF9) | (0x00 << 1);
  CC1200_spiWriteReg(CC1200_WOR_CFG0, &radio_vars.radioStatusByte, marcState);
}

void radio_rfOff(void) {
   
   // change state
   radio_vars.state = RADIOSTATE_TURNING_OFF;
   
   CC1200_spiStrobe(CC1200_SPWD, &radio_vars.radioStatusByte);
   // poipoipoi wait until off
   CC1200_spiStrobe(CC1200_SXOFF, &radio_vars.radioStatusByte);
   
   // wiggle debug pin
   debugpins_radio_clr();
   leds_radio_off();
   
   // change state
   radio_vars.state = RADIOSTATE_RFOFF;
}

//===== TX

void radio_loadPacket(uint8_t* packet, uint8_t len) {
   // change state
   radio_vars.state = RADIOSTATE_LOADING_PACKET;
   
   CC1200_spiStrobe( CC1200_SFTX, &radio_vars.radioStatusByte);
   CC1200_spiWriteFifo(&radio_vars.radioStatusByte, packet, len, CC1200_FIFO_ADDR);
   
   // change state
   radio_vars.state = RADIOSTATE_PACKET_LOADED;
}

void radio_txEnable(void) {
   // change state
   radio_vars.state = RADIOSTATE_ENABLING_TX;
   
   // wiggle debug pin
   debugpins_radio_set();
   leds_radio_on();
   
   // I don't fully understand how the CC2420_STXCA the can be used here.
   
   // change state
   radio_vars.state = RADIOSTATE_TX_ENABLED;
}

void radio_txNow(void) {
   // change state
   radio_vars.state = RADIOSTATE_TRANSMITTING;
   
   CC1200_spiStrobe( CC1200_STX, &radio_vars.radioStatusByte);
}

//===== RX

void radio_rxEnable(void) {
   // change state
   radio_vars.state = RADIOSTATE_ENABLING_RX;
   
   //put radio in reception mode
   CC1200_spiStrobe(CC1200_SWOR, &radio_vars.radioStatusByte);
   //CC1200_spiStrobe(CC1200_SFRX, &radio_vars.radioStatusByte);
   
   // wiggle debug pin
   debugpins_radio_set();
   leds_radio_on();
   
   // busy wait until radio really listening
   //while (!radio_vars.radioStatusByte.state==1) {
  //    CC1200_spiStrobe(CC1200_SNOP, &radio_vars.radioStatusByte);
  // }
   
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
   //read FIFO length 
   CC1200_spiReadReg(CC1200_NUM_RXBYTES, &radio_vars.radioStatusByte, lenRead);
     // read the received packet from the RXFIFO
   CC1200_spiReadRxFifo(&radio_vars.radioStatusByte, bufRead, lenRead, maxBufLen);
   
   // On reception, when MODEMCTRL0.AUTOCRC is set, the CC2420 replaces the
   // received CRC by:
   // - [1B] the rssi, a signed value. The actual value in dBm is that - 45.
   // - [1B] whether CRC checked (bit 7) and LQI (bit 6-0)
   *rssi  =  *(bufRead+*lenRead-2);
   *rssi -= 45;
   *crc   = ((*(bufRead+*lenRead-1))&0x80)>>7;
   *lqi   =  (*(bufRead+*lenRead-1))&0x7f;
   
   //put radio in reception mode
   CC1200_spiStrobe(CC1200_SWOR, &radio_vars.radioStatusByte);
   
}

//=========================== private =========================================

//=========================== callbacks =======================================

//=========================== interrupt handlers ==============================
kick_scheduler_t radio_isr() {
   PORT_TIMER_WIDTH capturedTime;
   //uint8_t  irq_status;
   // capture the time
   capturedTime = radiotimer_getCapturedTime();
    switch(__even_in_range(P1IV,16))
   {
   case 0:break;
   case 2:break;
   case 4:break;
   case 6:break;
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
   case 10:break;
   case 12:break;
   case 14:break;
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
