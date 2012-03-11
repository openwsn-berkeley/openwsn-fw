/**
\brief LPC17XX-specific definition of the "board" bsp module.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, February 2012.
*/

#include "LPC17xx.h"
#include "board.h"
#include "leds.h"
#include "uart.h"
#include "spi.h"
#include "radio.h"
#include "bsp_timers.h"
#include "clkpwr.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void board_init() {

   //call system_init from system_LPC17xx.c
   SystemInit();

   LPC_PINCON->PINSEL4     &= ~0x3<<6;          // set pinsel to GPIO pag 110 port 2.3
   LPC_PINCON->PINSEL4     &= ~0x3<<8;          // P2.4
   LPC_PINCON->PINSEL4     &= ~0x3<<10;         // P2.5

   LPC_GPIO2->FIODIR        |=  1<<3;            // P2.3 as SLP_TR -- set as output
   LPC_GPIO2->FIODIR        |=  1<<4;            // P2.4 as Radio RST --set as output
   LPC_GPIO2->FIODIR        &= ~1<<5;            // P2.5 as RADIO ISR --set as input

   LPC_GPIOINT->IO2IntClr  |=  1<<5;            // P2.5 clear interrupt.
   LPC_GPIOINT->IO2IntEnR  |=  1<<5;            // P2.5 enable as interrupt when low to high


   // initialize bsp modules
   //debugpins_init();
   leds_init();
   //uart_init();
   //spi_init();
   //radio_init();
   //timers_init();
   //radio_init();
   //radiotimer_init();
}

void board_sleep() {
   CLKPWR_Sleep();
}

//=========================== private =========================================
