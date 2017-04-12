/**
\brief Declaration of the "opentimers2" driver.

\author Tengfei Chang <tengfei.chang@inria.fr>, April 2017.
*/

#ifndef __OPENTIMERS_H
#define __OPENTIMERS_H

#include "opendefs.h"

/**
\addtogroup drivers
\{
\addtogroup OpenTimers2
\{
*/

//=========================== define ==========================================

/// Maximum number of timers that can run concurrently
#define MAX_NUM_TIMERS            10
#define MAX_TICKS_NUMBER          ((PORT_TIMER_WIDTH)0xFFFFFFFF)
#define TOO_MANY_TIMERS_ERROR     255
#define MAX_DURATION_ISR          33 // 33@32768Hz = 1ms
#define opentimer2_id_t uint8_t

typedef void (*opentimers2_cbt)(opentimer2_id_t id);

//=========================== typedef =========================================

/*the time can be in tics or in ms*/
typedef enum {
   TIME_MS,
   TIME_TICS,
} uint_type_t;

typedef struct {
   uint32_t             currentCompareValue;// total number of clock ticks
   uint32_t             lastCompareValue;   // the previous compare value
   bool                 isrunning;          // is running?
   opentimers2_cbt      callback;           // function to call when elapses
} opentimers2_t;

//=========================== module variables ================================

typedef struct {
   opentimers2_t        timersBuf[MAX_NUM_TIMERS];
   bool                 running;
   uint32_t             currentTimeout; // current timeout, in ticks
} opentimers2_vars_t;

//=========================== prototypes ======================================

void            opentimer2_init(void);
opentimer2_id_t opentimer2_create(void);
void            opentimer2_scheduleRelative(opentimer2_id_t     id, 
                                            uint32_t            duration,
                                            uint_type_t         uint_type, 
                                            opentimers2_cbt     cb);
void            opentimer2_scheduleAbsolute(opentimer2_id_t     id, 
                                            uint32_t            duration, 
                                            uint32_t            reference , 
                                            uint_type_t         uint_type, 
                                            opentimers2_cbt     cb);
void            opentimer2_cancel(opentimer2_id_t id);
bool            opentimer2_destroy(opentimer2_id_t id);
uint32_t        opentimer2_getValue(opentimer2_id_t id);

/**
\}
\}
*/

#endif
