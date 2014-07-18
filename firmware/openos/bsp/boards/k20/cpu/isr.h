/******************************************************************************
* File:    isr.h
* Purpose: Define interrupt service routines referenced by the vector table.
* Note: Only "vectors.c" should include this header file.
******************************************************************************/

#ifndef __ISR_H
#define __ISR_H 1

#include <stdint.h>
/*
   ISR prototypes
 */
void ftm0_isr(void);
void lptmr_isr(void);
void llwu_isr(void);
void uart_isr(void);
void _spi_isr(void);  //defined in spi.h. for compatibility reasons is kept there.
void radio_external_port_c_isr(); //radio interrupt on external port ptc11s

#undef VECTOR_037
#define VECTOR_037 llwu_isr

#undef VECTOR_042
#define VECTOR_042 _spi_isr

#undef VECTOR_078
#define VECTOR_078 ftm0_isr

#undef VECTOR_063
#define VECTOR_063 uart_isr

#undef VECTOR_101
#define VECTOR_101 lptmr_isr

#undef VECTOR_107
#define VECTOR_107 radio_external_port_c_isr

#undef VECTOR_105
#define VECTOR_105 radio_external_port_c_isr


#endif  //__ISR_H

/* End of "isr.h" */
