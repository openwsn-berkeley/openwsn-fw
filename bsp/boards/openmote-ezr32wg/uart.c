/**
 * Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Date:   Jan 2016
 * Description: EZR32WG-specific definition of the "uart" bsp module.
 */

#include "stdint.h"
#include "stdio.h"
#include "string.h"
#include "uart.h"
#include "board.h"
#include "debugpins.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_usart.h"


//=========================== defines =========================================

	#define UART_IRQ_NAME    USART1_RX_IRQHandler         /* USART IRQ Handler */
	#define UART_CLK         cmuClock_USART1              /* HFPER Clock */
	#define UART_IRQn        USART1_RX_IRQn               /* IRQ number */
	#define UART_UART        USART1                       /* UART instance */
	#define UART_TX          USART_Tx                     /* Set TX to USART_Tx */
	#define UART_RX          USART_Rx                     /* Set RX to USART_Rx */
	#define UART_LOCATION    USART_ROUTE_LOCATION_LOC1    /* Location of of the USART I/O pins */
	#define UART_TXPORT      gpioPortD                    /* USART transmission port */
	#define UART_TXPIN       0                            /* USART transmission pin */
	#define UART_RXPORT      gpioPortD                    /* USART reception port */
	#define UART_RXPIN       1                            /* USART reception pin */
	#define UART_USART       1                            /* Includes em_usart.h */
	#define UART_PERIPHERAL_ENABLE()

//=========================== variables =======================================

typedef struct {
   uart_tx_cbt txCb;
   uart_rx_cbt rxCb;
} uart_vars_t;

uart_vars_t uart_vars;

//=========================== prototypes ======================================

static void uart_isr_private(void);

//=========================== public ==========================================

void uart_init() { 
   // reset local variables
   memset(&uart_vars,0,sizeof(uart_vars_t));
   /* Enable peripheral clocks */
   CMU_ClockEnable(cmuClock_HFPER, TRUE);
   /* Configure GPIO pins */
   CMU_ClockEnable(cmuClock_GPIO, TRUE);

   /* To avoid false start, configure output as high */
   GPIO_PinModeSet(UART_TXPORT, UART_TXPIN, gpioModePushPull, 1);
   GPIO_PinModeSet(UART_RXPORT, UART_RXPIN, gpioModeInput, 0);

#if defined(UART_USART)
    USART_TypeDef           *usart = UART_UART;
    USART_InitAsync_TypeDef init   = USART_INITASYNC_DEFAULT;

    /* Enable DK RS232/UART switch */
    UART_PERIPHERAL_ENABLE();

    CMU_ClockEnable(UART_CLK, TRUE);

    /* Configure USART for basic async operation */
    init.enable = usartDisable;
    USART_InitAsync(usart, &init);

    /* Enable pins at correct UART/USART location. */
	#if defined( USART_ROUTEPEN_RXPEN )
    	usart->ROUTEPEN = USART_ROUTEPEN_RXPEN | USART_ROUTEPEN_TXPEN;
    	usart->ROUTELOC0 = ( usart->ROUTELOC0 &
                         ~( _USART_ROUTELOC0_TXLOC_MASK
                            | _USART_ROUTELOC0_RXLOC_MASK ) )
                       | ( UART_TX_LOCATION << _USART_ROUTELOC0_TXLOC_SHIFT )
                       | ( UART_RX_LOCATION << _USART_ROUTELOC0_RXLOC_SHIFT );
	#else
    usart->ROUTE = USART_ROUTE_RXPEN | USART_ROUTE_TXPEN | UART_LOCATION;
	#endif

    /* Clear previous RX interrupts */
    USART_IntClear(UART_UART, USART_IF_RXDATAV);
    NVIC_ClearPendingIRQ(UART_IRQn);

    /* Enable RX interrupts */
    USART_IntEnable(UART_UART, USART_IF_RXDATAV);
    NVIC_EnableIRQ(UART_IRQn);

    /* Finally enable it */
    USART_Enable(usart, usartEnable);

#else
    //low energy UART
    LEUART_TypeDef      *leuart = UART_LEUART;
    LEUART_Init_TypeDef init    = LEUART_INIT_DEFAULT;

    /* Enable DK LEUART/RS232 switch */
    UART_PERIPHERAL_ENABLE();

    /* Enable CORE LE clock in order to access LE modules */
    CMU_ClockEnable(cmuClock_CORELE, TRUE);

	#if defined(UART_VCOM)
    /* Select HFXO/2 for LEUARTs (and wait for it to stabilize) */
	#if defined(_CMU_LFCLKSEL_LFB_HFCORECLKLEDIV2)
    	CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_CORELEDIV2);
	#else
    	CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_HFCLKLE);
	#endif
	#else
    /* Select LFXO for LEUARTs (and wait for it to stabilize) */
    	CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);
  #endif

    CMU_ClockEnable(UART_CLK, true);

    /* Do not prescale clock */
    CMU_ClockDivSet(UART_CLK, cmuClkDiv_1);

    /* Configure LEUART */
    init.enable = leuartDisable;
  #if defined(UART_VCOM)
    init.baudrate = 115200;
  #endif
    LEUART_Init(leuart, &init);
    /* Enable pins at default location */
    #if defined( LEUART_ROUTEPEN_RXPEN )
    leuart->ROUTEPEN = USART_ROUTEPEN_RXPEN | USART_ROUTEPEN_TXPEN;
    leuart->ROUTELOC0 = ( leuart->ROUTELOC0 &
                         ~( _LEUART_ROUTELOC0_TXLOC_MASK
                            | _LEUART_ROUTELOC0_RXLOC_MASK ) )
                       | ( UART_TX_LOCATION << _LEUART_ROUTELOC0_TXLOC_SHIFT )
                       | ( UART_RX_LOCATION << _LEUART_ROUTELOC0_RXLOC_SHIFT );
    #else
    leuart->ROUTE = LEUART_ROUTE_RXPEN | LEUART_ROUTE_TXPEN | UART_LOCATION;
    #endif

    /* Clear previous RX interrupts */
    LEUART_IntClear(UART_UART, LEUART_IF_RXDATAV);
    NVIC_ClearPendingIRQ(UART_IRQn);

    /* Enable RX interrupts */
    LEUART_IntEnable(UART_UART, LEUART_IF_RXDATAV);
    NVIC_EnableIRQ(UART_IRQn);

    /* Finally enable it */
    LEUART_Enable(leuart, leuartEnable);
  #endif

  #if !defined(__CROSSWORKS_ARM) && defined(__GNUC__)
    setvbuf(stdout, NULL, _IONBF, 0);   /*Set unbuffered mode for stdout (newlib)*/
  #endif
    printf("\nEZR32 USART example\n");
}

