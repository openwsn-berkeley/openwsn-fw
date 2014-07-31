/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:11:00.316335.
*/
#include "openwsn_obj.h"
#include "opencoap_obj.h"
#include "opentimers_obj.h"
#include "openqueue_obj.h"
#include "packetfunctions_obj.h"
#include "openserial_obj.h"
#include "openrandom_obj.h"
#include "scheduler_obj.h"
#include "IEEE802154E_obj.h"
#include "idmanager_obj.h"

// include layer files to debug
#include "neighbors_obj.h"
#include "schedule_obj.h"
//=========================== defines =========================================

/// inter-packet period (in ms)
#define DEBUGPERIODNBS    11000
#define DEBUGPERIODSCH    7000

const uint8_t schedule_layerdebug_path0[]  = "d_s"; // debug/scheduling
const uint8_t neighbors_layerdebug_path0[] = "d_n"; // debug/neighbours

//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t schdesc;    ///< descriptor for shedule table
   coap_resource_desc_t nbsdesc;    ///< descriptor for neigbour table
   opentimer_id_t       schtimerId; ///< schedule timer
   opentimer_id_t       nbstimerId; ///< neigbour timer
} layerdebug_vars_t;

layerdebug_vars_t layerdebug_vars;

//=========================== prototypes ======================================

owerror_t layerdebug_schedule_receive(OpenMote* self, OpenQueueEntry_t* msg,
                    coap_header_iht*  coap_header,
                    coap_option_iht*  coap_options);


owerror_t layerdebug_neighbors_receive(OpenMote* self, OpenQueueEntry_t* msg,
                    coap_header_iht*  coap_header,
                    coap_option_iht*  coap_options);

void layerdebug_timer_schedule_cb(OpenMote* self);
void layerdebug_timer_neighbors_cb(OpenMote* self);

void layerdebug_task_schedule_cb(OpenMote* self);
void layerdebug_task_neighbors_cb(OpenMote* self);

void layerdebug_sendDone(OpenMote* self, OpenQueueEntry_t* msg,
                       owerror_t error);

//=========================== public ==========================================

void layerdebug_init(OpenMote* self) {
   return;
}

//=========================== private =========================================

//timer fired, but we don't want to execute task in ISR mode
//instead, push task to scheduler with COAP priority, and let scheduler take care of it
void layerdebug_timer_schedule_cb(OpenMote* self){
 scheduler_push_task(self, layerdebug_task_schedule_cb,TASKPRIO_COAP);
}

void layerdebug_timer_neighbors_cb(OpenMote* self){
 scheduler_push_task(self, layerdebug_task_neighbors_cb,TASKPRIO_COAP);
}

//schedule stats
void layerdebug_task_schedule_cb(OpenMote* self) {
   return;
}

//neighbours stats
void layerdebug_task_neighbors_cb(OpenMote* self) {
   return;
}

void layerdebug_sendDone(OpenMote* self, OpenQueueEntry_t* msg, owerror_t error) {
 openqueue_freePacketBuffer(self, msg);
}


owerror_t layerdebug_schedule_receive(OpenMote* self, OpenQueueEntry_t* msg,
                      coap_header_iht* coap_header,
                      coap_option_iht* coap_options) {
   return E_FAIL;
}

owerror_t layerdebug_neighbors_receive(OpenMote* self, OpenQueueEntry_t* msg,
                      coap_header_iht* coap_header,
                      coap_option_iht* coap_options) {
   return E_FAIL;
}
