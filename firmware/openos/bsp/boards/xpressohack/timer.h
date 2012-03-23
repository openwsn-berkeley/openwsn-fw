/**
\brief Cross-platform declaration "timer" bsp module.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, February 2012.
*/

#ifndef __TIMER_H
#define __TIMER_H

#include "stdint.h"

//=========================== define ==========================================

// TIMER_to_32kHz is used to temporarily convert the 1MHz timer into what looks
// like a 32kHz timer.
#define TIMER_to_32kHz       30
// ticks per second in the timer.
#define TICS_PER_SECOND      1000000
//#define TICS_PER_MS          1000
#define TICS_PER_MS          1//

#define TIMER_COMPARE_REG0   0
#define TIMER_COMPARE_REG1   1
#define TIMER_COMPARE_REG2   2

#define TIMER_CAPTURE_REG0   6
#define TIMER_CAPTURE_REG1   7

#define TIMER_NUM0           0
#define TIMER_NUM1           3
#define TIMER_NUM2           6
#define TIMER_NUM3           9


//=========================== typedef =========================================

typedef void (*low_timer_hook)(uint8_t);

//=========================== variables =======================================

//=========================== prototypes ======================================

void     timer_init(uint8_t t_num);
void     timer_set_compare(uint8_t timer_num,
                           uint8_t compareReg,
                           uint32_t delayInMs);
void     timer_set_capture(uint8_t timer_num,uint8_t captureReg);
void     timer_enable( uint8_t timer_num );
void     timer_reset( uint8_t timer_num );
void     timer_disable( uint8_t timer_num );
uint32_t timer_get_current_value(uint8_t timer_num);
void     timer_reset_compare(uint8_t timer_num,uint8_t compareReg);
uint32_t timer_get_capture_value(uint8_t timer_num,uint8_t compareReg);

void     timer_set_isr_compare_hook(uint8_t timer_num, low_timer_hook cbt);
void     timer_set_isr_capture_hook(uint8_t timer_num, low_timer_hook cbt);

#endif /* TIMER_H_ */
