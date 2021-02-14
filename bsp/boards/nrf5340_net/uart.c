/**
\brief nRF5340_network-specific definition of the "uart" bsp module.

\author Tengfei Chang <tengfei.chang@inria.fr>, August 2020.
*/

#include "nRF5340_network.h"
#include "nrf5340_network_bitfields.h"
#include "uart.h"
#include "board.h"
#include "debugpins.h"

//=========================== defines =========================================

#define NRF_GPIO_PIN_MAP(port, pin) (((port) << 5) | ((pin) & 0x1F))

#define UART_RX_PIN       NRF_GPIO_PIN_MAP(0,22) // p0.22
#define UART_TX_PIN       NRF_GPIO_PIN_MAP(0,20) // p0.20
#define UART_CTS_PIN      NRF_GPIO_PIN_MAP(0,21) // p0.21
#define UART_RTS_PIN      NRF_GPIO_PIN_MAP(0,19) // p0.19

#define UART_BAUDRATE_115200        0x01D7E000  // Baud 115200
#define UART_BAUDRATE_1M            0x10000000  // Baud 1M

#define UART_CONFIG_PARITY          0 // excluded
#define UART_CONFIG_PARITY_POS      1
#define UART_CONFIG_HWFC            0
#define UART_CONFIG_HWFC_POS        0
  
#define UART_INTEN_ENDRX_POS       4
#define UART_INTEN_ENDTX_POS       8

//=========================== variables =======================================

typedef struct {
    uart_tx_cbt txCb;
    uart_rx_cbt rxCb;
    bool        fXonXoffEscaping;
    uint8_t     xonXoffEscapedByte;
    uint8_t     byteToSend;
    uint8_t     rx_byte;
} uart_vars_t;

uart_vars_t uart_vars;

//=========================== private =========================================

//=========================== public ==========================================

void uart_init(void) {
    
    memset(&uart_vars,0,sizeof(uart_vars_t));
    
    // configure txd and rxd pin
    NRF_P0_NS->OUTSET =  1 << UART_TX_PIN;

    // tx pin configured as output
    NRF_P0_NS->PIN_CNF[UART_TX_PIN] =   \
          ((uint32_t)GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos)
        | ((uint32_t)GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos)
        | ((uint32_t)GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos)
        | ((uint32_t)GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)
        | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);
 
    // rx pin configured as input
    NRF_P0_NS->PIN_CNF[UART_RX_PIN] =   \
           ((uint32_t)GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos)
         | ((uint32_t)GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)
         | ((uint32_t)GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos)
         | ((uint32_t)GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)
         | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);

    // configure uart
    NRF_UARTE0_NS->BAUDRATE = (uint32_t)(UART_BAUDRATE_115200);
    NRF_UARTE0_NS->CONFIG   = 
          (uint32_t)(UART_CONFIG_PARITY << UART_CONFIG_PARITY_POS)
        | (uint32_t)(UART_CONFIG_HWFC   << UART_CONFIG_HWFC_POS);
    NRF_UARTE0_NS->PSEL.RXD = (uint32_t)UART_RX_PIN;
    NRF_UARTE0_NS->PSEL.TXD = (uint32_t)UART_TX_PIN;

    // enable UART rx done ready and tx done ready interrupts

    NRF_UARTE0_NS->INTENSET = 
          (uint32_t)(1<<UART_INTEN_ENDRX_POS)
        | (uint32_t)(1<<UART_INTEN_ENDTX_POS);

    // set priority and enable interrupt in NVIC
    NVIC->IPR[((uint32_t)SPIM0_SPIS0_TWIM0_TWIS0_UARTE0_IRQn)] = 
        (uint8_t)(
            (
                UART_PRIORITY << (8 - __NVIC_PRIO_BITS)
            ) & (uint32_t)0xff
        );
    NVIC->ISER[((uint32_t)SPIM0_SPIS0_TWIM0_TWIS0_UARTE0_IRQn)>>5] = 
       ((uint32_t)1) << ( ((uint32_t)SPIM0_SPIS0_TWIM0_TWIS0_UARTE0_IRQn) & 0x1f);

    // enable uart
    NRF_UARTE0_NS->ENABLE = (uint32_t)UARTE_ENABLE_ENABLE_Enabled;

    // start to rx (tx is started when sending byte)
    NRF_UARTE0_NS->RXD.PTR        = (uint32_t)(&uart_vars.rx_byte);
    NRF_UARTE0_NS->RXD.MAXCNT     = 1;
    NRF_UARTE0_NS->TASKS_STARTRX  = (uint32_t)1;

}

