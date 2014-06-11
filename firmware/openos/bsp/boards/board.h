#ifndef __BOARD_H
#define __BOARD_H

/**
\addtogroup BSP
\{
\addtogroup board
\{

\brief Cross-platform declaration "board" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "board_info.h"

//=========================== define ==========================================

typedef enum {
   DO_NOT_KICK_SCHEDULER,
   KICK_SCHEDULER,
} kick_scheduler_t;

#if defined(__GNUC__) && (__GNUC__==4)  && (__GNUC_MINOR__<=5) && defined(__MSP430__)
   // mspgcc <4.5.x
#include <signal.h>
#define ISR(v) interrupt (v ## _VECTOR) v ## _ISR(void)
#else
   // other
#define __PRAGMA__(x) _Pragma(#x)
#define ISR(v) __PRAGMA__(vector=v ##_VECTOR) __interrupt void v ##_ISR(void)
#endif

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void board_init(void);
void board_sleep(void);
void board_reset(void);

/**
\}
\}
*/

#endif
