


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

typedef uint8_t (*low_timer_hook)(void);


/* Function prototypes */

extern void lptmr_isr(void);
void lptmr_init(uint8_t clock_source);
void lptmr_set_isr_compare_hook(low_timer_hook cbt);

void lptmr_disable();
void lptmr_reset_compare();
PORT_TIMER_WIDTH lptmr_get_current_value();
void lptmr_set_compare(PORT_TIMER_WIDTH count);
void lptmr_enable();

/********************************************************************/

#endif /* __LPTMR_H__ */


