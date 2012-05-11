


/*
 * File:        lptmr.h
 * Purpose:     Provide common low power timer functions
 *
 * Notes:
 */

#ifndef __LPTMR_H__
#define __LPTMR_H__

#include <board_info.h>
/********************************************************************/
/* Miscellaneous defined */

#define LPTMR_USE_IRCLK 0 
#define LPTMR_USE_LPOCLK 1
#define LPTMR_USE_ERCLK32 2
#define LPTMR_USE_OSCERCLK 3


typedef uint8_t (*lptmr_cbt)(void);

typedef enum{
	LPTMR_BSP_COMPARE = 0,
	LPTMR_RADIO_OVERFLOW,
	LPTMR_RADIO_COMPARE,
	LPTMR_BSP_MAX	
}lptmr_source_t;


/* Function prototypes */

extern void lptmr_isr(void);

void lptmr_init(uint8_t clock_source);
void lptmr_set_isr_callback (lptmr_source_t type,lptmr_cbt cb);

void lptmr_disable();
PORT_TIMER_WIDTH lptmr_get_current_value();
void lptmr_enable();
void lptmr_reset_counter();

void lptmr_set_compare_bsp(PORT_TIMER_WIDTH count);
void lptmr_set_compare_radio(PORT_TIMER_WIDTH count);
void lptmr_set_overflow_radio(PORT_TIMER_WIDTH count);


void lptmr_reset_compare_bsp();
void lptmr_reset_compare_radio();
void lptmr_reset_overflow_radio();
/********************************************************************/

#endif /* __LPTMR_H__ */


