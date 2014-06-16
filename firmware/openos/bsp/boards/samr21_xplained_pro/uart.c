/* === INCLUDES ============================================================ */
#include "compiler.h"
#include "samr21_xplained_pro.h"
/* From OpenWSN */
#include "uart.h" 
/* From Atmel */
#include "usart.h"
#include "usart_interrupt.h"

/* === MACROS ============================================================== */
#define USART_HOST_BAUDRATE  (115200)
#define USART_HOST           EDBG_CDC_MODULE

/* === GLOBALS ============================================================= */
typedef struct {
	uart_tx_cbt txCb;
	uart_rx_cbt rxCb;
	uint8_t     startOrend;
	uint8_t     flagByte;
} uart_vars_t;

static struct usart_module cdc_uart_module;
uart_vars_t uart_vars;

volatile uint8_t uart_rx_byte;

void usart_write_callback(const struct usart_module *const usart_module);
void usart_read_callback(const struct usart_module *const usart_module);

/*
 * @brief uart_init Initialize the UART Functions with required baudrate 
 *
 * @param None
 *
 */
void uart_init(void)
{
	struct usart_config cdc_uart_config;
	/* Configure USART for unit test output */
	usart_get_config_defaults(&cdc_uart_config);
	cdc_uart_config.run_in_standby = false;
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
}

/*
 * @brief uart_setCallbacks register the callbacks
 * @param txCb uart tx call back function pointer
 * @param rxCb uart rx call back function pointer 
 *
 *
 */
void uart_setCallbacks(uart_tx_cbt txCb, uart_rx_cbt rxCb)
{
	/* Register Callbacks */
	uart_vars.txCb = txCb;
	uart_vars.rxCb = rxCb;	
	usart_enable_callback(&cdc_uart_module, USART_CALLBACK_BUFFER_TRANSMITTED);
    usart_enable_callback(&cdc_uart_module, USART_CALLBACK_BUFFER_RECEIVED);
}

/*
 * @brief usart_write_callback usart write callback 
 *        from the TXC interrupt
 *
 * @param usart_module initialized usart_module pointer
 *
 */
void usart_write_callback(const struct usart_module *const usart_module)
{
	uart_tx_isr();
}

/*
 * @brief usart_read_callback usart read callback 
 *        from the UART RXC interrupt
 *
 * @param usart_module initialized usart_module pointer
 *
 */
void usart_read_callback(const struct usart_module *const usart_module)
{
   uart_rx_isr();
}

/*
 * @brief uart_enableInterrupts This function will enable
 *        usart TXC and RXC interrupts
 *
 * @param None
 *
 */
void uart_enableInterrupts(void)
{
  usart_enable_interrupt(&cdc_uart_module);
}

/*
 * @brief uart_disableInterrupts This function will disable
 *        usart TXC and RXC interrupts
 *
 * @param None
 *
 */
void uart_disableInterrupts(void)
{
  usart_disable_interrupt(&cdc_uart_module);
}

/*
 * @brief uart_writeByte This function will write the
 *        one byte to usart Data Register
 *
 * @param None
 *
 */
void uart_writeByte(uint8_t byteToWrite)
{  
  usart_write_byte(&cdc_uart_module, byteToWrite);
}

/*
 * @brief uart_readByte This function will read
 *        one byte from usart Data Register
 *
 * @param None
 *
 */
uint8_t uart_readByte(void)
{
  /* Return the received data */
  return uart_rx_byte;  
}

/*
 * @brief uart_clearTxInterrupts This function will clear
 *        usart TXC interrupt flag
 *
 * @param None
 *
 */
void uart_clearTxInterrupts(void)
{
  usart_abort_job(&cdc_uart_module, USART_TRANSCEIVER_TX);
}

/*
 * @brief uart_clearRxInterrupts This function will clear
 *        usart RXC interrupt flag
 *
 * @param None
 *
 */
void uart_clearRxInterrupts(void)
{
  usart_abort_job(&cdc_uart_module, USART_TRANSCEIVER_RX);	
}

/*
 * @brief uart_tx_isr This function will be called from usart
 *        interrupt when TXC interrupt flag is set
 *
 * @param return kick_scheduler_t
 *
 */
kick_scheduler_t uart_tx_isr(void) 
{
  uart_clearTxInterrupts();
  if(uart_vars.txCb != NULL)
  {
	uart_vars.txCb();    
  } 
 return DO_NOT_KICK_SCHEDULER;  
}

/*
 * @brief uart_rx_isr This function will be called from usart
 *        interrupt when RTXC interrupt flag is set
 *
 * @param return kick_scheduler_t
 *
 */
kick_scheduler_t uart_rx_isr(void)
{
	uart_clearRxInterrupts();
	if (uart_vars.rxCb != NULL)
	{
		uart_vars.rxCb();  
	}
  return DO_NOT_KICK_SCHEDULER;  
}
