/**
\brief PC-specific definition of the "bsp_timer" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, April 2012.
*/

#include "bsp_timer.h"
#include "opensim_proto.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   bsp_timer_cbt    cb;
   PORT_TIMER_WIDTH last_compare_value;
} bsp_timer_vars_t;

bsp_timer_vars_t bsp_timer_vars;

//=========================== prototypes ======================================

//=========================== callbacks =======================================

void bsp_timer_set_callback(bsp_timer_cbt cb) {
   bsp_timer_vars.cb   = cb;
}

//=========================== public ==========================================

void bsp_timer_init() {
   
   // clear local variables
   memset(&bsp_timer_vars,0,sizeof(bsp_timer_vars_t));
   
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_bsp_timer_init,
                                    0,
                                    0,
                                    0,
                                    0);
}

void bsp_timer_reset() {
   
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_bsp_timer_reset,
                                    0,
                                    0,
                                    0,
                                    0);
}

void bsp_timer_scheduleIn(PORT_TIMER_WIDTH delayTicks) {
   opensim_requ_bsp_timer_scheduleIn_t reqparams;
   
   // prepare params
   reqparams.delayTicks = delayTicks;
   
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_bsp_timer_scheduleIn,
                                    &reqparams,
                                    sizeof(opensim_requ_bsp_timer_scheduleIn_t),
                                    0,
                                    0);
}

void bsp_timer_cancel_schedule() {
   
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_bsp_timer_cancel_schedule,
                                    0,
                                    0,
                                    0,
                                    0);
}

PORT_TIMER_WIDTH bsp_timer_get_currentValue() {
   opensim_repl_bsp_timer_get_currentValue_t replparams;

   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_bsp_timer_get_currentValue,
                                    0,
                                    0,
                                    &replparams,
                                    sizeof(opensim_repl_bsp_timer_get_currentValue_t));
   
   return replparams.value;
}
//=========================== private =========================================

//=========================== interrupt handlers ==============================