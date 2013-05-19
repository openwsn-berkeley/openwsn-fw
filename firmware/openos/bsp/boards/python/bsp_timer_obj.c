/**
\brief PC-specific definition of the "bsp_timer" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, April 2012.
*/

#include <string.h>
#include "bsp_timer_obj.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   bsp_timer_cbt    cb;
   PORT_TIMER_WIDTH last_compare_value;
} bsp_timer_vars_t;

bsp_timer_vars_t bsp_timer_vars;

//=========================== prototypes ======================================

//=========================== callbacks =======================================

void bsp_timer_set_callback(OpenMote* self, bsp_timer_cbt cb) {
   bsp_timer_vars.cb   = cb;
}

//=========================== public ==========================================

void bsp_timer_init(OpenMote* self) {
   
   // clear local variables
   memset((void*)&bsp_timer_vars,0,sizeof(bsp_timer_vars_t));
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_bsp_timer_init,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}

void bsp_timer_reset(OpenMote* self) {
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_bsp_timer_reset,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}

void bsp_timer_scheduleIn(OpenMote* self, PORT_TIMER_WIDTH delayTicks) {
   //opensim_requ_bsp_timer_scheduleIn_t reqparams;
   
   // prepare params
   //reqparams.delayTicks = delayTicks;
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_bsp_timer_scheduleIn,
                                    &reqparams,
                                    sizeof(opensim_requ_bsp_timer_scheduleIn_t),
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}

void bsp_timer_cancel_schedule(OpenMote* self) {
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_bsp_timer_cancel_schedule,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}

PORT_TIMER_WIDTH bsp_timer_get_currentValue(OpenMote* self) {
   //opensim_repl_bsp_timer_get_currentValue_t replparams;

   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_bsp_timer_get_currentValue,
                                    0,
                                    0,
                                    &replparams,
                                    sizeof(opensim_repl_bsp_timer_get_currentValue_t));
   */
   // TODO: replace by call to Python
   
   //return replparams.value;
   return 0; //poipoi
}
//=========================== private =========================================

//=========================== interrupt handlers ==============================

kick_scheduler_t bsp_timer_isr(OpenMote* self) {
   bsp_timer_vars.cb(self);
   return 0;//poipoi
}