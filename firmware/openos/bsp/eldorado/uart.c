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
#define BUS_CLOCK 16000000
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
void SetSCIBaudRate(uint16_t baudRate);
void sci2_PutChar(char send);

//=========================== public ==========================================

//adapted from Freescale example project USB_PHDC
void SetSCIBaudRate(uint16_t baudRate)
{
 uint_32 	baud_divisor;
 
 /* Calculate baud settings */
 /* baud_divisor = clock_speed/ baudrate + 0.5 */
 
 #ifdef  USER_CONFIG_BAUDRATE
  baudRate = USER_CONFIG_BAUDRATE;
 #endif
 
  //error turn off serial
 baud_divisor = (BUS_CLOCK + (8 * (uint_32)baudRate)) / (16 * (uint_32)baudRate);  
 if (baud_divisor > 0x1fff) 
 {
	 SCI2BDH = 0x00;
	 SCI2BDL = 0x00;
 }else{
     SCI2BDH = (uchar)((baud_divisor >> 8) & 0xFF);
     SCI2BDL = (uchar)(baud_divisor & 0xFF);
 }

 return kUARTNoError;
}

void uart_init() {

	
#ifdef UART_BAUDRATE_115200
	 /* Configure SCI baud rate = 115200 */
	SetSCIBaudRate(115200);
#else
     /* Configure SCI baud rate = 9600  */
	SetSCIBaudRate(9600);
#endif
   
	  SCI2C1  = 0;
	  SCI2C2  = 0x0C;
	  SCI2S2  = 0x04;
	  SCI2C2_RE = 0x1;
	  SCI2C2_RIE = 0x1;
	 //SCI2C2_TIE = 0x1;
}

//===== TX

void uart_txSetup(uart_txDone_cbt cb) {
   uart_vars.txDone_cb       = cb;               // register callback
}


//adapted from Freescale example project USB_PHDC
void sci2_PutChar(char send) 
{
  char dummy;
  while(!SCI2S1_TDRE){};
  dummy = (char)SCI2S1;
  SCI2D  = (uint_8)send;    
}

void uart_tx(uint8_t* txBuf, uint8_t txBufLen) {
   
   // register data to send
   uart_vars.txBuf           = txBuf;
   uart_vars.txBufLen        = txBufLen;
   
   // enable UART TX interrupt -- poipoi
   
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
   UC1IFG                   &= ~UCA1RXIFG;
   UC1IE                    |=  UCA1RXIE;
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
   UC1IE                    &= ~UCA1RXIE;
}

//=========================== private =========================================

void reset_rxBuf() {
   uart_vars.rxBufWrPtr      = uart_vars.rxBuf;
   uart_vars.rxBufRdPtr      = uart_vars.rxBuf;
   uart_vars.rxBufFill       = 0;
   
}

//=========================== interrupt handlers ==============================

#pragma vector = USCIAB1TX_VECTOR
__interrupt void USCIAB1TX_ISR (void) {
   // one byte less to go
   uart_vars.txBufLen--;
   uart_vars.txBuf++;
   
   if (uart_vars.txBufLen>0) {
      // send next byte
      UCA1TXBUF              = *uart_vars.txBuf;
   } else {
      if (uart_vars.txDone_cb!=NULL) {
         // disable UART1 TX interrupt
         UC1IE              &= ~UCA1TXIE;
         // call the callback
         uart_vars.txDone_cb();
         // make sure CPU restarts after leaving interrupt
         __bic_SR_register_on_exit(CPUOFF);
      }
   }
}

#pragma vector = USCIAB1RX_VECTOR
__interrupt void USCIAB1RX_ISR (void) {
   // copy received by into buffer
   *uart_vars.rxBufWrPtr     =  UCA1RXBUF;
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
         __bic_SR_register_on_exit(CPUOFF);
      }
      
   } else if (uart_vars.rxBufFill>=uart_vars.rxBufFillThres) {
      // buffer above threshold
      
      if (uart_vars.rx_cb!=NULL) {
         // call the callback
         uart_vars.rx_cb(UART_EVENT_THRES);
         // make sure CPU restarts after leaving interrupt
         __bic_SR_register_on_exit(CPUOFF);
      }
   }
}


//////////////////////////////////////////////////NEW///////////////////////
/******************************************************************************
*
*  SCI RX Interrupt
*
******************************************************************************/
void interrupt VectorNumber_Vsci2tx SCI2TX_int(void){
   int i;
	// Tratamento da interrupção
	(void)SCI2S1;		// Leitura do registrador SCI1S1 para analisar o estado da transmissão
	(void)SCI2C3;		// Leitura do registrador SCI1C3 para limpar o bit de paridade (caso exista)

   for (i=0;i<nrbytestx;i++)
   {
     while (!SCI2S1_TDRE);  /* ensure Tx data buffer empty */
     SCI2D = txbuf[i];				// dado a mandar
     //SCI1C2_TIE = 0;
     //SCI2C2_TCIE = 1; 
     while (!SCI2S1_TC); /* wait for Tx complete */
   }

   SCI2C2_TCIE = 0; 
 //  SCI1C2_RIE = 1;
}

//receive
void interrupt SCI2TX_INTNUM SCI2RX_int(void){

	// Tratamento da interrupção
	(void)SCI2S1;		// Leitura do registrador SCI1S1 para analisar o estado da transmissão
	(void)SCI2C3;		// Leitura do registrador SCI1C3 para limpar o bit de paridade (caso exista)

      sci_Data = SCI2D;				// dado a mandar
      sci_DataAvailable = sciDISPARA_1;	// existe dado pra mandar
}
////////////////////////////////////////////