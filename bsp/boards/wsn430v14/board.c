/**
\brief WSN430v14-specific definition of the "board" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "msp430f1611.h"
#include "board.h"
// bsp modules
#include "debugpins.h"
#include "leds.h"
#include "uart.h"
#include "spi.h"
#include "radio.h"
#include "sctimer.h"

//=========================== variables =======================================

slot_board_vars_t slot_board_vars [MAX_SLOT_TYPES];
slotType_t selected_slot_type;

//=========================== prototypes ======================================

kick_scheduler_t sctimer_isr_sfd(void);

//=========================== main ============================================

extern int mote_main(void);

int main(void) {
   return mote_main();
}

//=========================== public ==========================================

void board_init(void) {
   uint8_t delay;
   
   // disable watchdog timer
   WDTCTL     =  WDTPW + WDTHOLD;
   
   //===== clocking
   
   DCOCTL     = 0;                               // we are not using the DCO
   BCSCTL1    = 0;                               // we are not using the DCO
   BCSCTL2    = SELM_2 | (SELS | DIVS_3) ;       // MCLK=XT2, SMCLK=XT2/8
   
   // the MSP detected that the crystal is not running (it's normal, it is
   // starting). It set the OFIFG flag, causing the MSP430 to switch back to
   // the DC0. By software, we need to clear that flag, causing the MSP430 to
   // switch back to using the XT2 as a clocking source, but verify that it
   // stays cleared. This is explained in detail in Section 4.2.6 "Basic Clock
   // Module Fail-Safe Operation" of slau049f, pdf page 119.
   
   do {
      IFG1   &= ~OFIFG;                          // clear OSCFault flag
      for (delay=0;delay<0xff;delay++) {         // busy wait for at least 50us
          __no_operation();
      }
   } while ((IFG1 & OFIFG) != 0);                // repeat until OSCFault flag stays cleared
   
   //===== pins
   
   P3DIR     |=  0x01;                           // [P3.0] radio VREG:  output
   P1DIR     |=  0x80;                           // [P1.7] radio reset: output
   P1DIR     &= ~0x20;                           // [P1.5] radio SFD:   input
   P1IES     &= ~0x20;                           // [P1.5] radio SFD:   low->high
   P1IFG     &= ~0x20;                           // [P1.5] radio SFD:   clear interrupt flag
   P1IE      |=  0x20;                           // [P1.5] radio SFD:   interrupt enabled
   
   //===== bsp modules
   
   debugpins_init();
   leds_init();
   uart_init();
   spi_init();
   radio_init();
   sctimer_init();
   board_init_slot_vars();

   //===== enable interrupts
   
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
    
    #ifdef OPENWSN_IEEE802154E_SECURITY_C
        slot_board_vars [SLOT_20ms_24GHZ].delayTx                    = 7   ; //  214us (measured xxxus)
    #else
        slot_board_vars [SLOT_20ms_24GHZ].delayTx                    = 18  ; //  366us (measured xxxus)
    #endif
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
   __bis_SR_register(GIE+LPM0_bits);             // sleep, but leave ACLK on
}

void board_reset(void) {
   WDTCTL = (WDTPW+0x1200) + WDTHOLD;            // writing a wrong watchdog password to causes handler to reset
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

// DACDMA_VECTOR

// PORT2_VECTOR

ISR(USART0TX){
   debugpins_isr_set();
   if (uart_tx_isr()==KICK_SCHEDULER) {          // UART; TX
      __bic_SR_register_on_exit(CPUOFF);
   }
   debugpins_isr_clr();
}

ISR(USART0RX) {
   debugpins_isr_set();
   if (uart_rx_isr()==KICK_SCHEDULER) {          // UART: RX
      __bic_SR_register_on_exit(CPUOFF);
   }
   debugpins_isr_clr();
}

ISR(PORT1) {
   debugpins_isr_set();
   if (P1IFG & 0x20) {
      P1IFG &= ~0x20;
      if (sctimer_isr_sfd()==KICK_SCHEDULER){ // radio:  SFD pin [P1.6]
         __bic_SR_register_on_exit(CPUOFF);
      }
   } else {
      while (1); // should never happen
   }
   debugpins_isr_clr();
}

// TIMERA0_VECTOR

// TIMERA1_VECTOR

// ADC12_VECTOR

// USART0TX_VECTOR

/*
ISR(USART0RX) {
   debugpins_isr_set();
   if (spi_isr()==KICK_SCHEDULER) {              // SPI
      __bic_SR_register_on_exit(CPUOFF);
   }
   debugpins_isr_clr();
}
*/

// WDT_VECTOR

ISR(COMPARATORA) {
   debugpins_isr_set();
   __bic_SR_register_on_exit(CPUOFF);            // restart CPU
   debugpins_isr_clr();
}

// TIMERB0_VECTOR

ISR(TIMERB1) {
   debugpins_isr_set();
   if (sctimer_isr()==KICK_SCHEDULER) {          // sctimer
      __bic_SR_register_on_exit(CPUOFF);
   }
   debugpins_isr_clr();
}

// NMI_VECTOR

