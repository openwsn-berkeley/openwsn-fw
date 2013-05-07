/**
\brief PC-specific definition of the "radiotimer" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, April 2012.
*/

#include "radiotimer.h"

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
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radiotimer_init,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}

void radiotimer_start(uint16_t period) {
   /*
   opensim_requ_radiotimer_start_t requparams;
   
   // prepare params
   requparams.period = period;
   
   // send request to server and get reply
   
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radiotimer_start,
                                    &requparams,
                                    sizeof(opensim_requ_radiotimer_start_t),
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}

//===== direct access

uint16_t radiotimer_getValue() {
   /*
   opensim_repl_radiotimer_getValue_t replparams;
   
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radiotimer_getValue,
                                    0,
                                    0,
                                    &replparams,
                                    sizeof(opensim_repl_radiotimer_getValue_t));
   // TODO: replace by call to Python
   
   return replparams.value;
   */
   return 0;//poipoi
}

void radiotimer_setPeriod(uint16_t period) {
   /*
   opensim_requ_radiotimer_setPeriod_t requparams;
   
   // prepare params
   requparams.period = period;
   
   // send request to server and get reply
   
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radiotimer_setPeriod,
                                    &requparams,
                                    sizeof(opensim_requ_radiotimer_setPeriod_t),
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}

uint16_t radiotimer_getPeriod() {
   /*
   opensim_repl_radiotimer_getPeriod_t replparams;

   // send request to server and get reply
   
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radiotimer_getPeriod,
                                    0,
                                    0,
                                    &replparams,
                                    sizeof(opensim_repl_radiotimer_getPeriod_t));
   
   // TODO: replace by call to Python
   
   return replparams.period;
   */
   return 0;//poipoi
}

//===== compare

void radiotimer_schedule(uint16_t offset) {
   /*
   opensim_requ_radiotimer_schedule_t requparams;
   
   // prepare params
   requparams.offset = offset;
   
   // send request to server and get reply
   
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radiotimer_schedule,
                                    &requparams,
                                    sizeof(opensim_requ_radiotimer_schedule_t),
                                    0,
                                    0);
                                    
   */
   // TODO: replace by call to Python
}

void radiotimer_cancel() {
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radiotimer_cancel,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}

//===== capture

uint16_t radiotimer_getCapturedTime() {
   /*
   opensim_repl_radiotimer_getCapturedTime_t replparams;

   // send request to server and get reply
   
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radiotimer_getCapturedTime,
                                    0,
                                    0,
                                    &replparams,
                                    sizeof(opensim_repl_radiotimer_getCapturedTime_t));
   
   // TODO: replace by call to Python
   
   return replparams.capturedTime;
   */
   return 0;//poipoi
}

//=========================== interrupt handlers ==============================

void radiotimer_intr_compare() {
   radiotimer_vars.compare_cb();
}

void radiotimer_intr_overflow() {
   radiotimer_vars.overflow_cb();
}

//=========================== private =========================================