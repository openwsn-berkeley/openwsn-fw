#include "openwsn.h"
#include "openqueue.h"
#include "openserial.h"
#include "packetfunctions.h"

//=========================== variables =======================================

typedef struct {
   OpenQueueEntry_t queue[QUEUELENGTH];
} openqueue_vars_t;

openqueue_vars_t openqueue_vars;

//=========================== prototypes ======================================

void openqueue_reset_entry(OpenQueueEntry_t* entry);

//=========================== public ==========================================

//======= admin

void openqueue_init() {
   uint8_t i;
   for (i=0;i<QUEUELENGTH;i++){
      openqueue_reset_entry(&(openqueue_vars.queue[i]));
   }
}

bool debugPrint_queue() {
   debugOpenQueueEntry_t output[QUEUELENGTH];
   uint8_t i;
   for (i=0;i<QUEUELENGTH;i++) {
      output[i].creator = openqueue_vars.queue[i].creator;
      output[i].owner   = openqueue_vars.queue[i].owner;
   }
   openserial_printStatus(STATUS_QUEUE,(uint8_t*)&output,QUEUELENGTH*sizeof(debugOpenQueueEntry_t));
   return TRUE;
}

//======= called by any component

__monitor OpenQueueEntry_t* openqueue_getFreePacketBuffer() {
   uint8_t i;
   for (i=0;i<QUEUELENGTH;i++) {
      if (openqueue_vars.queue[i].owner==COMPONENT_NULL) {
         openqueue_vars.queue[i].owner=COMPONENT_OPENQUEUE;
         return &openqueue_vars.queue[i];
      }
   }
   return NULL;
}

__monitor error_t openqueue_freePacketBuffer(OpenQueueEntry_t* pkt) {
   uint8_t i;
   for (i=0;i<QUEUELENGTH;i++) {
      if (&openqueue_vars.queue[i]==pkt) {
         if (openqueue_vars.queue[i].owner==COMPONENT_NULL) {
            // log the error
            openserial_printError(COMPONENT_OPENQUEUE,ERR_FREEING_UNUSED,
                                  (errorparameter_t)0,
                                  (errorparameter_t)0);
         }
         openqueue_reset_entry(&(openqueue_vars.queue[i]));
         return E_SUCCESS;
      }
   }
   // log the error
   openserial_printError(COMPONENT_OPENQUEUE,ERR_FREEING_ERROR,
                         (errorparameter_t)0,
                         (errorparameter_t)0);
   return E_FAIL;
}

__monitor void openqueue_removeAllOwnedBy(uint8_t owner) {
   uint8_t i;
   for (i=0;i<QUEUELENGTH;i++){
      if (openqueue_vars.queue[i].owner==owner) {
         openqueue_reset_entry(&(openqueue_vars.queue[i]));
      }
   }
}

//======= called by RES

__monitor OpenQueueEntry_t* openqueue_resGetSentPacket() {
   uint8_t i;
   for (i=0;i<QUEUELENGTH;i++) {
      if (openqueue_vars.queue[i].owner==COMPONENT_IEEE802154E_TO_RES &&
          openqueue_vars.queue[i].creator!=COMPONENT_IEEE802154E) {
         return &openqueue_vars.queue[i];
      }
   }
   return NULL;
}

__monitor OpenQueueEntry_t* openqueue_resGetReceivedPacket() {
   uint8_t i;
   for (i=0;i<QUEUELENGTH;i++) {
      if (openqueue_vars.queue[i].owner==COMPONENT_IEEE802154E_TO_RES &&
          openqueue_vars.queue[i].creator==COMPONENT_IEEE802154E) {
         return &openqueue_vars.queue[i];
      }
   }
   return NULL;
}

//======= called by IEEE80215E

__monitor OpenQueueEntry_t* openqueue_macGetDataPacket(open_addr_t* toNeighbor) {
   uint8_t i;
   if (toNeighbor->type==ADDR_64B) {
      // a neighbor is specified, look for a packet unicast to that neigbhbor
      for (i=0;i<QUEUELENGTH;i++) {
         if (openqueue_vars.queue[i].owner==COMPONENT_RES_TO_IEEE802154E &&
            packetfunctions_sameAddress(toNeighbor,&openqueue_vars.queue[i].l2_nextORpreviousHop)) {
            return &openqueue_vars.queue[i];
         }
      }
   } else if (toNeighbor->type==ADDR_ANYCAST) {
      // anycast case: look for a packet which is either not created by RES
      // or an KA (created by RES, but not broadcast)
      for (i=0;i<QUEUELENGTH;i++) {
         if (openqueue_vars.queue[i].owner==COMPONENT_RES_TO_IEEE802154E &&
             ( openqueue_vars.queue[i].creator!=COMPONENT_RES ||
                (
                   openqueue_vars.queue[i].creator==COMPONENT_RES &&
                   packetfunctions_isBroadcastMulticast(&(openqueue_vars.queue[i].l2_nextORpreviousHop))==FALSE
                )
             )
            ) {
            return &openqueue_vars.queue[i];
         }
      }
   }
   return NULL;
}

__monitor OpenQueueEntry_t* openqueue_macGetAdvPacket() {
   uint8_t i;
   for (i=0;i<QUEUELENGTH;i++) {
      if (openqueue_vars.queue[i].owner==COMPONENT_RES_TO_IEEE802154E &&
          openqueue_vars.queue[i].creator==COMPONENT_RES              &&
          packetfunctions_isBroadcastMulticast(&(openqueue_vars.queue[i].l2_nextORpreviousHop))) {
         return &openqueue_vars.queue[i];
      }
   }
   return NULL;
}

//=========================== private =========================================

void openqueue_reset_entry(OpenQueueEntry_t* entry) {
   //admin
   entry->creator                     = COMPONENT_NULL;
   entry->owner                       = COMPONENT_NULL;
   entry->payload                     = &(entry->packet[127]);
   entry->length                      = 0;
   //l4
   entry->l4_protocol                 = IANA_UNDEFINED;
   //l3
   entry->l3_destinationORsource.type = ADDR_NONE;
   //l2
   entry->l2_nextORpreviousHop.type   = ADDR_NONE;
   entry->l2_frameType                = IEEE154_TYPE_UNDEFINED;
   entry->l2_retriesLeft              = 0;
}