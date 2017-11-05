/**
\brief iot-lab_A8-M3 definition of the NVIC.

\author Tengfei Chang <tengfei.chang@inria.fr>,  May 2017.
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
void NVIC_sctimer(void);
void NVIC_radio(void);

#endif