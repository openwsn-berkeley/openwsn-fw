/**
\brief MoteISTv5-specific definition of the "board" bsp module.

\author Diogo Guerra <diogoguerra@ist.utl.pt>, <dy090.guerra@gmail.com>, July 2015.
*/

#include "hal_MoteISTv5.h"
#include "board.h"
// bsp modules
#include "debugpins.h"
#include "leds.h"
#include "uart.h"
#include "spi.h"
#include "bsp_timer.h"
#include "radio.h"
#include "radiotimer.h"
#include "opendefs.h"

//=========================== definitions =====================================

#ifndef  configCPU_CLOCK_HZ
#define configCPU_CLOCK_HZ    (25000000UL)
#endif
#ifndef  configLFXT_CLOCK_HZ
#define configLFXT_CLOCK_HZ   ( 32768L )
#endif

#define SFD       0x02  // P1.1
#define RST       0x40  // P9.6
#define VREG      0x80  // P9.7

//=========================== variables =======================================

//=========================== prototypes ======================================

void hal430SetSystemClock(unsigned long req_clock_rate, unsigned long ref_clock_rate);
void activate_switches(void);
kick_scheduler_t radiotimer_isr_sfd(void);

//=========================== main ============================================

extern int mote_main(void);

int main(void) {
   return mote_main();
}

//=========================== public ==========================================

void board_init(void) {

   //===== disable watchdog timer
   WDTCTL = WDTPW + WDTHOLD;

   //===== pull-down all ports
   PASEL = 0;
   PADIR = 0;
   PAREN = 0xFFFF;
   PAOUT = 0;

   PBSEL = 0; //P5SEL |= 0x0C;              // Port select XT2
   PBDIR = 0;
   PBREN = 0xFFFF;   //PBREN = 0xFFF3
   PBOUT = 0;

   PCSEL = 0;
   PCDIR = 0;
   PCREN = 0xFFFF;
   PCOUT = 0;

   PDSEL = 0x0003; // P7SEL0,1 -> XIN e XOUT (cristal pins)
   PDDIR = 0;
   PDREN = 0xFFFC;
   PDOUT = 0;

   PESEL = 0;
   PEDIR = 0;
   PEREN = 0xFFFF;
   PEOUT = 0;

   PFSEL = 0; // only 8 bit port..
   PFDIR = 0;
   PFREN = 0xFF;
   PFOUT = 0;

   //===== clocking

   ///TODO: Create a I2C and make a driver controler for the Switch
   activate_switches(); //MSP430 starts at1MHz default, so we can call here SwitchFunction
   LFXT_Start( XT1DRIVE_0 );
   hal430SetSystemClock( configCPU_CLOCK_HZ, configLFXT_CLOCK_HZ );

   //===== serial debug

   P9SEL |= (BIT5 | BIT4);
   UCA2CTL1 |= UCSWRST;        // **Put state machine in reset**
   UCA2CTL1 |= UCSSEL_2;       // SMCLK
   UCA2BR0 = 0x0C;             // 25MHz 115200 (see User's Guide) 2MegaBaud
   UCA2BR1 = 0x00;             // 25MHz 115200
   UCA2MCTL = 0xAA; // Modulation UCBRSx=1, UCBRFx=0
   UCA2CTL1 &= ~UCSWRST;       // **Initialize USCI state machine**

   //===== pins

   P9DIR     |=  VREG;        // [P9.7] radio VREG:  output
   P9DIR     |=  RST;         // [P9.6] radio reset: output

   //===== bsp modules

   debugpins_init();
   leds_init();
   uart_init();
   spi_init();
   bsp_timer_init();
   radiotimer_init();
   radio_init();

   //===== enable interrupts

   __bis_SR_register(GIE);
}

void board_sleep(void) {
   __bis_SR_register(GIE+LPM0_bits);             // CPU sleep
}

void board_reset(void) {
   WDTCTL = (WDTPW+0x1200) + WDTHOLD;            // writing a wrong watchdog password causes handler to reset
}

//=========================== private =========================================

/**********************************************************************//**
 * @brief  Set function for MCLK frequency.
 *
 *
 * @return none
 *************************************************************************/
void hal430SetSystemClock(unsigned long req_clock_rate, unsigned long ref_clock_rate)
{
   /* Convert a Hz value to a KHz value, as required
    *  by the Init_FLL_Settle() function. */
   unsigned long ulCPU_Clock_KHz = req_clock_rate / 1000UL;

   //Make sure we aren't overclocking
   if(ulCPU_Clock_KHz > 25000L)
   {
      ulCPU_Clock_KHz = 25000L;
   }

   //Set VCore to a level sufficient for the requested clock speed.
   if(ulCPU_Clock_KHz <= 8000L)
   {
      SetVCore(PMMCOREV_0);
   }
   else if(ulCPU_Clock_KHz <= 12000L)
   {
      SetVCore(PMMCOREV_1);
   }
   else if(ulCPU_Clock_KHz <= 20000L)
   {
      SetVCore(PMMCOREV_2);
   }
   else
   {
      SetVCore(PMMCOREV_3);
   }

   //Set the DCO
   Init_FLL_Settle( ( unsigned short )ulCPU_Clock_KHz, req_clock_rate / ref_clock_rate );
}

