#include "openwsn.h"
#include "layerdebug.h"
#include "opencoap.h"
#include "opentimers.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "openrandom.h"
#include "scheduler.h"


// include layer files to debug
#include "neighbors.h"
#include "schedule.h"
//=========================== defines =========================================

/// inter-packet period (in ms)
#define DEBUGPERIOD    10000
#define MAXPAYLOADLEN    80
#define ROUTINGDEBUG     1
//#define SCHEDULEDEBUG     1


const uint8_t layerdebug_path0[] = "ldbg"; // equ. to layerdebug

//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t desc;
   opentimer_id_t  timerId;
} layerdebug_vars_t;

layerdebug_vars_t layerdebug_vars;

//=========================== prototypes ======================================

error_t layerdebug_receive(OpenQueueEntry_t* msg,
                    coap_header_iht*  coap_header,
                    coap_option_iht*  coap_options);
void    layerdebug_timer_cb();
void    layerdebug_task_cb();
void    layerdebug_sendDone(OpenQueueEntry_t* msg,
                       error_t error);

//=========================== public ==========================================

void layerdebug_init() {
   
   // prepare the resource descriptor for the /layerdebug path
   layerdebug_vars.desc.path0len             = sizeof(layerdebug_path0)-1;
   layerdebug_vars.desc.path0val             = (uint8_t*)(&layerdebug_path0);
   layerdebug_vars.desc.path1len             = 0;
   layerdebug_vars.desc.path1val             = NULL;
   layerdebug_vars.desc.componentID          = COMPONENT_LAYERDEBUG;
   layerdebug_vars.desc.callbackRx           = &layerdebug_receive;
   layerdebug_vars.desc.callbackSendDone     = &layerdebug_sendDone;
   
#ifdef ROUTINGDEBUG
   opencoap_register(&layerdebug_vars.desc);
   layerdebug_vars.timerId    = opentimers_start(DEBUGPERIOD,
                                                TIMER_PERIODIC,TIME_MS,
                                                layerdebug_timer_cb);

#elif SCHEDULEDEBUG
   opencoap_register(&layerdebug_vars.desc);
   layerdebug_vars.timerId    = opentimers_start(DEBUGPERIOD,
                                                TIMER_PERIODIC,TIME_MS,
                                                layerdebug_timer_cb);

#endif
}

//=========================== private =========================================

error_t layerdebug_receive(OpenQueueEntry_t* msg,
                      coap_header_iht* coap_header,
                      coap_option_iht* coap_options) {
   return E_FAIL;
}

//timer fired, but we don't want to execute task in ISR mode
//instead, push task to scheduler with COAP priority, and let scheduler take care of it
void layerdebug_timer_cb(){
   scheduler_push_task(layerdebug_task_cb,TASKPRIO_COAP);
}

void layerdebug_task_cb() {
   OpenQueueEntry_t* pkt;
   error_t           outcome;
   uint8_t           numOptions;
   uint8_t           i,j;
   
   neighborRow_t *   table = NULL;
   scheduleEntry_t * schbuffer = NULL;
   // create a CoAP RD packet
   pkt = openqueue_getFreePacketBuffer();
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
   packetfunctions_reserveHeaderSize(pkt,MAXPAYLOADLEN);
//   for (i=0;i<MAXPAYLOADLEN;i++) {
//      pkt->payload[i] = i;    // add your data to be sent!
//   }
//   avg = openrandom_get16b();
//   pkt->payload[0] = (avg>>8)&0xff;
//   pkt->payload[1] = (avg>>0)&0xff;
 
#ifdef ROUTINGDEBUG   
   
   neighbors_getAll(table);
   for(j=0;j<MAXNUMNEIGHBORS;j++) {
     if(table[j].used) {
       pkt->payload[i++] = table[j].addr_64b.addr_16b[1];//last byte of the address; poipoi could be [0]; endianness
       pkt->payload[i++] = table[j].rssi;
       pkt->payload[i++] = table[j].parentPreference;
//       pkt->payload[i++] = table[j].numRx;
//       pkt->payload[i++] = table[j].numTx;
//       pkt->payload[i++] = table[j].numTxACK;
       pkt->payload[i++] = table[j].DAGrank;
       pkt->payload[i++] = (table[j].asn.bytes0and1>>8)&0xff;
       pkt->payload[i++] = (table[j].asn.bytes0and1>>0)&0xff;
     }
   }
#elif SCHEDULEDEBUG
  
   scheduleBuf_getAll(schbuffer);
   for(j=0;j<MAXACTIVESLOTS;j++) {
   pkt->payload[i++] = schbuffer[j].neighbor.addr_16b[1];//last byte of the address; poipoi could be [0]; endianness
   pkt->payload[i++] = schbuffer[j].(slotOffset>>8)&0xff;
   pkt->payload[i++] = schbuffer[j].(slotOffset>>0)&0xff;
   pkt->payload[i++] = schbuffer[j].channelOffset;
//   pkt->payload[i++] = backoffExponent;
//   pkt->payload[i++] = backoff;
//   pkt->payload[i++] = numRx;
//   pkt->payload[i++] = numTx;
//   pkt->payload[i++] = numTxACK;
//   pkt->payload[i++] = (lastUsedAsn.bytes0and1>>8)&0xff;
//   pkt->payload[i++] = (lastUsedAsn.bytes0and1>>0)&0xff;
//   pkt->payload[i++] = numTxACK;
//   pkt->payload[i++] = numTxACK; 
   
   }
#endif
   
   numOptions = 0;
   // location-path option
   packetfunctions_reserveHeaderSize(pkt,sizeof(layerdebug_path0)-1);
   memcpy(&pkt->payload[0],&layerdebug_path0,sizeof(layerdebug_path0)-1);
   packetfunctions_reserveHeaderSize(pkt,1);
   pkt->payload[0]                  = (COAP_OPTION_LOCATIONPATH-COAP_OPTION_CONTENTTYPE) << 4 |
      sizeof(layerdebug_path0)-1;
   numOptions++;
   // content-type option
   packetfunctions_reserveHeaderSize(pkt,2);
   pkt->payload[0]                  = COAP_OPTION_CONTENTTYPE << 4 |
      1;
   pkt->payload[1]                  = COAP_MEDTYPE_APPOCTETSTREAM;
   numOptions++;
   // metadata
   pkt->l4_destination_port         = WKP_UDP_COAP;
   pkt->l3_destinationORsource.type = ADDR_128B;
   memcpy(&pkt->l3_destinationORsource.addr_128b[0],&ipAddr_motesEecs,16);
   // send
   outcome = opencoap_send(pkt,
                           COAP_TYPE_NON,
                           COAP_CODE_REQ_PUT,
                           numOptions,
                           &layerdebug_vars.desc);
   // avoid overflowing the queue if fails
   if (outcome==E_FAIL) {
      openqueue_freePacketBuffer(pkt);
   }
   
   return;
}

void layerdebug_sendDone(OpenQueueEntry_t* msg, error_t error) {
   openqueue_freePacketBuffer(msg);
}
