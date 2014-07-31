/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:11:19.160994.
*/
#include "openwsn_obj.h"
#include "openqueue_obj.h"
#include "openserial_obj.h"
#include "packetfunctions_obj.h"
#include "IEEE802154E_obj.h"

//=========================== variables =======================================

// declaration of global variable _openqueue_vars_ removed during objectification.

//=========================== prototypes ======================================

void openqueue_reset_entry(OpenMote* self, OpenQueueEntry_t* entry);

//=========================== public ==========================================

//======= admin

/**
\brief Initialize this module.
*/
void openqueue_init(OpenMote* self) {
   uint8_t i;
   for (i=0;i<QUEUELENGTH;i++){
 openqueue_reset_entry(self, &((self->openqueue_vars).queue[i]));
   }
}

/**
\brief Trigger this module to print status information, over serial.

debugPrint_* functions are used by the openserial module to continuously print
status information about several modules in the OpenWSN stack.

\returns TRUE if this function printed something, FALSE otherwise.
*/
//bool debugPrint_queue(OpenMote* self) {
//   debugOpenQueueEntry_t output[QUEUELENGTH];
//   uint8_t i;
//   for (i=0;i<QUEUELENGTH;i++) {
//      output[i].creator = (self->openqueue_vars).queue[i].creator;
//      output[i].owner   = (self->openqueue_vars).queue[i].owner;
//   }
// openserial_printStatus(self, STATUS_QUEUE,(uint8_t*)&output,QUEUELENGTH*sizeof(debugOpenQueueEntry_t));
//   return TRUE;
//}

//======= called by any component

/**
\brief Request a new (free) packet buffer.

Component throughout the protocol stack can call this function is they want to
get a new packet buffer to start creating a new packet.

\note Once a packet has been allocated, it is up to the creator of the packet
      to free it using the openqueue_freePacketBuffer(self) function.

\returns A pointer to the queue entry when it could be allocated, or NULL when
         it could not be allocated (buffer full or not synchronized).
*/
OpenQueueEntry_t* openqueue_getFreePacketBuffer(OpenMote* self, uint8_t creator) {
   uint8_t i;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   
   // refuse to allocate if we're not in sync
   if ( ieee154e_isSynch(self)==FALSE && creator > COMPONENT_IEEE802154E){
     ENABLE_INTERRUPTS();
     return NULL;
   }
   
   // if you get here, I will try to allocate a buffer for you
   
   // walk through queue and find free entry
   for (i=0;i<QUEUELENGTH;i++) {
      if ((self->openqueue_vars).queue[i].owner==COMPONENT_NULL) {
         (self->openqueue_vars).queue[i].creator=creator;
         (self->openqueue_vars).queue[i].owner=COMPONENT_OPENQUEUE;
         ENABLE_INTERRUPTS(); 
         return &(self->openqueue_vars).queue[i];
      }
   }
   ENABLE_INTERRUPTS();
   return NULL;
}


/**
\brief Free a previously-allocated packet buffer.

\param pkt A pointer to the previsouly-allocated packet buffer.

\returns E_SUCCESS when the freeing was succeful.
\returns E_FAIL when the module could not find the specified packet buffer.
*/
owerror_t openqueue_freePacketBuffer(OpenMote* self, OpenQueueEntry_t* pkt) {
   uint8_t i;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   for (i=0;i<QUEUELENGTH;i++) {
      if (&(self->openqueue_vars).queue[i]==pkt) {
         if ((self->openqueue_vars).queue[i].owner==COMPONENT_NULL) {
            // log the error
// openserial_printCritical(self, COMPONENT_OPENQUEUE,ERR_FREEING_UNUSED,
//                                  (errorparameter_t)0,
//                                  (errorparameter_t)0);
         }
 openqueue_reset_entry(self, &((self->openqueue_vars).queue[i]));
         ENABLE_INTERRUPTS();
         return E_SUCCESS;
      }
   }
   // log the error
// openserial_printCritical(self, COMPONENT_OPENQUEUE,ERR_FREEING_ERROR,
//                         (errorparameter_t)0,
//                         (errorparameter_t)0);
   ENABLE_INTERRUPTS();
   return E_FAIL;
}

/**
\brief Free all the packet buffers created by a specific module.

\param creator The identifier of the component, taken in COMPONENT_*.
*/
void openqueue_removeAllCreatedBy(OpenMote* self, uint8_t creator) {
   uint8_t i;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   for (i=0;i<QUEUELENGTH;i++){
      if ((self->openqueue_vars).queue[i].creator==creator) {
 openqueue_reset_entry(self, &((self->openqueue_vars).queue[i]));
      }
   }
   ENABLE_INTERRUPTS();
}

