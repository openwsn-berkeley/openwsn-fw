#include "uart.h"
#include "sctimer.h"
#include "radio.h"

void UART_Handler(void){
    uart_rx_isr();
}

void RF_Handler(void){
    radio_isr();
}

void RFTIMER_Handler(void){
    sctimer_isr();
}