/**
\brief PC-specific definition of the "uart" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, April 2012.
*/

#include "uart.h"
#include "opensim_proto.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   uart_tx_cbt txCb;
   uart_rx_cbt rxCb;
} uart_vars_t;

uart_vars_t uart_vars;

//=========================== prototypes ======================================

//=========================== callbacks =======================================

void uart_setCallbacks(uart_tx_cbt txCb, uart_rx_cbt rxCb) {
   uart_vars.txCb = txCb;
   uart_vars.rxCb = rxCb;
}

//=========================== public ==========================================

void uart_init() {
   
   // clear local variables
   memset(&uart_vars,0,sizeof(uart_vars_t));
   
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_uart_init,
                                    0,
                                    0,
                                    0,
                                    0);
}

void uart_enableInterrupts(){
   
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_uart_enableInterrupts,
                                    0,
                                    0,
                                    0,
                                    0);
}

void uart_disableInterrupts(){
  
  // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_uart_disableInterrupts,
                                    0,
                                    0,
                                    0,
                                    0);
}

void uart_clearRxInterrupts(){
  
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_uart_clearRxInterrupts,
                                    0,
                                    0,
                                    0,
                                    0);
}

void uart_clearTxInterrupts(){
   
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_uart_clearTxInterrupts,
                                    0,
                                    0,
                                    0,
                                    0);
}

void uart_writeByte(uint8_t byteToWrite){
  opensim_requ_uart_writeByte_t requparams;
   
   // prepare params
   requparams.byteToWrite = byteToWrite;
   
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_uart_writeByte,
                                    &requparams,
                                    sizeof(opensim_requ_uart_writeByte_t),
                                    0,
                                    0);
}

uint8_t uart_readByte(){
   opensim_repl_uart_readByte_t replparams;

   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_uart_readByte,
                                    0,
                                    0,
                                    &replparams,
                                    sizeof(opensim_repl_uart_readByte_t));
   
   return replparams.byteRead;
}

//=========================== interrupt handlers ==============================