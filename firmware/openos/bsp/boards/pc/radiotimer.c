/**
\brief PC-specific definition of the "radiotimer" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, April 2012.
*/

#include "radiotimer.h"
#include "opensim_proto.h"

//=========================== variables =======================================

typedef struct {
   radiotimer_compare_cbt    overflow_cb;
   radiotimer_compare_cbt    compare_cb;
} radiotimer_vars_t;

radiotimer_vars_t radiotimer_vars;

//=========================== prototypes ======================================

//=========================== callback ========================================

void radiotimer_setOverflowCb(radiotimer_compare_cbt cb) {
   radiotimer_vars.overflow_cb = cb;
}

void radiotimer_setCompareCb(radiotimer_compare_cbt cb) {
   radiotimer_vars.compare_cb = cb;
}

//=========================== public ==========================================

//===== admin

void radiotimer_init() {
   
   // clear local variables
   memset(&radiotimer_vars,0,sizeof(radiotimer_vars_t));
   
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radiotimer_init,
                                    0,
                                    0,
                                    0,
                                    0);
}

void radiotimer_start(PORT_RADIOTIMER_WIDTH period) {
   opensim_requ_radiotimer_start_t requparams;
   
   // prepare params
   requparams.period = period;
   
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radiotimer_start,
                                    &requparams,
                                    sizeof(opensim_requ_radiotimer_start_t),
                                    0,
                                    0);
}

//===== direct access

PORT_RADIOTIMER_WIDTH radiotimer_getValue() {
   opensim_repl_radiotimer_getValue_t replparams;

   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radiotimer_getValue,
                                    0,
                                    0,
                                    &replparams,
                                    sizeof(opensim_repl_radiotimer_getValue_t));
   
   return replparams.value;
}

void radiotimer_setPeriod(PORT_RADIOTIMER_WIDTH period) {
   opensim_requ_radiotimer_setPeriod_t requparams;
   
   // prepare params
   requparams.period = period;
   
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radiotimer_setPeriod,
                                    &requparams,
                                    sizeof(opensim_requ_radiotimer_setPeriod_t),
                                    0,
                                    0);
}

PORT_RADIOTIMER_WIDTH radiotimer_getPeriod() {
   opensim_repl_radiotimer_getPeriod_t replparams;

   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radiotimer_getPeriod,
                                    0,
                                    0,
                                    &replparams,
                                    sizeof(opensim_repl_radiotimer_getPeriod_t));
   
   return replparams.period;
}

//===== compare

void radiotimer_schedule(PORT_RADIOTIMER_WIDTH offset) {
   opensim_requ_radiotimer_schedule_t requparams;
   
   // prepare params
   requparams.offset = offset;
   
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radiotimer_schedule,
                                    &requparams,
                                    sizeof(opensim_requ_radiotimer_schedule_t),
                                    0,
                                    0);
}

void radiotimer_cancel() {
   
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radiotimer_cancel,
                                    0,
                                    0,
                                    0,
                                    0);
}

//===== capture

PORT_RADIOTIMER_WIDTH radiotimer_getCapturedTime() {
   opensim_repl_radiotimer_getCapturedTime_t replparams;

   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radiotimer_getCapturedTime,
                                    0,
                                    0,
                                    &replparams,
                                    sizeof(opensim_repl_radiotimer_getCapturedTime_t));
   
   return replparams.capturedTime;
}

//=========================== interrupt handlers ==============================

void radiotimer_intr_compare() {
   radiotimer_vars.compare_cb();
}

void radiotimer_intr_overflow() {
   radiotimer_vars.overflow_cb();
}

//=========================== private =========================================