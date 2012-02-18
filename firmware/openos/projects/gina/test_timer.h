#ifndef __TEST_TIMER_H
#define __TEST_TIMER_H

#ifdef TEST_TIMER
#define MAIN_TEST_TIMER main
#else
#define MAIN_TEST_TIMER main_test_timer
#endif

int MAIN_TEST_TIMER(void);

#endif