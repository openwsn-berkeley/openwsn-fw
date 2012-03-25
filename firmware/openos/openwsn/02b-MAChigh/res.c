#include "openwsn.h"
#include "res.h"
#include "idmanager.h"
#include "openserial.h"
#include "IEEE802154.h"
#include "IEEE802154E.h"
#include "openqueue.h"
#include "neighbors.h"
#include "IEEE802154E.h"
#include "iphc.h"
#include "leds.h"
#include "packetfunctions.h"
#include "openrandom.h"
#include "schedule.h"
#include "scheduler.h"
#include "bsp_timers.h"
#include "opentimers.h"

//=========================== variables =======================================

typedef struct {
   uint16_t        periodMaintenance;
   bool            busySending;          // TRUE when busy sending an advertisement or keep-alive
   uint8_t         dsn;                  // current data sequence number
   uint8_t         MacMgtTaskCounter;    // counter to determine what management task to do
   opentimer_id_t  timerId;
} res_vars_t;

res_vars_t res_vars;

//=========================== prototypes ======================================

error_t res_send_internal(OpenQueueEntry_t* msg);
void    sendAdv();
void    sendKa();
void    res_timer_cb();

//=========================== public ==========================================

void res_init() {
   res_vars.periodMaintenance = 16384+openrandom_get16b()%32768; // fires every 1 sec on average
   res_vars.busySending       = FALSE;
   res_vars.dsn               = 0;
   res_vars.MacMgtTaskCounter = 0;
   res_vars.timerId = opentimers_start(res_vars.periodMaintenance,
                                       TIMER_PERIODIC,
                                       res_timer_cb);
}

bool debugPrint_myDAGrank() {
   uint8_t output=0;
   output = neighbors_getMyDAGrank();
   openserial_printStatus(STATUS_DAGRANK,(uint8_t*)&output,sizeof(uint8_t));
   return TRUE;
}

//======= from upper layer

error_t res_send(OpenQueueEntry_t *msg) {
   msg->owner        = COMPONENT_RES;
   msg->l2_frameType = IEEE154_TYPE_DATA;
   return res_send_internal(msg);
}

//======= from lower layer

