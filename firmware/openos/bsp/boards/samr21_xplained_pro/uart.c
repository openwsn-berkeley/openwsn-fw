#include "compiler.h"
#include "samr21_xplained_pro.h"
#include "uart.h"  //From OpenWSN
#include "usart.h" //From Atmel
#include "usart_interrupt.h"




typedef struct {
	uart_tx_cbt txCb;
	uart_rx_cbt rxCb;
	uint8_t     startOrend;
	uint8_t     flagByte;
} uart_vars_t;



#define USART_HOST_BAUDRATE (115200)
#define USART_HOST                 EDBG_CDC_MODULE

static struct usart_module cdc_uart_module;
uart_vars_t uart_vars;

void usart_write_callback(const struct usart_module *const usart_module);
void usart_read_callback(const struct usart_module *const usart_module);

void uart_init(void)
{
	struct usart_config cdc_uart_config;
	/* Configure USART for unit test output */
	usart_get_config_defaults(&cdc_uart_config);
	cdc_uart_config.run_in_standby = true;
	cdc_uart_config.mux_setting = EDBG_CDC_SERCOM_MUX_SETTING;
	cdc_uart_config.pinmux_pad0 = EDBG_CDC_SERCOM_PINMUX_PAD0;
	cdc_uart_config.pinmux_pad1 = EDBG_CDC_SERCOM_PINMUX_PAD1;
	cdc_uart_config.pinmux_pad2 = EDBG_CDC_SERCOM_PINMUX_PAD2;
	cdc_uart_config.pinmux_pad3 = EDBG_CDC_SERCOM_PINMUX_PAD3;
	cdc_uart_config.baudrate        = USART_HOST_BAUDRATE;
	while(usart_init(&cdc_uart_module, USART_HOST,&cdc_uart_config) != STATUS_OK){
		}
	
	/* Enable USART & transceivers */
	usart_enable(&cdc_uart_module);
	
	usart_register_callback(&cdc_uart_module,
								usart_write_callback, USART_CALLBACK_BUFFER_TRANSMITTED);
	usart_register_callback(&cdc_uart_module,
								usart_read_callback, USART_CALLBACK_BUFFER_RECEIVED); 
	
	usart_enable_transceiver(&cdc_uart_module, USART_TRANSCEIVER_TX);
	usart_enable_transceiver(&cdc_uart_module, USART_TRANSCEIVER_RX);
	/* Dummy Read */
	uart_readByte();
}

void uart_setCallbacks(uart_tx_cbt txCb, uart_rx_cbt rxCb)
{
	//Register Callbacks
	uart_vars.txCb = txCb;
	uart_vars.rxCb = rxCb;
}

void usart_write_callback(const struct usart_module *const usart_module)
{
	uart_tx_isr();
}

void usart_read_callback(const struct usart_module *const usart_module)
{
   uart_rx_isr();
}



void uart_enableInterrupts(void)
{
  usart_enable_callback(&cdc_uart_module, USART_CALLBACK_BUFFER_TRANSMITTED);
  usart_enable_callback(&cdc_uart_module, USART_CALLBACK_BUFFER_RECEIVED);
}

void uart_disableInterrupts(void)
{
  usart_disable_callback(&cdc_uart_module, USART_CALLBACK_BUFFER_TRANSMITTED);
  usart_disable_callback(&cdc_uart_module, USART_CALLBACK_BUFFER_RECEIVED);
}


void uart_writeByte(uint8_t byteToWrite)
{
  usart_write_buffer_job(&cdc_uart_module, &byteToWrite, 1);
}

uint8_t uart_readByte(void)
{
  uint8_t rxd_data;
  uint8_t retry = 3;
  while(retry--)
  {
	if (STATUS_OK == usart_read_buffer_job(&cdc_uart_module, &rxd_data, 1))
	{
		/* Got one byte */
		return rxd_data;
	}
  }
  /* Failed to retrieve the data */
  return 0;  
}

void uart_clearTxInterrupts(void)
{
  usart_abort_job(&cdc_uart_module, USART_TRANSCEIVER_TX);
}

void uart_clearRxInterrupts(void)
{
  //usart_abort_job(&cdc_uart_module, USART_TRANSCEIVER_RX);	
}

kick_scheduler_t uart_tx_isr(void) 
{
  if(uart_vars.txCb != NULL)
  {
	uart_vars.txCb();    
  } 
 return DO_NOT_KICK_SCHEDULER;  
}

kick_scheduler_t uart_rx_isr(void)
{
	if (uart_vars.rxCb != NULL)
	{
		uart_vars.rxCb();  
	}
  return DO_NOT_KICK_SCHEDULER;  
}