void uart_setCallbacks(uart_tx_cbt txCb, uart_rx_cbt rxCb) {
    uart_vars.txCb = txCb;
    uart_vars.rxCb = rxCb;
}

void uart_enableInterrupts(){
	USART_IntEnable(UART_UART, USART_IF_RXDATAV);
	USART_IntEnable(UART_UART, USART_IF_TXBL);
}

void uart_disableInterrupts(){
	USART_IntDisable(UART_UART, USART_IF_RXDATAV);
	USART_IntDisable(UART_UART, USART_IF_TXBL);
}

void uart_clearRxInterrupts(){
	USART_IntClear(UART_UART, USART_IF_RXDATAV);
}

void uart_clearTxInterrupts(){
	USART_IntClear(UART_UART, USART_IF_TXBL);
}

void  uart_writeByte(uint8_t byteToWrite){
	USART_Tx(UART_UART, byteToWrite);
}

uint8_t uart_readByte(){
	 int32_t i32Char = 0;
	 i32Char = USART_Rx(UART_UART);
	 return (uint8_t)(i32Char & 0xFF);
}

//=========================== interrupt handlers ==============================

static void uart_isr_private(void){
	uint32_t reg;
	debugpins_isr_set();


	debugpins_isr_clr();
}

kick_scheduler_t uart_tx_isr() {
   uart_clearTxInterrupts();
   if (uart_vars.txCb != NULL) {
       uart_vars.txCb();
   }
   return DO_NOT_KICK_SCHEDULER;
}

kick_scheduler_t uart_rx_isr() {
   uart_clearRxInterrupts();
   if (uart_vars.rxCb != NULL) {
       uart_vars.rxCb();
   }
   return DO_NOT_KICK_SCHEDULER;
}
