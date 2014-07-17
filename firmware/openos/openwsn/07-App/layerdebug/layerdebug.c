#include "openwsn.h"
#include "opencoap.h"
#include "opentimers.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "openrandom.h"
#include "scheduler.h"
#include "IEEE802154E.h"
#include "idmanager.h"

// include layer files to debug
#include "neighbors.h"
#include "schedule.h"
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

owerror_t layerdebug_schedule_receive(OpenQueueEntry_t* msg,
                    coap_header_iht*  coap_header,
                    coap_option_iht*  coap_options);


owerror_t layerdebug_neighbors_receive(OpenQueueEntry_t* msg,
                    coap_header_iht*  coap_header,
                    coap_option_iht*  coap_options);

void    layerdebug_timer_schedule_cb(void);
void    layerdebug_timer_neighbors_cb(void);

void    layerdebug_task_schedule_cb(void);
void    layerdebug_task_neighbors_cb(void);

void    layerdebug_sendDone(OpenQueueEntry_t* msg,
                       owerror_t error);

//=========================== public ==========================================

void layerdebug_init() {
   return;
}

//=========================== private =========================================

//timer fired, but we don't want to execute task in ISR mode
//instead, push task to scheduler with COAP priority, and let scheduler take care of it
void layerdebug_timer_schedule_cb(){
   scheduler_push_task(layerdebug_task_schedule_cb,TASKPRIO_COAP);
}

void layerdebug_timer_neighbors_cb(){
   scheduler_push_task(layerdebug_task_neighbors_cb,TASKPRIO_COAP);
}

//schedule stats
void layerdebug_task_schedule_cb() {
   return;
}

//neighbours stats
void layerdebug_task_neighbors_cb() {
   return;
}

void layerdebug_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openqueue_freePacketBuffer(msg);
}


owerror_t layerdebug_schedule_receive(OpenQueueEntry_t* msg,
                      coap_header_iht* coap_header,
                      coap_option_iht* coap_options) {
   return E_FAIL;
}

owerror_t layerdebug_neighbors_receive(OpenQueueEntry_t* msg,
                      coap_header_iht* coap_header,
                      coap_option_iht* coap_options) {
   return E_FAIL;
}
