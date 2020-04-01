 /**
 * Author: Tamas Harczos (tamas.harczos@imms.de)
 * Date:   Apr 2018
 * Description: nRF52840-specific definition of the "uart" bsp module.
 */


#include "sdk/components/boards/boards.h"
#include "sdk/components/libraries/uart/app_uart.h"
#include "sdk/modules/nrfx/hal/nrf_uart.h"
#include "sdk/integration/nrfx/legacy/nrf_drv_clock.h"
#include "sdk/modules/nrfx/drivers/include/nrfx_systick.h"

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
   bool        fXonXoffEscaping;
   uint8_t     xonXoffEscapedByte;
} uart_vars_t;

uart_vars_t uart_vars;

//=========================== prototypes ======================================

static void uart_event_handler(app_uart_evt_t * p_event);

//=========================== public ==========================================

static app_uart_comm_params_t const app_config =
{
  .rx_pin_no= RX_PIN_NUMBER,
  .tx_pin_no= TX_PIN_NUMBER,
  .rts_pin_no= RTS_PIN_NUMBER,                // defaults to UART_PIN_DISCONNECTED
  .cts_pin_no= CTS_PIN_NUMBER,                // defaults to UART_PIN_DISCONNECTED
  .baud_rate= UART_DEFAULT_CONFIG_BAUDRATE,
  .use_parity= (UART_DEFAULT_CONFIG_PARITY != 0) ? (true) : (false),
  .flow_control= UART_DEFAULT_CONFIG_HWFC     // defaults to false
};

static void uart_reinit(void)
{ 
  // for the case that the UART has previously been initialized, uninitialize it first  
  app_uart_close();

  #define APP_UART_BUF_SIZE 128

  static uint8_t rx_buf[APP_UART_BUF_SIZE];
  static uint8_t tx_buf[APP_UART_BUF_SIZE];
  static app_uart_buffers_t app_uart_buffers=
  {
    .rx_buf= rx_buf,
    .tx_buf= tx_buf,
    .rx_buf_size= sizeof(rx_buf),
    .tx_buf_size= sizeof(tx_buf)
  };

  memset(rx_buf, 0, sizeof(rx_buf));
  memset(tx_buf, 0, sizeof(tx_buf));

  // if UART cannot be initialized, blink error LED for 10s, and then reset
  if (NRF_SUCCESS != app_uart_init(&app_config, &app_uart_buffers, uart_event_handler, (app_irq_priority_t) NRFX_UART_DEFAULT_CONFIG_IRQ_PRIORITY))
  {
    leds_error_blink();
    board_reset();
  }
}

void uart_init(void)
{ 
  // reset local variables
  memset(&uart_vars,0,sizeof(uart_vars_t));

  // UART baudrate accuracy depends on HFCLK
  // see radio.c for details on enabling HFCLK
  #define hfclk_request_timeout_us 380
  {
    nrfx_systick_state_t systick_time;
    nrfx_systick_get(&systick_time);
    nrf_drv_clock_hfclk_request(NULL);
    while ((!nrf_drv_clock_hfclk_is_running()) && (!nrfx_systick_test(&systick_time, hfclk_request_timeout_us))) {}
  }

  uart_reinit();  
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

void uart_setCTS(bool state)
{
  if (state==0x01)
  {
    app_uart_put(XON);
  }
  else
  {
    app_uart_put(XOFF);
  }
}

void uart_writeByte(uint8_t byteToWrite)
{
   if (byteToWrite==XON || byteToWrite==XOFF || byteToWrite==XONXOFF_ESCAPE) {
        uart_vars.fXonXoffEscaping     = 0x01;
        uart_vars.xonXoffEscapedByte   = byteToWrite;
        app_uart_put(XONXOFF_ESCAPE);
    } else {
        app_uart_put(byteToWrite);;
    }
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

  if ((p_event->evt_type == APP_UART_COMMUNICATION_ERROR) || (p_event->evt_type == APP_UART_FIFO_ERROR))
  {
    // handle error ...
    // leds_error_blink();
    leds_error_toggle();
    uart_reinit();  
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

void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
  // handle error ...
}
#endif // UART_DISABLED  
