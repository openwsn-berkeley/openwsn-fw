/**
\brief Definition of the "opentimers2" driver.

This driver uses a single hardware timer, which it virtualizes to support
at most MAX_NUM_TIMERS timers.

\author Tengfei Chang <tengfei.chang@inria.fr>, April 2017.
 */

#include "opendefs.h"
#include "opentimers2.h"
#include "sctimer.h"
#include "leds.h"

//=========================== define ==========================================

//=========================== variables =======================================

opentimers2_vars_t opentimers2_vars;
//uint32_t counter; //counts the elapsed time.

//=========================== prototypes ======================================

void opentimers2_timer_callback(void);

//=========================== public ==========================================

/**
\brief Initialize this module.

Initializes data structures and hardware timer.
 */
void opentimer2_init(){
   // initialize local variables
   memset(&opentimers2_vars,0,sizeof(opentimers2_vars_t));

   // set callback for sctimer module
   sctimer_set_callback(opentimers2_timer_callback);
} 

opentimer2_id_t opentimer2_create(void){
    return 0;
}

void            opentimer2_scheduleRelative(opentimer2_id_t     id, 
                                            uint32_t            duration,
                                            uint_type_t         uint_type, 
                                            opentimers2_cbt     cb){
    
}

void            opentimer2_scheduleAbsolute(opentimer2_id_t     id, 
                                            uint32_t            duration, 
                                            uint32_t            reference , 
                                            uint_type_t         uint_type, 
                                            opentimers2_cbt     cb){
    
}

void            opentimer2_cancel(opentimer2_id_t id){
    
}

void            opentimer2_destroy(opentimer2_id_t id){
    
}

uint32_t        opentimer2_getValue(opentimer2_id_t id){
    return 0;
}

// ========================== callback ========================================

void opentimers2_timer_callback(void){
    
}
