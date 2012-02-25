/**
\brief Cross-platform declaration "timer" bsp module.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, February 2012.
*/


#ifndef TIMER_H_
#define TIMER_H_

/* TIME_INTERVALmS is a value to load the timer match register with to get a 1 mS delay */
#define TIME_INTERVALmS	1000 //depends on the prescaler and clock configuration TICS_PER_SECOND
#define TICS_PER_SECOND 1000000 //tics per second in the timer.

#define TIMER_COMPARE_REG0 0
#define TIMER_COMPARE_REG1 1
#define TIMER_COMPARE_REG2 2

#define TIMER_CAPTURE_REG0 0
#define TIMER_CAPTURE_REG1 1



void timer_init(uint8_t timer_num);
void timer_set_compare(uint8_t timer_num,uint8_t compareReg, uint32_t delayInMs);
void timer_set_capture(uint8_t timer_num,uint8_t captureReg);
void timer_enable( uint8_t timer_num );
void timer_reset( uint8_t timer_num );
void timer_disable( uint8_t timer_num );



#endif /* TIMER_H_ */