/**
\brief Free all the packet buffers owned by a specific module.

\param owner The identifier of the component, taken in COMPONENT_*.
*/
void openqueue_removeAllOwnedBy(OpenMote* self, uint8_t owner) {
   uint8_t i;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   for (i=0;i<QUEUELENGTH;i++){
      if ((self->openqueue_vars).queue[i].owner==owner) {
 openqueue_reset_entry(self, &((self->openqueue_vars).queue[i]));
      }
   }
   ENABLE_INTERRUPTS();
}

//======= called by RES

OpenQueueEntry_t* openqueue_sixtopGetSentPacket(OpenMote* self) {
   uint8_t i;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   for (i=0;i<QUEUELENGTH;i++) {
      if ((self->openqueue_vars).queue[i].owner==COMPONENT_IEEE802154E_TO_SIXTOP &&
          (self->openqueue_vars).queue[i].creator!=COMPONENT_IEEE802154E) {
         ENABLE_INTERRUPTS();
         return &(self->openqueue_vars).queue[i];
      }
   }
   ENABLE_INTERRUPTS();
   return NULL;
}

OpenQueueEntry_t* openqueue_sixtopGetReceivedPacket(OpenMote* self) {
   uint8_t i;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   for (i=0;i<QUEUELENGTH;i++) {
      if ((self->openqueue_vars).queue[i].owner==COMPONENT_IEEE802154E_TO_SIXTOP &&
          (self->openqueue_vars).queue[i].creator==COMPONENT_IEEE802154E) {
         ENABLE_INTERRUPTS();
         return &(self->openqueue_vars).queue[i];
      }
   }
   ENABLE_INTERRUPTS();
   return NULL;
}

//======= called by IEEE80215E

OpenQueueEntry_t* openqueue_macGetDataPacket(OpenMote* self, open_addr_t* toNeighbor) {
   uint8_t i;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   if (toNeighbor->type==ADDR_64B) {
      // a neighbor is specified, look for a packet unicast to that neigbhbor
      for (i=0;i<QUEUELENGTH;i++) {
         if ((self->openqueue_vars).queue[i].owner==COMPONENT_SIXTOP_TO_IEEE802154E &&
 packetfunctions_sameAddress(self, toNeighbor,&(self->openqueue_vars).queue[i].l2_nextORpreviousHop)) {
            ENABLE_INTERRUPTS();
            return &(self->openqueue_vars).queue[i];
         }
      }
   } else if (toNeighbor->type==ADDR_ANYCAST) {
      // anycast case: look for a packet which is either not created by RES
      // or an KA (created by RES, but not broadcast)
      for (i=0;i<QUEUELENGTH;i++) {
         if ((self->openqueue_vars).queue[i].owner==COMPONENT_SIXTOP_TO_IEEE802154E &&
             ( (self->openqueue_vars).queue[i].creator!=COMPONENT_SIXTOP ||
                (
                   (self->openqueue_vars).queue[i].creator==COMPONENT_SIXTOP &&
 packetfunctions_isBroadcastMulticast(self, &((self->openqueue_vars).queue[i].l2_nextORpreviousHop))==FALSE
                )
             )
            ) {
            ENABLE_INTERRUPTS();
            return &(self->openqueue_vars).queue[i];
         }
      }
   }
   ENABLE_INTERRUPTS();
   return NULL;
}

OpenQueueEntry_t* openqueue_macGetAdvPacket(OpenMote* self) {
   uint8_t i;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   for (i=0;i<QUEUELENGTH;i++) {
      if ((self->openqueue_vars).queue[i].owner==COMPONENT_SIXTOP_TO_IEEE802154E &&
          (self->openqueue_vars).queue[i].creator==COMPONENT_SIXTOP              &&
 packetfunctions_isBroadcastMulticast(self, &((self->openqueue_vars).queue[i].l2_nextORpreviousHop))) {
         ENABLE_INTERRUPTS();
         return &(self->openqueue_vars).queue[i];
      }
   }
   ENABLE_INTERRUPTS();
   return NULL;
}

//=========================== private =========================================

void openqueue_reset_entry(OpenMote* self, OpenQueueEntry_t* entry) {
   //admin
   entry->creator                      = COMPONENT_NULL;
   entry->owner                        = COMPONENT_NULL;
   entry->payload                      = &(entry->packet[127]);
   entry->length                       = 0;
   //l4
   entry->l4_protocol                  = IANA_UNDEFINED;
   //l3
   entry->l3_destinationAdd.type       = ADDR_NONE;
   entry->l3_sourceAdd.type            = ADDR_NONE;
   //l2
   entry->l2_nextORpreviousHop.type    = ADDR_NONE;
   entry->l2_frameType                 = IEEE154_TYPE_UNDEFINED;
   entry->l2_retriesLeft               = 0;
   entry->l2_IEListPresent             = 0;
}
