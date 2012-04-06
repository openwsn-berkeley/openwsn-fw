/**
\brief Cross-platform declaration "UART" specific LPCXpresso config bsp module.

\author Xavi V <watteyne@eecs.berkeley.edu>, February 2012.
*/

#ifndef UART_CONFIG_H_
#define UART_CONFIG_H_

/* Accepted Error baud rate value (in percent unit) */
//#define UART_ACCEPTED_BAUDRATE_ERROR	(3)			/*!< Acceptable UART baudrate error */
//
//#define UART_IER_RBR		0x01
//#define UART_IER_THRE	0x02
//#define UART_IER_RLS		0x04
//
//#define UART_IIR_PEND	0x01
//#define UART_IIR_RLS		0x03
//#define UART_IIR_RDA		0x02
//#define UART_IIR_CTI		0x06
//#define UART_IIR_THRE	0x01
//
//#define UART_LSR_RDR		0x01
//#define UART_LSR_OE		0x02
//#define UART_LSR_PE		0x04
//#define UART_LSR_FE		0x08
//#define UART_LSR_BI		0x10
//#define UART_LSR_THRE	0x20
//#define UART_LSR_TEMT	0x40
//#define UART_LSR_RXFE	0x80
//
///*********************************************************************//**
// * Macro defines for Macro defines for UART FIFO control register
// **********************************************************************/
//#define UART_FCR_FIFO_EN		((uint8_t)(1<<0)) 	/*!< UART FIFO enable */
//#define UART_FCR_RX_RS			((uint8_t)(1<<1)) 	/*!< UART FIFO RX reset */
//#define UART_FCR_TX_RS			((uint8_t)(1<<2)) 	/*!< UART FIFO TX reset */
//#define UART_FCR_DMAMODE_SEL 	((uint8_t)(1<<3)) 	/*!< UART DMA mode selection */
//#define UART_FCR_TRG_LEV0		((uint8_t)(0)) 		/*!< UART FIFO trigger level 0: 1 character */
//#define UART_FCR_TRG_LEV1		((uint8_t)(1<<6)) 	/*!< UART FIFO trigger level 1: 4 character */
//#define UART_FCR_TRG_LEV2		((uint8_t)(2<<6)) 	/*!< UART FIFO trigger level 2: 8 character */
//#define UART_FCR_TRG_LEV3		((uint8_t)(3<<6)) 	/*!< UART FIFO trigger level 3: 14 character */
//#define UART_FCR_BITMASK		((uint8_t)(0xCF))	/*!< UART FIFO control bit mask */
//#define UART_TX_FIFO_SIZE		(16)
//
//
///*********************************************************************//**
// * Macro defines for Macro defines for UART line control register
// **********************************************************************/
//#define UART_LCR_WLEN5     		((uint8_t)(0))   		/*!< UART 5 bit data mode */
//#define UART_LCR_WLEN6     		((uint8_t)(1<<0))   	/*!< UART 6 bit data mode */
//#define UART_LCR_WLEN7     		((uint8_t)(2<<0))   	/*!< UART 7 bit data mode */
//#define UART_LCR_WLEN8     		((uint8_t)(3<<0))   	/*!< UART 8 bit data mode */
//#define UART_LCR_STOPBIT_SEL	((uint8_t)(1<<2))   	/*!< UART Two Stop Bits Select */
//#define UART_LCR_PARITY_EN		((uint8_t)(1<<3))		/*!< UART Parity Enable */
//#define UART_LCR_PARITY_ODD		((uint8_t)(0))         	/*!< UART Odd Parity Select */
//#define UART_LCR_PARITY_EVEN	((uint8_t)(1<<4))		/*!< UART Even Parity Select */
//#define UART_LCR_PARITY_F_1		((uint8_t)(2<<4))		/*!< UART force 1 stick parity */
//#define UART_LCR_PARITY_F_0		((uint8_t)(3<<4))		/*!< UART force 0 stick parity */
//#define UART_LCR_BREAK_EN		((uint8_t)(1<<6))		/*!< UART Transmission Break enable */
//#define UART_LCR_DLAB_EN		((uint8_t)(1<<7))    	/*!< UART Divisor Latches Access bit enable */
//#define UART_LCR_BITMASK		((uint8_t)(0xFF))		/*!< UART line control bit mask */
//
//
//#define UART_TER_TXEN			((uint8_t)(1<<7)) 		/*!< Transmit enable bit */
//
//
//
//
//#define UART_LSR_BITMASK	((uint8_t)(0xFF)) 	/*!<UART Line status bit mask */
//
//#define UART_LOAD_DLM(div)  (((div) >> 8) & 0xFF)	/**< Macro for loading most significant halfs of divisors */
//#define UART_LOAD_DLL(div)	((div) & 0xFF)	/**< Macro for loading least significant halfs of divisors */
//#define UART_LCR_BITMASK		((uint8_t)(0xFF))		/*!< UART line control bit mask */
//
//#define UART_FDR_DIVADDVAL(n)	((uint32_t)(n&0x0F))		/**< Baud-rate generation pre-scaler divisor */
//#define UART_FDR_MULVAL(n)		((uint32_t)((n<<4)&0xF0))	/**< Baud-rate pre-scaler multiplier value */
//#define UART_FDR_BITMASK		((uint32_t)(0xFF))			/**< UART Fractional Divider register bit mask */
//
//#define PORTNUM_0 1 //set to 1 if you want to use port 0
//#define PORTNUM_1 1 //set to 1 if you tant to use port 1


#endif /* UART_CONFIG_H_ */
