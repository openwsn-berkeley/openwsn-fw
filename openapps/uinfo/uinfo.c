#include "opendefs.h"
#include "uinfo.h"
#include "openudp.h"
#include "openqueue.h"
#include "opentimers.h"
#include "openserial.h"
#include "packetfunctions.h"
#include "scheduler.h"
#include "IEEE802154E.h"
#include "idmanager.h"
//#include "sensors.h"
#include "stats.h"

//=========================== variables =======================================

uinfo_vars_t uinfo_vars;

static const uint8_t uinfo_dst_addr[]   = {
   0xbb, 0xbb, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
}; 

//=========================== prototypes ======================================

void uinfo_timer_cb(opentimer_id_t id);
void uinfo_task_cb(void);
void uinfo_readSensors(OpenQueueEntry_t*    pkt);
//=========================== public ==========================================

void uinfo_init() {
   
   // clear local variables
   memset(&uinfo_vars,0,sizeof(uinfo_vars_t));
   
   uinfo_vars.period = UINFO_PERIOD_MS;
   
   // start periodic timer
   uinfo_vars.timerId                    = opentimers_start(
      uinfo_vars.period,
      TIMER_PERIODIC,TIME_MS,
      uinfo_timer_cb
   );
}

void uinfo_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openqueue_freePacketBuffer(msg);
}

void uinfo_receive(OpenQueueEntry_t* pkt) {
   
   openqueue_freePacketBuffer(pkt);
   
   openserial_printError(
      COMPONENT_UINFO,
      ERR_RCVD_ECHO_REPLY,
      (errorparameter_t)0,
      (errorparameter_t)0
   );
}

//=========================== private =========================================

/**
\note timer fired, but we don't want to execute task in ISR mode instead, push
   task to scheduler with CoAP priority, and let scheduler take care of it.
*/
void uinfo_timer_cb(opentimer_id_t id){
   
   scheduler_push_task(uinfo_task_cb,TASKPRIO_COAP);
}

void uinfo_task_cb() {
   OpenQueueEntry_t*    pkt;
   uint8_t              asnArray[5];
   uint16_t             queueuse;
   uint16_t             scheduleuse;


   // don't run if not synch
   if (ieee154e_isSynch() == FALSE) return;
   
   // don't run on dagroot
   if (idmanager_getIsDAGroot()) {
      opentimers_stop(uinfo_vars.timerId);
      return;
   }
   
   // if you get here, send a packet
   
   // get a free packet buffer
   pkt = openqueue_getFreePacketBuffer(COMPONENT_UINFO);
   if (pkt==NULL) {
      openserial_printError(
         COMPONENT_UINFO,
         ERR_NO_FREE_PACKET_BUFFER,
         (errorparameter_t)0,
         (errorparameter_t)0
      );
      return;
   }
   
   pkt->owner                         = COMPONENT_UINFO;
   pkt->creator                       = COMPONENT_UINFO;
   pkt->l4_protocol                   = IANA_UDP;
   pkt->l4_destination_port           = WKP_UDP_UINFO;
   pkt->l4_sourcePortORicmpv6Type     = WKP_UDP_UINFO;
   pkt->l3_destinationAdd.type        = ADDR_128B;
   memcpy(&pkt->l3_destinationAdd.addr_128b[0],uinfo_dst_addr,16);
   
   //counter
   packetfunctions_reserveHeaderSize(pkt,sizeof(uint16_t));
   pkt->payload[1] = (uint8_t)((uinfo_vars.counter & 0xff00)>>8);
   pkt->payload[0] = (uint8_t)(uinfo_vars.counter & 0x00ff);
   uinfo_vars.counter++;
   
   //asn
   packetfunctions_reserveHeaderSize(pkt,sizeof(asn_t));
   ieee154e_getAsn(asnArray);
   pkt->payload[0] = asnArray[0];
   pkt->payload[1] = asnArray[1];
   pkt->payload[2] = asnArray[2];
   pkt->payload[3] = asnArray[3];
   pkt->payload[4] = asnArray[4];

   //sensors?
   //uinfo_readSensors(pkt);

   //queue use
   queueuse = stats_getCounter(STAT_QUEUE_USE);
   packetfunctions_reserveHeaderSize(pkt,sizeof(uint16_t));
   pkt->payload[1] = (uint8_t)((queueuse & 0xff00)>>8);
   pkt->payload[0] = (uint8_t)(queueuse & 0x00ff);

   //schedule use
   scheduleuse = stats_getCounter(STAT_SCHED_EMPTY);
   packetfunctions_reserveHeaderSize(pkt,sizeof(uint16_t));
   pkt->payload[1] = (uint8_t)((scheduleuse & 0xff00)>>8);
   pkt->payload[0] = (uint8_t)(scheduleuse & 0x00ff);

   scheduleuse = stats_getCounter(STAT_SCHED_USE_RX);
   packetfunctions_reserveHeaderSize(pkt,sizeof(uint16_t));
   pkt->payload[1] = (uint8_t)((scheduleuse & 0xff00)>>8);
   pkt->payload[0] = (uint8_t)(scheduleuse & 0x00ff);

   scheduleuse = stats_getCounter(STAT_SCHED_USE_TX);
   packetfunctions_reserveHeaderSize(pkt,sizeof(uint16_t));
   pkt->payload[1] = (uint8_t)((scheduleuse & 0xff00)>>8);
   pkt->payload[0] = (uint8_t)(scheduleuse & 0x00ff);

   scheduleuse = stats_getCounter(STAT_SCHED_USE_SHARED_TXRX);
   packetfunctions_reserveHeaderSize(pkt,sizeof(uint16_t));
   pkt->payload[1] = (uint8_t)((scheduleuse & 0xff00)>>8);
   pkt->payload[0] = (uint8_t)(scheduleuse & 0x00ff);

   if ((openudp_send(pkt))==E_FAIL) {
      openqueue_freePacketBuffer(pkt);
   }
}


void uinfo_readSensors(OpenQueueEntry_t*    pkt){
/*
    uint16_t             temp;
    uint16_t             hum;
    uint16_t             lig;

    callbackRead_cbt     cbsen;
    callbackConvert_cbt  cbcon;

    cbsen = sensors_getCallbackRead(SENSOR_TEMPERATURE);

    temp = cbsen();

    packetfunctions_reserveHeaderSize(pkt,sizeof(uint16_t));
    pkt->payload[1] = (uint8_t)((temp & 0xff00)>>8);
    pkt->payload[0] = (uint8_t)(temp & 0x00ff);

    cbsen = sensors_getCallbackRead(SENSOR_HUMIDITY);

    hum = cbsen();

    packetfunctions_reserveHeaderSize(pkt,sizeof(uint16_t));
    pkt->payload[1] = (uint8_t)((hum & 0xff00)>>8);
    pkt->payload[0] = (uint8_t)(hum & 0x00ff);

    cbsen = sensors_getCallbackRead(SENSOR_LIGHT);

    lig = cbsen();

    cbcon = sensors_getCallbackConvert(SENSOR_LIGHT);

    lig = (uint16_t) (cbcon(lig)*100); //2 decimals

    packetfunctions_reserveHeaderSize(pkt,sizeof(uint16_t));
    pkt->payload[1] = (uint8_t)((lig & 0xff00)>>8);
    pkt->payload[0] = (uint8_t)(lig & 0x00ff);
    */

}
