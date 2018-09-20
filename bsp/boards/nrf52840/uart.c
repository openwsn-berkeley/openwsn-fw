 /**
 * Author: Tamas Harczos (tamas.harczos@imms.de)
 * Date:   Apr 2018
 * Description: nRF52840-specific definition of the "uart" bsp module.
 */


#include "sdk/components/boards/boards.h"
#include "sdk/components/libraries/uart/app_uart.h"

#include "board.h"
#include "leds.h"
#include "debugpins.h"
#include "uart.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifndef UART_DISABLED  

//=========================== defines =========================================

// see sdk/config/nrf52840/config/sdk_config.h for UART related settings
#if BOARD_PCA10059
// nrf52840-DONGLE
#define RX_PIN_NUMBER  47
#define TX_PIN_NUMBER  45
#define RTS_PIN_NUMBER UART_PIN_DISCONNECTED
#define CTS_PIN_NUMBER UART_PIN_DISCONNECTED
#endif


//=========================== variables =======================================

typedef struct
{
   uart_tx_cbt txCb;
   uart_rx_cbt rxCb;
} uart_vars_t;

uart_vars_t uart_vars;

//=========================== prototypes ======================================

static void uart_event_handler(app_uart_evt_t * p_event);

//=========================== public ==========================================

void uart_init(void)
{ 
  // reset local variables
  memset(&uart_vars,0,sizeof(uart_vars_t));
   
  // for the case that the UART has previously been initialized, uninitialize it first  
  app_uart_close();

  app_uart_comm_params_t const app_config =
  {
    .rx_pin_no= RX_PIN_NUMBER,
    .tx_pin_no= TX_PIN_NUMBER,
    .rts_pin_no= RTS_PIN_NUMBER,                // defaults to UART_PIN_DISCONNECTED
    .cts_pin_no= CTS_PIN_NUMBER,                // defaults to UART_PIN_DISCONNECTED
    .baud_rate= UART_DEFAULT_CONFIG_BAUDRATE,
    .use_parity= (UART_DEFAULT_CONFIG_PARITY != 0) ? (true) : (false),
    .flow_control= UART_DEFAULT_CONFIG_HWFC     // defaults to false
  };
  
  // if UART cannot be initialized, blink error LED for 10s, and then reset
  if (NRF_SUCCESS != app_uart_init(&app_config, NULL, uart_event_handler, (app_irq_priority_t) NRFX_UART_DEFAULT_CONFIG_IRQ_PRIORITY))
  {
    leds_error_blink();
    board_reset();
  }
}

void uart_setCallbacks(uart_tx_cbt txCb, uart_rx_cbt rxCb)
{
  uart_vars.txCb = txCb;
  uart_vars.rxCb = rxCb;
}

void uart_enableInterrupts(void)
{
  // nrf_uart_int_enable(..., NRF_UART_INT_MASK_TXDRDY | NRF_UART_INT_MASK_RXDRDY | NRF_UART_INT_MASK_RXTO | NRF_UART_INT_MASK_ERROR);
}

void uart_disableInterrupts(void)
{
  // nrf_uart_int_disable(..., NRF_UART_INT_MASK_TXDRDY | NRF_UART_INT_MASK_RXDRDY | NRF_UART_INT_MASK_RXTO | NRF_UART_INT_MASK_ERROR);
}

void uart_clearRxInterrupts(void)
{
  // is handled within app_uart
}

void uart_clearTxInterrupts(void)
{
  // is handled within app_uart
}

void uart_writeByte(uint8_t byteToWrite)
{
  app_uart_put(byteToWrite);
}

uint8_t uart_readByte(void)
{
  uint8_t newChar;
  app_uart_get(&newChar);
  return (newChar);
}

//=========================== interrupt handlers ==============================

void uart_event_handler(app_uart_evt_t * p_event)
{
// debugpins_isr_set();

  if ((p_event->evt_type == APP_UART_COMMUNICATION_ERROR) || (p_event->evt_type == APP_UART_FIFO_ERROR) || (p_event->evt_type == APP_UART_FIFO_ERROR))
  {
    // handle error ...
    // leds_error_blink();
    leds_error_toggle();
  }
  else if ((p_event->evt_type == APP_UART_DATA) || (p_event->evt_type == APP_UART_DATA_READY))
  {
    // RX data has been received
    uart_rx_isr();
  }
  else if (p_event->evt_type == APP_UART_TX_EMPTY)
  {
    // TX data has been sent
    uart_tx_isr();
  }

//  debugpins_isr_clr();
}


kick_scheduler_t uart_tx_isr(void)
{
  uart_clearTxInterrupts();

  if (uart_vars.txCb != NULL)
  {
    uart_vars.txCb();
  }

  return DO_NOT_KICK_SCHEDULER;
}

kick_scheduler_t uart_rx_isr(void)
{
  uart_clearRxInterrupts();

  if (uart_vars.rxCb != NULL)
  {
    uart_vars.rxCb();
  }

  return DO_NOT_KICK_SCHEDULER;
}

#endif // UART_DISABLED  
