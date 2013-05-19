/**
\brief Python-specific definition of the "uart" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, May 2013.
*/

#include "uart_obj.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   uart_tx_cbt txCb;
   uart_rx_cbt rxCb;
} uart_vars_t;

uart_vars_t uart_vars;

//=========================== prototypes ======================================

//=========================== callbacks =======================================

void uart_setCallbacks(OpenMote* self, uart_tx_cbt txCb, uart_rx_cbt rxCb) {
   uart_vars.txCb = txCb;
   uart_vars.rxCb = rxCb;
}

//=========================== public ==========================================

void uart_init(OpenMote* self) {
   // clear local variables
   memset(&uart_vars,0,sizeof(uart_vars_t));
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_uart_init,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}

void uart_enableInterrupts(OpenMote* self) {
   /*
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_uart_enableInterrupts,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}

void uart_disableInterrupts(OpenMote* self) {
   /*
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_uart_disableInterrupts,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}

void uart_clearRxInterrupts(OpenMote* self) {
   /*
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_uart_clearRxInterrupts,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}

void uart_clearTxInterrupts(OpenMote* self) {
   /*
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_uart_clearTxInterrupts,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}

void uart_writeByte(OpenMote* self, uint8_t byteToWrite) {
   /*
   opensim_requ_uart_writeByte_t requparams;
   
   // prepare params
   requparams.byteToWrite = byteToWrite;
   
   // send request to server and get reply
   
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_uart_writeByte,
                                    &requparams,
                                    sizeof(opensim_requ_uart_writeByte_t),
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}

uint8_t uart_readByte(OpenMote* self) {
   /*
   opensim_repl_uart_readByte_t replparams;
   
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_uart_readByte,
                                    0,
                                    0,
                                    &replparams,
                                    sizeof(opensim_repl_uart_readByte_t));
   
   // TODO: replace by call to Python
   
   return replparams.byteRead;
   */
   return 0;//poipoi
}

//=========================== interrupt handlers ==============================

kick_scheduler_t uart_tx_isr(OpenMote* self) {
   uart_vars.txCb(self);
   return 0;//poipoi
}

kick_scheduler_t uart_rx_isr(OpenMote* self) {
   uart_vars.rxCb(self);
   return 0;//poipoi
}