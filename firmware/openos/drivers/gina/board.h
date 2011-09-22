/**
\brief GINA's board service package

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2010
*/

#ifndef __GINA_H
#define __GINA_H

#include "openwsn.h"

//=========================== define ==========================================

#define IRQ_VECTOR   COMPARATORA_VECTOR
#define IRQ_ENABLE   CACTL1 |=  CAIE
#define IRQ_DISABLE  CACTL1 &= ~CAIE
#define IRQ_IFG_SET  CACTL1 |=  CAIFG
#define IRQ_IFG_CLR  CACTL1 &= ~CAIFG
volatile char irq_ifg;

#define IRQ_AT_RX_DONE BIT0
#define IRQ_AT_RX_WAIT BIT1
#define IRQ_USB_RX BIT2

#define IRQ_SET(val) { irq_ifg |= val; IRQ_IFG_SET; }
#define IRQ_CLR(val) { irq_ifg &= ~val; if (!irq_ifg) IRQ_IFG_CLR; }
#define IRQ_GET(val) irq_ifg & val

// Pushbutton
#  define GINA_IRQ_PIN BIT7
// Gyro
#  define GY_IRQ_PIN BIT5 
// Digital Accel
#  define XL_IRQ_PIN BIT7
// Radio
#  define AT_IRQ_PIN BIT6
#  define AT_SLP_TR_POUT P4OUT
#  define AT_SLP_TR_PDIR P4DIR
#  define AT_SLP_TR_PIN BIT7
#  define AT_SPI_POUT P4OUT
#  define AT_SPI_PDIR P4DIR
#  define AT_SPI_PIN BIT0
// Analog Accel
#  define ADC_AX  INCH_2
#  define ADC_AY  INCH_1
#  define ADC_AZ1 INCH_5
#  define ADC_AZ3 INCH_6
#  define ADC_TI  INCH_7
#  define ADC_P6S 0xE6 // 1110 0110

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void gina_init();

#endif
