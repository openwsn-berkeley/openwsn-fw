#ifndef HAL_BOARD_H
#define HAL_BOARD_H

// LEDS
#define LED_PORT_DIR	P4DIR
#define LED_PORT_SEL	P4SEL
#define LED_PORT_OUT	P4OUT
#define LED_R         	BIT5
#define LED_O         	BIT6
#define LED_G		 	BIT7

#define LED_1			BIT5
#define LED_2			BIT6
#define LED_3			BIT7

#include "msp430f5438a.h"
#include "hal_UCS.h"
#include "HAL_PMM.h"


#endif /* HAL_BOARD_H */