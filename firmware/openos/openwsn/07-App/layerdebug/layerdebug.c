#include "openwsn.h"
#include "layerdebug.h"
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

void    layerdebug_timer_schedule_cb();
void    layerdebug_timer_neighbors_cb();

void    layerdebug_task_schedule_cb();
void    layerdebug_task_neighbors_cb();

void    layerdebug_sendDone(OpenQueueEntry_t* msg,
                       owerror_t error);

//=========================== public ==========================================

void layerdebug_init() {
   
   // prepare the resource descriptor for the scheduling path
   layerdebug_vars.schdesc.path0len             = sizeof(schedule_layerdebug_path0)-1;
   layerdebug_vars.schdesc.path0val             = (uint8_t*)(&schedule_layerdebug_path0);
   layerdebug_vars.schdesc.path1len             = 0;
   layerdebug_vars.schdesc.path1val             = NULL;
   layerdebug_vars.schdesc.componentID          = COMPONENT_LAYERDEBUG;
   layerdebug_vars.schdesc.callbackRx           = &layerdebug_schedule_receive;
   layerdebug_vars.schdesc.callbackSendDone     = &layerdebug_sendDone;
   opencoap_register(&layerdebug_vars.schdesc);
    
   layerdebug_vars.schtimerId     = opentimers_start(DEBUGPERIODSCH,
                                                     TIMER_PERIODIC,TIME_MS,
                                                     layerdebug_timer_schedule_cb);
   
   // prepare the resource descriptor for the neighbors path
   layerdebug_vars.nbsdesc.path0len             = sizeof(neighbors_layerdebug_path0)-1;
   layerdebug_vars.nbsdesc.path0val             = (uint8_t*)(&neighbors_layerdebug_path0);
   layerdebug_vars.nbsdesc.path1len             = 0;
   layerdebug_vars.nbsdesc.path1val             = NULL;
   layerdebug_vars.nbsdesc.componentID          = COMPONENT_LAYERDEBUG;
   layerdebug_vars.nbsdesc.callbackRx           = &layerdebug_neighbors_receive;
   layerdebug_vars.nbsdesc.callbackSendDone     = &layerdebug_sendDone;
   opencoap_register(&layerdebug_vars.nbsdesc);
   
   layerdebug_vars.nbstimerId     = opentimers_start(DEBUGPERIODNBS,
                                                     TIMER_PERIODIC,TIME_MS,
                                                     layerdebug_timer_neighbors_cb);
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
   OpenQueueEntry_t* pkt;
   owerror_t           outcome;
   uint8_t           numOptions;
   uint8_t           size;

   // don't run if not synch
   if (ieee154e_isSynch() == FALSE) return;
   
    // don't run on dagroot
   if (idmanager_getIsDAGroot()) {
       opentimers_stop( layerdebug_vars.schtimerId);
       opentimers_stop( layerdebug_vars.nbstimerId);
       return;
   }
   
   // create a CoAP RD packet
   pkt = openqueue_getFreePacketBuffer(COMPONENT_LAYERDEBUG);
   if (pkt==NULL) {
      openserial_printError(COMPONENT_LAYERDEBUG,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      openqueue_freePacketBuffer(pkt);
      return;
   }
   // take ownership over that packet
   pkt->creator    = COMPONENT_LAYERDEBUG;
   pkt->owner      = COMPONENT_LAYERDEBUG;
   // CoAP payload
   size=sizeof(netDebugScheduleEntry_t)*MAXACTIVESLOTS;
   packetfunctions_reserveHeaderSize(pkt,size);//reserve for some schedule entries
   //get the schedule information from the mac layer 
   schedule_getNetDebugInfo((netDebugScheduleEntry_t*)pkt->payload);
   
   packetfunctions_reserveHeaderSize(pkt,1);//reserve for the size of schedule entries
   pkt->payload[0] = MAXACTIVESLOTS;
  
   
   numOptions = 0;
   // location-path option
   packetfunctions_reserveHeaderSize(pkt,sizeof(schedule_layerdebug_path0)-1);
   memcpy(&pkt->payload[0],&schedule_layerdebug_path0,sizeof(schedule_layerdebug_path0)-1);
   packetfunctions_reserveHeaderSize(pkt,1);
   pkt->payload[0]                  = (COAP_OPTION_NUM_URIPATH) << 4 |
      sizeof(schedule_layerdebug_path0)-1;
   numOptions++;
   // content-type option
   packetfunctions_reserveHeaderSize(pkt,2);
   pkt->payload[0]                  = COAP_OPTION_NUM_CONTENTFORMAT << 4 |
      1;
   pkt->payload[1]                  = COAP_MEDTYPE_APPOCTETSTREAM;
   numOptions++;
   // metadata
   pkt->l4_destination_port         = WKP_UDP_COAP;
   pkt->l3_destinationAdd.type = ADDR_128B;
   memcpy(&pkt->l3_destinationAdd.addr_128b[0],&ipAddr_local,16);
   // send
   outcome = opencoap_send(pkt,
                           COAP_TYPE_NON,
                           COAP_CODE_REQ_PUT,
                           numOptions,
                           &layerdebug_vars.schdesc);
   // avoid overflowing the queue if fails
   if (outcome==E_FAIL) {
      openqueue_freePacketBuffer(pkt);
   }
   
   return;
}

//neighbours stats
void layerdebug_task_neighbors_cb() {
  
   OpenQueueEntry_t* pkt;
   owerror_t           outcome;
   uint8_t           numOptions;
   uint8_t           size;
   
   // create a CoAP RD packet
   pkt = openqueue_getFreePacketBuffer(COMPONENT_LAYERDEBUG);
   if (pkt==NULL) {
      openserial_printError(COMPONENT_LAYERDEBUG,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      openqueue_freePacketBuffer(pkt);
      return;
   }
   // take ownership over that packet
   pkt->creator    = COMPONENT_LAYERDEBUG;
   pkt->owner      = COMPONENT_LAYERDEBUG;
   // CoAP payload

   size=neighbors_getNumNeighbors(); //compute the number of neigbours sent   
   packetfunctions_reserveHeaderSize(pkt,size*sizeof(netDebugNeigborEntry_t));//reserve for the size of schedule entries
  
   debugNetPrint_neighbors((netDebugNeigborEntry_t*) pkt->payload);
   
   //now we know the size of the neihbours. Put it on the packet.
   packetfunctions_reserveHeaderSize(pkt,1);//reserve for the size of neighbours entries
   pkt->payload[0] = size;

   //coap options
   numOptions = 0;
   // location-path option
   packetfunctions_reserveHeaderSize(pkt,sizeof(neighbors_layerdebug_path0)-1);
   memcpy(&pkt->payload[0],&neighbors_layerdebug_path0,sizeof(neighbors_layerdebug_path0)-1);
   packetfunctions_reserveHeaderSize(pkt,1);
   pkt->payload[0]                  = (COAP_OPTION_NUM_URIPATH) << 4 |
      sizeof(neighbors_layerdebug_path0)-1;
   numOptions++;
   // content-type option
   packetfunctions_reserveHeaderSize(pkt,2);
   pkt->payload[0]                  = COAP_OPTION_NUM_CONTENTFORMAT << 4 |
      1;
   pkt->payload[1]                  = COAP_MEDTYPE_APPOCTETSTREAM;
   numOptions++;
   // metadata
   pkt->l4_destination_port         = WKP_UDP_COAP;
   pkt->l3_destinationAdd.type = ADDR_128B;
   memcpy(&pkt->l3_destinationAdd.addr_128b[0],&ipAddr_local,16);
   // send
   outcome = opencoap_send(pkt,
                           COAP_TYPE_NON,
                           COAP_CODE_REQ_PUT,
                           numOptions,
                           &layerdebug_vars.nbsdesc);
   // avoid overflowing the queue if fails
   if (outcome==E_FAIL) {
      openqueue_freePacketBuffer(pkt);
   }
   
   return;
}

void layerdebug_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openqueue_freePacketBuffer(msg);
}


owerror_t layerdebug_schedule_receive(OpenQueueEntry_t* msg,
                      coap_header_iht* coap_header,
                      coap_option_iht* coap_options) {
   owerror_t outcome;
   uint8_t size;
  
   
  if (coap_header->Code==COAP_CODE_REQ_GET) {
      // return current schedule value
      
      // reset packet payload
      msg->payload                     = &(msg->packet[127]);
      msg->length                      = 0;
           
      size=sizeof(netDebugScheduleEntry_t)*MAXACTIVESLOTS;
      packetfunctions_reserveHeaderSize(msg,size);//reserve for some schedule entries
      //get the schedule information from the mac layer 
      schedule_getNetDebugInfo((netDebugScheduleEntry_t*)msg->payload);

      packetfunctions_reserveHeaderSize(msg,2);
      msg->payload[0] = COAP_PAYLOAD_MARKER;
      msg->payload[1] = MAXACTIVESLOTS;
           
      // set the CoAP header
      coap_header->Code                = COAP_CODE_RESP_CONTENT;
      
      outcome                          = E_SUCCESS;
   
   } else {
      // return an error message
      outcome = E_FAIL;
   }
   
   return outcome;
}

owerror_t layerdebug_neighbors_receive(OpenQueueEntry_t* msg,
                      coap_header_iht* coap_header,
                      coap_option_iht* coap_options) {
   owerror_t outcome;
   uint8_t size;
  
   
  if (coap_header->Code==COAP_CODE_REQ_GET) {
      // return current schedule value
      
      // reset packet payload
      msg->payload                     = &(msg->packet[127]);
      msg->length                      = 0;
           
      size=neighbors_getNumNeighbors(); //compute the number of neigbours sent   
      packetfunctions_reserveHeaderSize(msg,size*sizeof(netDebugNeigborEntry_t));//reserve for the size of schedule entries
  
      debugNetPrint_neighbors((netDebugNeigborEntry_t*)msg->payload);
    
      //now we know the size of the neighbours. Put it on the packet.
      packetfunctions_reserveHeaderSize(msg,2);
      msg->payload[0] = COAP_PAYLOAD_MARKER;
      msg->payload[1] = size;
           
      // set the CoAP header
      coap_header->Code                = COAP_CODE_RESP_CONTENT;
      
      outcome                          = E_SUCCESS;
   
   } else {
      // return an error message
      outcome = E_FAIL;
   }
   
   return outcome;
}