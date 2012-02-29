/**
\brief Cross-platform declaration "UART" specific LPCXpresso config bsp module.

\author Xavi V <watteyne@eecs.berkeley.edu>, February 2012.
*/

#ifndef UART_CONFIG_H_
#define UART_CONFIG_H_

#define IER_RBR		0x01
#define IER_THRE	0x02
#define IER_RLS		0x04

#define IIR_PEND	0x01
#define IIR_RLS		0x03
#define IIR_RDA		0x02
#define IIR_CTI		0x06
#define IIR_THRE	0x01

#define LSR_RDR		0x01
#define LSR_OE		0x02
#define LSR_PE		0x04
#define LSR_FE		0x08
#define LSR_BI		0x10
#define LSR_THRE	0x20
#define LSR_TEMT	0x40
#define LSR_RXFE	0x80



#define PORTNUM_0 1 //set to 1 if you want to use port 0
#define PORTNUM_1 1 //set to 1 if you tant to use port 1


#endif /* UART_CONFIG_H_ */
