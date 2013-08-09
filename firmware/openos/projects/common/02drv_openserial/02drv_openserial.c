/**
\brief This is a program which shows how to use the "openserial "driver module.

Since the driver modules for different platforms have the same declaration, you
can use this project with any platform.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, January 2013.
*/

#include "stdint.h"
#include "stdio.h"
// bsp modules required
#include "board.h"
#include "leds.h"
#include "uart.h"

// driver modules required
#include "openserial.h"

//=========================== defines =========================================


//=========================== variables =======================================


//=========================== prototypes ======================================
void isr_serie_tx();
void isr_serie_rx();

//=========================== main ============================================

/**
\brief The program starts executing here.
in order to echo chunks of bytes, each chunk needs to start with character 'H' as
openserial takes different actions according to the initial character of the stream.
*/
int mote_main() {  
   board_init();
   openserial_init();
   
   uart_clearTxInterrupts();
   uart_clearRxInterrupts();          // clear possible pending interrupts
   uart_enableInterrupts(); 
  
   openserial_startInput();
      
   while(1) {
      board_sleep();
   }
}

//=========================== callbacks =======================================

void isr_serie_tx(){
  leds_all_toggle();
  uart_clearTxInterrupts();
  uart_clearRxInterrupts();          // clear possible pending interrupts
  uart_enableInterrupts(); 
  
  openserial_startInput();
 }


void isr_serie_rx(){
  uint8_t bytes;
  uint8_t bufferToWrite[136];
  
  leds_all_toggle();
  bytes=openserial_getNumDataBytes();
  openserial_getInputBuffer(bufferToWrite, bytes);
  
  openserial_printData(bufferToWrite, bytes);
  openserial_startOutput();
  
}