void uart_setCallbacks(uart_tx_cbt txCb, uart_rx_cbt rxCb) {

    uart_vars.txCb = txCb;
    uart_vars.rxCb = rxCb;
}

void    uart_enableInterrupts(void) {

    NRF_UARTE0_NS->INTENSET = 
      (uint32_t)(1<<UART_INTEN_ENDRX_POS)
    | (uint32_t)(1<<UART_INTEN_ENDTX_POS);
}

void    uart_disableInterrupts(void) {

    NRF_UARTE0_NS->INTENCLR = 
      (uint32_t)(1<<UART_INTEN_ENDRX_POS)
    | (uint32_t)(1<<UART_INTEN_ENDTX_POS);
}

void    uart_clearRxInterrupts(void) {
    
    NRF_UARTE0_NS->EVENTS_ENDRX = (uint32_t)0;
}

void    uart_clearTxInterrupts(void) {
    
    NRF_UARTE0_NS->EVENTS_ENDTX = (uint32_t)0;
}

void uart_setCTS(bool state) {

    uint8_t byteToSend;

    if (state==0x01) {
        byteToSend= XON;
    } else {
        byteToSend= XOFF;
    }

    NRF_UARTE0_NS->TXD.PTR        = &byteToSend;
    NRF_UARTE0_NS->TXD.MAXCNT     = 1;
    NRF_UARTE0_NS->TASKS_STARTTX  = 1;

}

void uart_writeByte(uint8_t byteToWrite){

    //if (byteToWrite==XON || byteToWrite==XOFF || byteToWrite==XONXOFF_ESCAPE) {
    //    uart_vars.fXonXoffEscaping     = 0x01;
    //    uart_vars.xonXoffEscapedByte   = byteToWrite;
    //    byteToSend = XONXOFF_ESCAPE;
    //} else {
    //    byteToSend = byteToWrite;
    //}

    uart_vars.byteToSend = byteToWrite;
    NRF_UARTE0_NS->TXD.PTR        = &uart_vars.byteToSend;
    NRF_UARTE0_NS->TXD.MAXCNT     = 1;
    NRF_UARTE0_NS->TASKS_STARTTX  = 1;
}

uint8_t uart_readByte(void) {
    
    return uart_vars.rx_byte;
}

//=========================== private =========================================

void SPIM0_SPIS0_TWIM0_TWIS0_UARTE0_IRQHandler(void) {

    debugpins_isr_set();

    if (NRF_UARTE0_NS->EVENTS_ENDRX) {

        NRF_UARTE0_NS->EVENTS_ENDRX = (uint32_t)0;
        uart_rx_isr();
    }

    
    if (NRF_UARTE0_NS->EVENTS_ENDTX) {
        
        NRF_UARTE0_NS->EVENTS_ENDTX = (uint32_t)0;
        uart_tx_isr();
    }

    debugpins_isr_clr();
}

//=========================== interrupt handlers ==============================

kick_scheduler_t uart_tx_isr(void) {

    uint8_t byteToSend;
    
    //if (uart_vars.fXonXoffEscaping==0x01) {
    //    uart_vars.fXonXoffEscaping = 0x00;
    //    byteToSend = uart_vars.xonXoffEscapedByte^XONXOFF_MASK;

    //    NRF_UARTE0_NS->TXD.PTR        = &byteToSend;
    //    NRF_UARTE0_NS->TXD.MAXCNT     = 1;
    //    NRF_UARTE0_NS->TASKS_STARTTX  = 1;

    //} else {
    //    if (uart_vars.txCb != NULL){
    //        uart_vars.txCb();
    //        return KICK_SCHEDULER;
    //    }
    //}
    if (uart_vars.txCb != NULL){
        uart_vars.txCb();
        return KICK_SCHEDULER;
    }

    return DO_NOT_KICK_SCHEDULER;
}

kick_scheduler_t uart_rx_isr(void) {

    if (uart_vars.rxCb != NULL){
        uart_vars.rxCb();

        // start to rx (tx is started when need sending packet)
        NRF_UARTE0_NS->RXD.PTR        = &uart_vars.rx_byte;
        NRF_UARTE0_NS->RXD.MAXCNT     = 1;

        NRF_UARTE0_NS->TASKS_STARTRX  = (uint32_t)1;

        return KICK_SCHEDULER;
    }

    return DO_NOT_KICK_SCHEDULER;
}