/**********************************************************************//**
 * @brief  Open All CB's Electronic Switches.
 *
 *
 * @return none
 *************************************************************************/
void activate_switches(void){
   P3SEL |= 0x80;                            // P3.7 - SDA - Assign I2C pins
   P5SEL |= 0x10;                            // P5.4 - SCL

   UCB1CTL1 |= UCSWRST;                      // Enable SW reset

   UCB1CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
   UCB1CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK
   UCB1BR0 = 144;                            // fSCL = SMCLK/12 = ~100kHz
   UCB1BR1 = 0;
   UCB1I2CSA = 0x48;                         // Slave Address is 090h 1001 0000 -> address (7bit, A0=A1=0) + R/!W

   UCB1CTL1 &= ~UCSWRST;                     // Clear SW reset*/

   while (UCB1CTL1 & UCTXSTP);               // Ensure stop condition got sent
    UCB1CTL1 |= UCTR + UCTXSTT;              // I2C start condition
   UCB1TXBUF = 0xFF;                         // Send init data
   while(UCB1CTL1 & UCTXSTT);                // Start condition sent?
   UCB1CTL1 |= UCTXSTP;                      // I2C1 Stop Condition*/
}


//=========================== interrupt handlers ==============================

// DACDMA_VECTOR

// PORT2_VECTOR


// USCI_A0_VECTOR
ISR(USCI_A0){
//void __attribute__((__interrupt__ (USCI_A0_VECTOR))) __MSPGCC_MAYBE_C16__ (USCI_A0_ISR)(void){
   uint16_t op = UCA0IV;

   switch(op) { //switch(__even_in_range(UCA0IV,4))
      case 0:                                   // Vector 0 - no interrupt
         break;
      case 2:                                   // Vector 2 - RXIFG
         debugpins_isr_set();
         if (uart_rx_isr()==KICK_SCHEDULER) {          // UART: RX
               __bic_SR_register_on_exit(CPUOFF);
         }
         debugpins_isr_clr();
         break;
      case 4:                                   // Vector 4 - TXIFG
         debugpins_isr_set();
         if (uart_tx_isr()==KICK_SCHEDULER) {          // UART; TX
               __bic_SR_register_on_exit(CPUOFF);
         }
         debugpins_isr_clr();
         break;
      default:
         break;
   }
}

// USCI_B3_VECTOR
ISR(USCI_B3){
//void __attribute__((__interrupt__ (USCI_B3_VECTOR))) __MSPGCC_MAYBE_C16__ (USCI_B3_ISR)(void){
   uint16_t op = UCB3IV;

   switch(op) {
      case 0:                                   // Vector 0 - no interrupt
         break;
      case 2:                                   // Vector 2 - RXIFG
         debugpins_isr_set();
         if (spi_isr()==KICK_SCHEDULER) {       // SPI
               __bic_SR_register_on_exit(CPUOFF);
         }
         debugpins_isr_clr();
         break;
      case 4:                                   // Vector 4 - TXIFG
         // we only enable the SPI RX interrupt
         // since RX completion implies a TX completion.
         break;
      default:
         break;
   }
}


// PORT1_VECTOR
ISR(PORT1) {
//void __attribute__((__interrupt__ (PORT1_VECTOR))) __MSPGCC_MAYBE_C16__ (PORT1_ISR)(void){
   uint16_t op = P1IV;

   switch(op){
      case   0:  break;  //  No  Interrupt
      case   2:  break;  //  P1.0
      case   4:                                   //  P1.1
         debugpins_isr_set();
         if (radiotimer_isr_sfd()==KICK_SCHEDULER){ // radio:  SFD pin [P1.1]
               __bic_SR_register_on_exit(CPUOFF);
         }
         debugpins_isr_clr();
         break;
      case   6:  break;    //  P1.2
      case   8:  break;    //  P1.3
      case   10: break;    //  P1.4
      case   12: break;    //  P1.5
      case   14: break;    //  P1.6
      case   16: break;    //  P1.7
      default: break;      //Should never happen
   }
}

// TIMERA1 -- USED BY OS (FreeRTOS)


// TIMERA0_VECTOR
// This interrupt only serves TA0CCR1-6
ISR(TIMER0_A1) {
//void __attribute__((__interrupt__ (TIMER0_A1_VECTOR))) __MSPGCC_MAYBE_C16__ (TIMER0_A1_ISR)(void){
//ISR(TIMER0_A1) {
   debugpins_isr_set();
   if (radiotimer_isr()==KICK_SCHEDULER) {       // radiotimer
         __bic_SR_register_on_exit(CPUOFF);
   }
   debugpins_isr_clr();
}
// ADC12_VECTOR

// WDT_VECTOR

// TIMERB0_VECTOR
// This interrupt only serves TB0CCR0
// For other TB0CC's use TIMER0_B0_VECTOR
ISR(TIMER0_B0) {
//void __attribute__((__interrupt__ (TIMER0_B0_VECTOR))) __MSPGCC_MAYBE_C16__ (TIMER0_B0_ISR)(void){
//ISR(TIMER0_B0) {

   debugpins_isr_set();
   if (bsp_timer_isr()==KICK_SCHEDULER) {        // timer: 0
         __bic_SR_register_on_exit(CPUOFF);
   }
   debugpins_isr_clr();
}



// NMI_VECTOR

#undef SFD
#undef RST
#undef VREG
