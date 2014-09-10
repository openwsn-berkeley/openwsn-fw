/**
\brief Mercator firmware, see https://github.com/openwsn-berkeley/mercator/.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2014.
*/

// stack initialization
#include "board.h"
#include "scheduler.h"
#include "opentimers.h"
#include "openhdlc.h"
#include "leds.h"
#include "uart.h"

//=========================== define ==========================================

#define UART_BUF_LEN    30
#define RF_BUF_LEN      128

//=========================== variables =======================================

typedef struct {
   opentimer_id_t  timerId;
   
   //=== UART
   // tx
   uint8_t         uartbuftx[UART_BUF_LEN];
   uint8_t         uartbufrdidx;
   uint8_t         uartbuftxfill;
   uint8_t         uarttxescape;
   uint16_t        uarttxcrc;
   uint8_t         uarttxcrcAdded;
   uint8_t         uarttxclosingSent;
   // rx
   uint8_t         uartbufrx[UART_BUF_LEN];
   uint8_t         uartbufrxfill;
   uint8_t         uartlastRxByte;
   
   // RF
   uint8_t         rfbuftx[RF_BUF_LEN];
   uint8_t         rfbufrx[RF_BUF_LEN];
   
} mercator_vars_t;

mercator_vars_t mercator_vars;

//=========================== prototypes ======================================

void mercator_poipoi(void);

void uart_enable(void);
void uart_flushtx(void);
void uart_disable(void);

void isr_openserial_tx(void);
void isr_openserial_rx(void);

//=========================== initialization ==================================

int mote_main(void) {
   
   board_init();
   scheduler_init();
   opentimers_init();
   
   // initial UART
   uart_setCallbacks(
      isr_openserial_tx,
      isr_openserial_rx
   );
   uart_enable();
   
   mercator_vars.timerId  = opentimers_start(
      500,
      TIMER_PERIODIC,TIME_MS,
      mercator_poipoi
   );
   scheduler_start();
   return 0; // this line should never be reached
}

//=========================== private =========================================

void mercator_poipoi(void) {
   
   mercator_vars.uartbuftx[0] = 0x00;
   mercator_vars.uartbuftx[1] = 0x11;
   mercator_vars.uartbuftx[2] = 0x22;
   mercator_vars.uartbuftx[3] = 0x33;
   mercator_vars.uartbuftx[4] = 0x44;
   mercator_vars.uartbuftx[5] = 0x7e;
   mercator_vars.uartbuftx[6] = 0x66;
   mercator_vars.uartbuftx[7] = 0x77;
   mercator_vars.uartbuftx[8] = 0x88;
   mercator_vars.uartbuftx[9] = 0x99;
   
   mercator_vars.uartbuftxfill = 10;
   uart_flushtx();
}

//===== uart

void uart_enable(void) {
   uart_clearTxInterrupts();
   uart_clearRxInterrupts();      // clear possible pending interrupts
   uart_enableInterrupts();       // Enable USCI_A1 TX & RX interrupt
}

void uart_flushtx(void) {
   
   // abort if nothing to print
   if (mercator_vars.uartbuftxfill==0) {
      return;
   }
   
   mercator_vars.uarttxcrc             = HDLC_CRCINIT;
   mercator_vars.uartbufrdidx          = 0;
   mercator_vars.uarttxcrcAdded        = 0;
   mercator_vars.uarttxclosingSent     = 0;
   uart_writeByte(HDLC_FLAG);
}

void uart_disable(void) {
   uart_disableInterrupts();      // disable USCI_A1 TX & RX interrupt
}

//===== interrupt handlers

uint8_t  b;

//executed in ISR, called from scheduler.c
void isr_openserial_tx(void) {
   
   uint16_t finalCrc;
   
   // led
   leds_sync_on();
   
   // write next byte, if any
   if (mercator_vars.uartbuftxfill>0) {
      b = mercator_vars.uartbuftx[mercator_vars.uartbufrdidx];
      if (mercator_vars.uarttxescape==0 && (b==HDLC_FLAG || b==HDLC_ESCAPE)) {
         uart_writeByte(HDLC_ESCAPE);
         mercator_vars.uarttxescape=1;
      } else {
         mercator_vars.uarttxcrc = crcIteration(mercator_vars.uarttxcrc,b);
         if (mercator_vars.uarttxescape==1) {
             b = b^HDLC_ESCAPE_MASK;
             mercator_vars.uarttxescape = 0;
         }
         mercator_vars.uartbufrdidx++;
         mercator_vars.uartbuftxfill--;
         uart_writeByte(b);
      }
      if (mercator_vars.uartbuftxfill==0 && mercator_vars.uarttxcrcAdded==0) {
          // finalize the calculation of the CRC
          finalCrc   = ~mercator_vars.uarttxcrc;
          
          // write the CRC value
          mercator_vars.uartbuftx[mercator_vars.uartbufrdidx]   = (finalCrc>>0)&0xff;
          mercator_vars.uartbuftx[mercator_vars.uartbufrdidx+1] = (finalCrc>>8)&0xff;
          
          mercator_vars.uartbuftxfill += 2;
          mercator_vars.uarttxcrcAdded = 1;
      }
   } else {
      if (mercator_vars.uarttxclosingSent==0){
         mercator_vars.uarttxclosingSent = 1;
         uart_writeByte(HDLC_FLAG);
      }
   }
   
   // led
   leds_sync_off();
}

// executed in ISR, called from scheduler.c
void isr_openserial_rx(void) {
   uint8_t rxbyte;
   
   // led
   leds_sync_on();
   
   // read byte just received
   rxbyte = uart_readByte();
   
   // store byte
   mercator_vars.uartlastRxByte = rxbyte;
   
   // led
   leds_sync_off();
}
