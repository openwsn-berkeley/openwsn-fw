/**
\brief ELDORADO-specific definition of the "uart" bsp module.

\author Branko Kerkez <bkerkez@berkeley.edu>, February 2012.
*/


#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */
#include "stdio.h"
#include "string.h"
#include "uart.h"


//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   // TX
   uint8_t*        txBuf;
   uint8_t         txBufLen;
   uart_txDone_cbt txDone_cb;
   // RX
   uint8_t*        rxBuf;
   uint8_t         rxBufLen;
   uint8_t         rxBufFillThres;
   uint8_t*        rxBufWrPtr;
   uint8_t*        rxBufRdPtr;
   uint8_t         rxBufFill;
   uart_rx_cbt     rx_cb;
} uart_vars_t;

uart_vars_t uart_vars;

//=========================== prototypes ======================================

void reset_rxBuf();
void SetSCIBaudRate(uint32_t baudRate);
void sci2_PutChar(char send);
void interrupt VectorNumber_Vsci2tx SCI2TX_int(void);
void interrupt VectorNumber_Vsci2rx SCI2RX_ISR(void);


//=========================== public ==========================================

void uart_init() {

	SCI2C2 = 0x00;                       /* Disable the SCI2 module */
	(void)(SCI2S1 == 0);                 /* Dummy read of the SCI2S1 registr to clear flags */
	(void)(SCI2D == 0);                  /* Dummy read of the SCI2D registr to clear flags */
	
   //given a bus speed of 8.886857MHZ	
#ifdef UART_BAUDRATE_115200
	 /* Configure SCI baud rate = 115200 */
	SCI2BDH = 0x00; 
	SCI2BDL = 5;  //baud=BUS_SPEED/(SCI2BDL*16)
#else
     /* Configure SCI baud rate = 9600 */	
	SCI2BDH = 0x00; 
	SCI2BDL = 58;  //baud=BUS_SPEED/(SCI2BDL*16)
#endif
  
    SCI2C1  = 0;
	SCI2C2  = 0x0C;
	SCI2S2  = 0x04;
	SCI2C2_RE = 0x1;  //RX enable
    SCI2C2_RIE = 0x0; //RX interupt disable, enable  in tx function    
	SCI2C2_TE = 0x1;  //TX enable
    SCI2C2_TIE = 0x0; //TX interupt disable, enable  in tx function  
}




//===== TX

void uart_txSetup(uart_txDone_cbt cb) {
   uart_vars.txDone_cb       = cb;               // register callback
}


//adapted from Freescale example project USB_PHDC
void sci2_PutChar(char send) 
{

  while (!SCI2S1_TDRE);  /* ensure Tx data buffer empty */
  SCI2D = send;			 
  while (!SCI2S1_TC); /* wait for Tx complete */
  
}

void uart_tx(uint8_t* txBuf, uint8_t txBufLen) {
   
   // register data to send
   uart_vars.txBuf           = txBuf;
   uart_vars.txBufLen        = txBufLen;
   
   // enable UART TX interrupt 
   SCI2C2_TIE = 0x1;   
   // send first byte
   sci2_PutChar(*uart_vars.txBuf);
}



//===== RX

void uart_rxSetup(uint8_t*    rxBuf,
                  uint8_t     rxBufLen,
                  uint8_t     rxBufFillThres,
                  uart_rx_cbt cb) {
   uart_vars.rxBuf           = rxBuf;
   uart_vars.rxBufLen        = rxBufLen;
   uart_vars.rxBufFillThres  = rxBufFillThres;
   reset_rxBuf();
   uart_vars.rx_cb           = cb;
   
}

void uart_rxStart() {
   // enable UART RX interrupt
   SCI2C2_RIE = 0x1;
}

void uart_readBytes(uint8_t* buf, uint8_t numBytes) {
   uint8_t i;
   
   for (i=0;i<numBytes;i++) {
      // copy byte into receive buffer
      *buf                   = *uart_vars.rxBufRdPtr;
      // advance counters
      buf++;
      uart_vars.rxBufRdPtr++;
      if (uart_vars.rxBufRdPtr>=uart_vars.rxBuf+uart_vars.rxBufLen) {
         uart_vars.rxBufRdPtr= uart_vars.rxBuf;
      }
   }
   
   // reduce fill
   uart_vars.rxBufFill      -= numBytes;
}

void uart_rxStop() {
   // disable UART1 RX interrupt
	SCI2C2_RIE = 0x0;
}

//=========================== private =========================================

void reset_rxBuf() {
   uart_vars.rxBufWrPtr      = uart_vars.rxBuf;
   uart_vars.rxBufRdPtr      = uart_vars.rxBuf;
   uart_vars.rxBufFill       = 0;
   
}

//=========================== interrupt handlers ==============================

void interrupt VectorNumber_Vsci2tx SCI2TX_int(void) {
   // one byte less to go
   uart_vars.txBufLen--;
   uart_vars.txBuf++;
   
   if (uart_vars.txBufLen>0) {
      // send next byte
	   sci2_PutChar(*uart_vars.txBuf);
   } else {
      if (uart_vars.txDone_cb!=NULL) {
         // disable UART1 TX interrupt
    	  SCI2C2_TIE = 0x0;
         // call the callback
         uart_vars.txDone_cb();
      }//poipoi
   }
}


void interrupt VectorNumber_Vsci2rx SCI2RX_ISR(void){
   // copy received by into buffer
   *uart_vars.rxBufWrPtr     =  SCI2D;
   // shift pointer
   uart_vars.rxBufWrPtr++;
   if (uart_vars.rxBufWrPtr>=uart_vars.rxBuf+uart_vars.rxBufLen) {
      uart_vars.rxBufWrPtr   =  uart_vars.rxBuf;
   }
   // increment fill
   uart_vars.rxBufFill++;
   
   if        (uart_vars.rxBufFill>=uart_vars.rxBufLen) {
      // buffer has overflown
      
      // reset buffer
      reset_rxBuf();
      
      
      if (uart_vars.rx_cb!=NULL) {
         // call the callback
         uart_vars.rx_cb(UART_EVENT_OVERFLOW);
         // make sure CPU restarts after leaving interrupt
      }
      
   } else if (uart_vars.rxBufFill>=uart_vars.rxBufFillThres) {
      // buffer above threshold
      
      if (uart_vars.rx_cb!=NULL) {
         // call the callback
         uart_vars.rx_cb(UART_EVENT_THRES);
         
      }
   }
}
