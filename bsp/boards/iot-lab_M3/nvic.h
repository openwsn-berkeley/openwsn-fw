/**
\brief openmoteSTM32 definition of the NVIC.

\author Chang Tengfei <tengfei.chang@gmail.com>,  July 2012.
*/

#ifndef __NVIC_H
#define __NVIC_H

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//initial NVIC on openmotestm32
void NVIC_init(void);

//configuration of interrupt on openmotestm32
void NVIC_Configuration(void);

//configuration module interrput on openmotestm32 
void NVIC_uart(void);
void NVIC_spi(void);
void NVIC_bsptimer(void);
void NVIC_rtctimer(void);
void NVIC_radiotimer(void);
void NVIC_radio(void);

#endif