/**
\brief SCuM-specific definition of the "uart" bsp module.

\author Tengfei Chang <tengfei.chang@inria.fr>, August 2016.
*/

#include "memory_map.h"
#include "stdint.h"
#include "stdio.h"
#include "string.h"
#include "uart.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   uart_tx_cbt txCb;
   uart_rx_cbt rxCb;
   bool        fXonXoffEscaping;
   uint8_t     xonXoffEscapedByte;
} uart_vars_t;

uart_vars_t uart_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void uart_init(void) {
    // reset local variables
    memset(&uart_vars,0,sizeof(uart_vars_t));
}

void uart_setCallbacks(uart_tx_cbt txCb, uart_rx_cbt rxCb) {
    uart_vars.txCb = txCb;
    uart_vars.rxCb = rxCb;
}

void    uart_writeByte(uint8_t byteToWrite){
    
    if (byteToWrite==XON || byteToWrite==XOFF || byteToWrite==XONXOFF_ESCAPE) {
        uart_vars.fXonXoffEscaping     = 0x01;
        uart_vars.xonXoffEscapedByte   = byteToWrite;
        UART_REG__TX_DATA = XONXOFF_ESCAPE;
    } else {
        UART_REG__TX_DATA = byteToWrite;
    }
    // there is no txdone interruption, call the handler directly
    uart_tx_isr();
}

void    uart_enableInterrupts(void) { 
    // there is no interruption register for uart on SCuM. 
    // the interruption is enabled by default, donot need to enable
}

void    uart_disableInterrupts(void) {
    // there is no interruption register for uart on SCuM. 
    // the interruption is enabled by default, can't be disabled
}

void    uart_clearRxInterrupts(void) {
    // there is no interruption clear register for uart on SCuM. 
    // not required to clear
}

void    uart_clearTxInterrupts(void) {
    // there is no interruption clear register for uart on SCuM. 
    // not required to clear
}

uint8_t uart_readByte(void) {
    return UART_REG__RX_DATA;
}

void uart_setCTS(bool state){
    if (state==0x01) {
        UART_REG__TX_DATA = XON;
    } else {
        UART_REG__TX_DATA = XOFF;
    }
}

//=========================== interrupt handlers ==============================

kick_scheduler_t uart_tx_isr(void) {
    if (uart_vars.fXonXoffEscaping==0x01) {
        uart_vars.fXonXoffEscaping = 0x00;
        UART_REG__TX_DATA = uart_vars.xonXoffEscapedByte^XONXOFF_MASK;
    } else {
        if (uart_vars.txCb != NULL){
            uart_vars.txCb();
        }
    }
    return DO_NOT_KICK_SCHEDULER;
}

kick_scheduler_t uart_rx_isr(void) {
    if (uart_vars.rxCb != NULL){
        uart_vars.rxCb();
    }
    return DO_NOT_KICK_SCHEDULER;
}