void task_resNotifSendDone() {
   OpenQueueEntry_t* msg;
   // get recently-sent packet from openqueue
   msg = openqueue_resGetSentPacket();
   if (msg==NULL) {
      // log the error
      openserial_printError(COMPONENT_RES,ERR_NO_SENT_PACKET,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      // abort
      return;
   }
   // declare it as mine
   msg->owner = COMPONENT_RES;
   // indicate transmission (to update statistics)
   if (msg->l2_sendDoneError==E_SUCCESS) {
      neighbors_indicateTx(&(msg->l2_nextORpreviousHop),
                           msg->l2_numTxAttempts,
                           TRUE,
                           &msg->l2_asn);
   } else {
      neighbors_indicateTx(&(msg->l2_nextORpreviousHop),
                           msg->l2_numTxAttempts,
                           FALSE,
                           &msg->l2_asn);
   }
   // send the packet to where it belongs
   if (msg->creator == COMPONENT_RES) {
      // discard (ADV or KA) packets this component has created
      openqueue_freePacketBuffer(msg);
      // I can send the next ADV or KA
      res_vars.busySending = FALSE;
      // restart a random timer
      res_vars.periodMaintenance = 16384+openrandom_get16b()%32768;
      res_vars.timerId = opentimers_start(res_vars.periodMaintenance,
                                          TIMER_PERIODIC,
                                          res_timer_cb);
   } else {
      // send the rest up the stack
      iphc_sendDone(msg,msg->l2_sendDoneError);
   }
}

void task_resNotifReceive() {
   OpenQueueEntry_t* msg;
   
   // get received packet from openqueue
   msg = openqueue_resGetReceivedPacket();
   if (msg==NULL) {
      // log the error
      openserial_printError(COMPONENT_RES,ERR_NO_RECEIVED_PACKET,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      // abort
      return;
   }
   
   // declare it as mine
   msg->owner = COMPONENT_RES;
   
   // indicate reception (to update statistics)
   neighbors_indicateRx(&(msg->l2_nextORpreviousHop),
                        msg->l1_rssi,
                        &msg->l2_asn);
   
   // send the packet up the stack, if it qualifies
   switch (msg->l2_frameType) {
      case IEEE154_TYPE_BEACON:
      case IEEE154_TYPE_DATA:
      case IEEE154_TYPE_CMD:
         if (msg->length>0) {
            // send to upper layer
            iphc_receive(msg);
         } else {
            // free up the RAM
            openqueue_freePacketBuffer(msg);
         }
         break;
      case IEEE154_TYPE_ACK:
      default:
         // free the packet's RAM memory
         openqueue_freePacketBuffer(msg);
         // log the error
         openserial_printError(COMPONENT_RES,ERR_MSG_UNKNOWN_TYPE,
                               (errorparameter_t)msg->l2_frameType,
                               (errorparameter_t)0);
         break;
   }
}

//======= timer

/**
\brief Timer handlers which triggers MAC management task.

This function is called in task context by the scheduler after the RES timer
has fired. This timer is set to fire every second, on average.

The body of this function executes one of the MAC management task.
*/
void timers_res_fired() {
   res_vars.MacMgtTaskCounter = (res_vars.MacMgtTaskCounter+1)%2;
   if (idmanager_getMyID(ADDR_16B)->addr_16b[1]==DEBUG_MOTEID_MASTER) {
      if (res_vars.MacMgtTaskCounter==0) {
         sendAdv();
      } else {
         // don't send KAs if you're the master
      }
   } else {
      if (res_vars.MacMgtTaskCounter==0) {
         // don't send ADVs if you're not the master
      } else {
         sendKa();
      }
   }
}

//=========================== private =========================================

/**
\brief Transfer packet to MAC.

This function adds a IEEE802.15.4 header to the packet and leaves it the 
OpenQueue buffer. The very last thing it does is assigning this packet to the 
virtual component COMPONENT_RES_TO_IEEE802154E. Whenever it gets a change,
IEEE802154E will handle the packet.

\param [in] msg The packet to the transmitted

\returns E_SUCCESS iff successful.
*/
error_t res_send_internal(OpenQueueEntry_t* msg) {
   // assign a number of retries
   if (packetfunctions_isBroadcastMulticast(&(msg->l2_nextORpreviousHop))==TRUE) {
      msg->l2_retriesLeft = 1;
   } else {
      msg->l2_retriesLeft = TXRETRIES;
   }
   // record this packet's dsn (for matching the ACK)
   msg->l2_dsn = res_vars.dsn++;
   // this is a new packet which I never attempted to send
   msg->l2_numTxAttempts = 0;
   // transmit with the default TX power
   msg->l1_txPower = TX_POWER;
   // record the location, in the packet, where the l2 payload starts
   msg->l2_payload = msg->payload;
   // add a IEEE802.15.4 header
   ieee802154_prependHeader(msg,
                            msg->l2_frameType,
                            IEEE154_SEC_NO_SECURITY,
                            msg->l2_dsn,
                            &(msg->l2_nextORpreviousHop)
                            );
   // reserve space for 2-byte CRC
   packetfunctions_reserveFooterSize(msg,2);
   // change owner to IEEE802154E fetches it from queue
   msg->owner  = COMPONENT_RES_TO_IEEE802154E;
   return E_SUCCESS;
}

/**
\brief Send an advertisement.

This is one of the MAC managament tasks. This function inlines in the
timers_res_fired() function, but is declared as a separate function for better
readability of the code.
*/
inline void sendAdv() {
   OpenQueueEntry_t* adv;
   // only send a packet if I received a sendDone for the previous.
   // the packet might be stuck in the queue for a long time for
   // example while the mote is synchronizing
   if (res_vars.busySending==FALSE) {
      // get a free packet buffer
      adv = openqueue_getFreePacketBuffer();
      if (adv==NULL) {
         openserial_printError(COMPONENT_RES,ERR_NO_FREE_PACKET_BUFFER,
                               (errorparameter_t)0,
                               (errorparameter_t)0);
         return;
      }
      
      // declare ownership over that packet
      adv->creator = COMPONENT_RES;
      adv->owner   = COMPONENT_RES;
      
      // reserve space for ADV-specific header
      packetfunctions_reserveHeaderSize(adv, ADV_PAYLOAD_LENGTH);
      // the actual value of the current ASN will be written by the
      // IEEE802.15.4e when transmitting
      
      // some l2 information about this packet
      adv->l2_frameType                     = IEEE154_TYPE_BEACON;
      adv->l2_nextORpreviousHop.type        = ADDR_16B;
      adv->l2_nextORpreviousHop.addr_16b[0] = 0xff;
      adv->l2_nextORpreviousHop.addr_16b[1] = 0xff;
      
      // put in queue for MAC to handle
      res_send_internal(adv);
      res_vars.busySending = TRUE;
   }
}

/**
\brief Send an keep-alive message, if nessary.

This is one of the MAC managament tasks. This function inlines in the
timers_res_fired() function, but is declared as a separate function for better
readability of the code.
*/
inline void sendKa() {
   OpenQueueEntry_t* kaPkt;
   open_addr_t*      kaNeighAddr;
   
   // only send a packet if I received a sendDone for the previous.
   // the packet might be stuck in the queue for a long time for
   // example while the mote is synchronizing
   if (res_vars.busySending==FALSE) {
      kaNeighAddr = neighbors_KaNeighbor();
      if (kaNeighAddr!=NULL) {
         // get a free packet buffer
         kaPkt = openqueue_getFreePacketBuffer();
         if (kaPkt==NULL) {
            openserial_printError(COMPONENT_RES,ERR_NO_FREE_PACKET_BUFFER,
                                  (errorparameter_t)0,
                                  (errorparameter_t)0);
            return;
         }
         
         // declare ownership over that packet
         kaPkt->creator = COMPONENT_RES;
         kaPkt->owner   = COMPONENT_RES;
         
         // some l2 information about this packet
         kaPkt->l2_frameType = IEEE154_TYPE_DATA;
         memcpy(&(kaPkt->l2_nextORpreviousHop),kaNeighAddr,sizeof(open_addr_t));
         
         // put in queue for MAC to handle
         res_send_internal(kaPkt);
         res_vars.busySending = TRUE;
      }
   }
}

void res_timer_cb() {
   scheduler_push_task(timers_res_fired,TASKPRIO_RES);
}