 /**
 * Author: Tamas Harczos (tamas.harczos@imms.de)
 * Date:   Apr 2018
 * Description: nRF52840-specific definition of the "uart" bsp module.
 */


#include "nrf52840.h"
#include "nrf52840_bitfields.h"
#include "board_info.h"

#include "leds.h"
#include "debugpins.h"
#include "uart.h"

//=========================== defines =========================================

#define UART_RX_PIN       NRF_GPIO_PIN_MAP(0,8) // p0.08
#define UART_TX_PIN       NRF_GPIO_PIN_MAP(0,6) // p0.06
#define UART_CTS_PIN      NRF_GPIO_PIN_MAP(0,7) // p0.07
#define UART_RTS_PIN      NRF_GPIO_PIN_MAP(0,5) // p0.05

#define UART_BAUDRATE_115200        0x01D7E000  // Baud 115200
#define UART_BAUDRATE_1M            0x10000000  // Baud 1M

#define UART_INTEN_RXDRDY_POS       2
#define UART_INTEN_TXDRDY_POS       7

#define UART_CONFIG_PARITY          0 // excluded
#define UART_CONFIG_PARITY_POS      1
#define UART_CONFIG_HWFC            0
#define UART_CONFIG_HWFC_POS        0

//=========================== variables =======================================

typedef struct
{
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

    NRF_P0->PIN_CNF[UART_TX_PIN] = ((uint32_t)GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos)
                               | ((uint32_t)GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos)
                               | ((uint32_t)GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos)
                               | ((uint32_t)GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)
                               | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);
    NRF_P0->PIN_CNF[UART_RX_PIN] = ((uint32_t)GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos)
                               | ((uint32_t)GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)
                               | ((uint32_t)GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos)
                               | ((uint32_t)GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)
                               | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);
    
    NRF_UART0->PSEL.TXD = UART_TX_PIN;
    NRF_UART0->PSEL.RXD = UART_RX_PIN;

    // default: stop 1 bit, no parity, no HWFC
    NRF_UART0->CONFIG   = 0 ;
    NRF_UART0->BAUDRATE = UART_BAUDRATE_115200;

    NRF_UART0->EVENTS_TXDRDY      = 0;
    NRF_UART0->EVENTS_RXDRDY      = 0;
    NRF_UART0->INTENSET = (1 << UART_INTEN_RXDRDY_POS) |\
                          (1 << UART_INTEN_TXDRDY_POS);

    NVIC->IP[UARTE0_UART0_IRQn]         = (uint8_t)((UART_PRIORITY << (8 - __NVIC_PRIO_BITS)) & (uint32_t)0xFF);
    NVIC->ISER[UARTE0_UART0_IRQn>>5]    = (uint32_t)(0x1 << (UARTE0_UART0_IRQn & 0x1f));

    NRF_UART0->ENABLE   = 4;  // set to 4 to enable

    NRF_UART0->TASKS_STARTTX = 1;
    NRF_UART0->TASKS_STARTRX = 1;
}

void uart_setCallbacks(uart_tx_cbt txCb, uart_rx_cbt rxCb) {
    uart_vars.txCb = txCb;
    uart_vars.rxCb = rxCb;
}

void    uart_enableInterrupts(void) {

    NRF_UART0->INTENSET = 
      (uint32_t)(1<<UART_INTEN_RXDRDY_POS)
    | (uint32_t)(1<<UART_INTEN_TXDRDY_POS);
}

void    uart_clearRxInterrupts(void) {
    
    NRF_UART0->EVENTS_RXDRDY = (uint32_t)0;
}

void    uart_clearTxInterrupts(void) {
    
    NRF_UART0->EVENTS_TXDRDY = (uint32_t)0;
}

void uart_setCTS(bool state) {

    if (state==0x01) {
        NRF_UART0->TXD = XON;
    } else {
        NRF_UART0->TXD = XOFF;
    }
}

void uart_writeByte(uint8_t byteToWrite){

    if (byteToWrite==XON || byteToWrite==XOFF || byteToWrite==XONXOFF_ESCAPE) {
        uart_vars.fXonXoffEscaping     = 0x01;
        uart_vars.xonXoffEscapedByte   = byteToWrite;
        NRF_UART0->TXD = XONXOFF_ESCAPE;
    } else {
        NRF_UART0->TXD = byteToWrite;
    }
}

uint8_t uart_readByte(void) {
    
    return NRF_UART0->RXD;
}

//=========================== private =========================================

void UARTE0_UART0_IRQHandler(void) {

    debugpins_isr_set();

    if (NRF_UART0->EVENTS_RXDRDY) {

        NRF_UART0->EVENTS_RXDRDY = (uint32_t)0;
        uart_rx_isr();
    }

    
    if (NRF_UART0->EVENTS_TXDRDY) {
        
        NRF_UART0->EVENTS_TXDRDY = (uint32_t)0;
        uart_tx_isr();
    }

    debugpins_isr_clr();
}

//=========================== interrupt handlers ==============================

kick_scheduler_t uart_tx_isr(void) {
    
    if (uart_vars.fXonXoffEscaping==0x01) {
        uart_vars.fXonXoffEscaping = 0x00;
        NRF_UART0->TXD = uart_vars.xonXoffEscapedByte^XONXOFF_MASK;
    } else {
        if (uart_vars.txCb != NULL){
            uart_vars.txCb();
            return KICK_SCHEDULER;
        }
    }

    return DO_NOT_KICK_SCHEDULER;
}

kick_scheduler_t uart_rx_isr(void) {

    if (uart_vars.rxCb != NULL){
        uart_vars.rxCb();
        return KICK_SCHEDULER;
    }

    return DO_NOT_KICK_SCHEDULER;
}