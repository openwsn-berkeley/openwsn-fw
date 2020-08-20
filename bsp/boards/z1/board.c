/**
\brief Z1-specific definition of the "board" bsp module.

\author Xavier Vilajosana <xvilajosana@eecs.berkeley.edu>, May 2013.
*/

#include "msp430x26x.h"
#include "board.h"
#include "debugpins.h"
// bsp modules
#include "leds.h"
#include "uart.h"
#include "spi.h"
#include "i2c.h"
#include "sctimer.h"
#include "radio.h"
#include "eui64.h"

//#define ISR_BUTTON 1

//=========================== variables =======================================

slot_board_vars_t slot_board_vars [MAX_SLOT_TYPES];
slotType_t selected_slot_type;

//=========================== prototypes ======================================

//=========================== main ============================================

extern int mote_main(void);

int main(void) {
   return mote_main();
}

//=========================== public ==========================================

void board_init(void) {
   
   // disable watchdog timer
   WDTCTL  = WDTPW + WDTHOLD;
   
   // setup clock speed --seems that does not work
   //BCSCTL1 = CALBC1_16MHZ;                       // MCLK at ~16MHz
   //DCOCTL  = CALDCO_16MHZ;                       // MCLK at ~16MHz
   
   if(CALBC1_8MHZ != 0xFF) {
     DCOCTL   = 0x00;
     BCSCTL1  = CALBC1_8MHZ;                     //Set DCO to 8MHz
     DCOCTL   = CALDCO_8MHZ;    
   } else { //start using reasonable values at 8 Mhz
     DCOCTL   = 0x00;
     BCSCTL1  = 0x8D;
     DCOCTL   = 0x88;
   }
   
   // enable flash access violation NMIs
   IE1 |= ACCVIE;
   
    // initialize pins
   P4DIR     |=  0x20;                           // [P4.5] radio VREG:  output
   P4DIR     |=  0x40;                           // [P4.6] radio reset: output
   
   // initialize bsp modules
   debugpins_init();
   leds_init();
   uart_init();
   spi_init();
   radio_init();
   sctimer_init();
   board_init_slot_vars();
   // enable interrupts
   __bis_SR_register(GIE);
}

//====  IEEE802154E timing: bootstrapping slot info lookup table
// 1 clock tick = 30.5 us
void board_init_slot_vars(void){

    // 20ms slot
    slot_board_vars [SLOT_20ms_24GHZ].slotDuration                   = 655 ; // tics  
    slot_board_vars [SLOT_20ms_24GHZ].maxTxDataPrepare               = 110 ; // 3355us (based on measurement)
    slot_board_vars [SLOT_20ms_24GHZ].maxRxAckPrepare                = 20  ; // 610us (based on measurement)
    slot_board_vars [SLOT_20ms_24GHZ].maxRxDataPrepare               = 33  ; // 1000us (based on measurement)
    slot_board_vars [SLOT_20ms_24GHZ].maxTxAckPrepare                = 50  ; // 1525us (based on measurement)
    slot_board_vars [SLOT_20ms_24GHZ].delayTx                        = 18  ; //  366us (measured xxxus)
    slot_board_vars [SLOT_20ms_24GHZ].delayRx                        = 0   ; // 0us (can not measure)
}

// To get the current slotDuration at any time (in tics)
// if you need the value in MS, divide by PORT_TICS_PER_MS (which varies by board and clock frequency and defined in board_info.h)
uint16_t board_getSlotDuration (void){
    return slot_board_vars [selected_slot_type].slotDuration;
}

// Setter/Getter function for slot_board_vars
slot_board_vars_t board_selectSlotTemplate (slotType_t slot_type){
    selected_slot_type = slot_type;
    return slot_board_vars [selected_slot_type];
}

void board_sleep(void) {
   __bis_SR_register(GIE+LPM3_bits);             // sleep, but leave ACLK on
}

void board_reset(void) {
   WDTCTL = (WDTPW+0x1200) + WDTHOLD; // writing a wrong watchdog password to causes handler to reset
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

ISR(DAC12){
   
   debugpins_isr_set();
   while(1); // should never happen
}

ISR(DMA){
   debugpins_isr_set();
   while(1); // should never happen
}

ISR(USCIAB1TX){
   debugpins_isr_set();
   if ( ((UC1IFG & UCB1TXIFG) && (UC1IE & UCB1TXIE)) ||
        ((UC1IFG & UCB1RXIFG) && (UC1IE & UCB1RXIE)) ) {
         isr_i2c_tx(1);                             // I2C: TX
   }
   debugpins_isr_clr();
}

ISR(USCIAB1RX){
   debugpins_isr_set();
   if ( ((UC1IFG & UCB1RXIFG) && (UC1IE & UCB1RXIE)) ||
         (UCB1STAT & UCNACKIFG) ) {
          isr_i2c_rx(1);                             // I2C: RX, bus 1
   }
   debugpins_isr_clr();
}

ISR(ADC12){
   debugpins_isr_set();
   ADC12IFG &= ~0x1F;
   __bic_SR_register_on_exit(CPUOFF);
   debugpins_isr_clr();
}

ISR(USCIAB0TX){
   debugpins_isr_set();
   if ( (UC0IFG & UCA0TXIFG) && (UC0IE & UCA0TXIE) ){
      if (uart_tx_isr()==KICK_SCHEDULER) {       // UART: TX
         __bic_SR_register_on_exit(CPUOFF);
      }
   }
   debugpins_isr_clr();
}

ISR(USCIAB0RX){
   debugpins_isr_set();
   if ( (IFG2 & UCB0RXIFG) && (IE2 & UCB0RXIE) ) {
      if (spi_isr()==KICK_SCHEDULER) {           // SPI
         __bic_SR_register_on_exit(CPUOFF);
      }
   }
   if ( (UC0IFG & UCA0RXIFG) && (UC0IE & UCA0RXIE) ){
      if (uart_rx_isr()==KICK_SCHEDULER) {       // UART: RX
         __bic_SR_register_on_exit(CPUOFF);
      }
   }
   
   debugpins_isr_clr();
}

ISR(TIMERA1){
   debugpins_isr_set();
   while(1); // should never happen
}

ISR(WDT){
   debugpins_isr_set();
   while(1); // should never happen
}

ISR(COMPARATORA){
   debugpins_isr_set();
   __bic_SR_register_on_exit(CPUOFF);            // restart CPU
   debugpins_isr_clr();
}

ISR(TIMERB1){
   debugpins_isr_set();
   if (sctimer_isr()==KICK_SCHEDULER) {       // radiotimer
      __bic_SR_register_on_exit(CPUOFF);
   }
   debugpins_isr_clr();
}

ISR(TIMERA0){
   debugpins_isr_set();
   while(1); // should never happen
}

ISR(TIMERB0){
   debugpins_isr_set();
   while(1); // should never happen
}

ISR(NMI){
   debugpins_isr_set();
   debugpins_frame_set();
   while(1); // should never happen
}
