#include "openwsn.h"
#include "appudptimer.h"
#include "udp.h"
#include "openqueue.h"
#include "openserial.h"
#include "packetfunctions.h"
#include "timers.h"
#include "idmanager.h"

//=========================== variables =======================================

typedef struct {
   bool       busySending;
   int8_t     delayCounter;
} appudptimer_vars_t;

appudptimer_vars_t appudptimer_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void appudptimer_init() {
   appudptimer_vars.busySending  = FALSE;
   appudptimer_vars.delayCounter = 0;
   // all motes which are not the DAGroot publish data periodically
   if (idmanager_getIsDAGroot()==FALSE) {
      //poipoipoitimer_startPeriodic(TIMER_UDPTIMER,0xffff);// every 2 seconds
   }
}

void timer_appudptimer_fired() {
   OpenQueueEntry_t* pkt;
   /*
   // send every 10s
   appudptimer_vars.delayCounter = (appudptimer_vars.delayCounter+1)%5;
   if (appudptimer_vars.delayCounter!=0) {
      return;
   }
   */
   // only send a packet if I received a sendDone for the previous.
   // the packet might be stuck in the queue for a long time for
   // example while the mote is synchronizing
   if (appudptimer_vars.busySending==FALSE) {
      //prepare packet
      pkt = openqueue_getFreePacketBuffer();
      if (pkt==NULL) {
         openserial_printError(COMPONENT_APPUDPTIMER,ERR_NO_FREE_PACKET_BUFFER,
                               (errorparameter_t)0,
                               (errorparameter_t)0);
         return;
      }
      pkt->creator                               = COMPONENT_APPUDPTIMER;
      pkt->owner                                 = COMPONENT_APPUDPTIMER;
      pkt->l4_protocol                           = IANA_UDP;
      pkt->l4_sourcePortORicmpv6Type             = WKP_UDP_TIMER;
      pkt->l4_destination_port                   = WKP_UDP_TIMER;
      pkt->l3_destinationORsource.type           = ADDR_128B;
      pkt->l3_destinationORsource.addr_128b[ 0]  = 0x20;
      pkt->l3_destinationORsource.addr_128b[ 1]  = 0x01;
      pkt->l3_destinationORsource.addr_128b[ 2]  = 0x04;
      pkt->l3_destinationORsource.addr_128b[ 3]  = 0x70;
      pkt->l3_destinationORsource.addr_128b[ 4]  = 0x81;
      pkt->l3_destinationORsource.addr_128b[ 5]  = 0x9f;
      pkt->l3_destinationORsource.addr_128b[ 6]  = 0x00;
      pkt->l3_destinationORsource.addr_128b[ 7]  = 0x01;
      pkt->l3_destinationORsource.addr_128b[ 8]  = 0x14;
      pkt->l3_destinationORsource.addr_128b[ 9]  = 0xf1;
      pkt->l3_destinationORsource.addr_128b[10]  = 0xff;
      pkt->l3_destinationORsource.addr_128b[11]  = 0xef;
      pkt->l3_destinationORsource.addr_128b[12]  = 0xa4;
      pkt->l3_destinationORsource.addr_128b[13]  = 0x4c;
      pkt->l3_destinationORsource.addr_128b[14]  = 0x70;
      pkt->l3_destinationORsource.addr_128b[15]  = 0xb5;
      packetfunctions_reserveHeaderSize(pkt,6);
      ((uint8_t*)pkt->payload)[0]                = 'p';
      ((uint8_t*)pkt->payload)[1]                = 'o';
      ((uint8_t*)pkt->payload)[2]                = 'i';
      ((uint8_t*)pkt->payload)[3]                = 'p';
      ((uint8_t*)pkt->payload)[4]                = 'o';
      ((uint8_t*)pkt->payload)[5]                = 'i';
      //send packet
      if ((udp_send(pkt))==E_FAIL) {
         openqueue_freePacketBuffer(pkt);
         appudptimer_vars.busySending            = FALSE;
      } else {
         appudptimer_vars.busySending            = TRUE;
      }
   }
}

void appudptimer_sendDone(OpenQueueEntry_t* msg, error_t error) {
   msg->owner = COMPONENT_APPUDPTIMER;
   if (msg->creator!=COMPONENT_APPUDPTIMER) {
      openserial_printError(COMPONENT_APPUDPTIMER,ERR_UNEXPECTED_SENDDONE,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
   }
   openqueue_freePacketBuffer(msg);
   appudptimer_vars.busySending = FALSE;
}

void appudptimer_receive(OpenQueueEntry_t* msg) {
   openserial_printError(COMPONENT_APPUDPTIMER,ERR_UNSPECIFIED,
                         (errorparameter_t)0,
                         (errorparameter_t)0);
   openqueue_freePacketBuffer(msg);
}

bool appudptimer_debugPrint() {
   return FALSE;
}

//=========================== private =========================================