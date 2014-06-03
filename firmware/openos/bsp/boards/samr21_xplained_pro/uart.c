#include "compiler.h"
#include "samr21_xplained_pro.h"
#include "uart.h"  //From OpenWSN
#include "usart.h" //From Atmel
#include "usart_interrupt.h"

volatile uint8_t uart_rx_byte;


typedef struct {
	uart_tx_cbt txCb;
	uart_rx_cbt rxCb;
	uint8_t     startOrend;
	uint8_t     flagByte;
} uart_vars_t;

#define USART_HOST_BAUDRATE (115200)
#define USART_HOST           EDBG_CDC_MODULE

static struct usart_module cdc_uart_module;
uart_vars_t uart_vars;

void usart_write_callback(const struct usart_module *const usart_module);
void usart_read_callback(const struct usart_module *const usart_module);

void uart_init(void)
{
	uint16_t rxd_data;
	struct usart_config cdc_uart_config;
	/* Configure USART for unit test output */
	usart_get_config_defaults(&cdc_uart_config);
	cdc_uart_config.run_in_standby = true;
	cdc_uart_config.mux_setting = EDBG_CDC_SERCOM_MUX_SETTING;
	cdc_uart_config.pinmux_pad0 = EDBG_CDC_SERCOM_PINMUX_PAD0;
	cdc_uart_config.pinmux_pad1 = EDBG_CDC_SERCOM_PINMUX_PAD1;
	cdc_uart_config.pinmux_pad2 = EDBG_CDC_SERCOM_PINMUX_PAD2;
	cdc_uart_config.pinmux_pad3 = EDBG_CDC_SERCOM_PINMUX_PAD3;
	cdc_uart_config.baudrate    = USART_HOST_BAUDRATE;
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
	/* Dummy Read to Enable the RXC Interrupt */
	usart_read_job(&cdc_uart_module, &rxd_data);	
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
  uint16_t byte_to_write = byteToWrite;
  usart_write_job(&cdc_uart_module, byte_to_write);
}

uint8_t uart_readByte(void)
{
 //uint8_t data = uart_rx_byte;

//uint16_t rxd_data;
  //if (cdc_uart_module.remaining_rx_buffer_length == UART_RX_BUF_MAX_SIZE)
  //{
	  ///* No Bytes are available for read */
	  //return 0;
  //}
  //else
  //{
	  //rxd_data = uart_rx_buf[0];
	  //cdc_uart_module.rx_buffer_ptr = uart_rx_buf;
	  //cdc_uart_module.remaining_rx_buffer_length++;	  
	  //return (uint8_t)rxd_data;
  //}
//if (STATUS_OK == usart_read_job(&cdc_uart_module, &rxd_data))
//{
	///* Got one byte */
	//return ((uint8_t)rxd_data);
//}
  /* Failed to retrieve the data */
  return uart_rx_byte;  
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
