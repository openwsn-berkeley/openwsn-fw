#include "opendefs.h"
#include "openqueue.h"
#include "openserial.h"
#include "packetfunctions.h"
#include "IEEE802154E.h"
#include "IEEE802154_security.h"
#include "sixtop.h"
// telosb need debugpins to indicate ISR activity
#include "debugpins.h"

//=========================== defination =====================================

#define HIGH_PRIORITY_QUEUE_ENTRY 5

//=========================== variables =======================================

openqueue_vars_t openqueue_vars;

//=========================== prototypes ======================================
void openqueue_reset_entry(OpenQueueEntry_t* entry);

//=========================== public ==========================================

//======= admin

/**
\brief Initialize this module.
*/
void openqueue_init(void) {
   uint8_t i;
   for (i=0;i<QUEUELENGTH;i++){
      openqueue_reset_entry(&(openqueue_vars.queue[i]));
   }
}

/**
\brief Trigger this module to print status information, over serial.

debugPrint_* functions are used by the openserial module to continuously print
status information about several modules in the OpenWSN stack.

\returns TRUE if this function printed something, FALSE otherwise.
*/
bool debugPrint_queue(void) {
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

/**
\brief Request a new (free) packet buffer.

Component throughout the protocol stack can call this function is they want to
get a new packet buffer to start creating a new packet.

\note Once a packet has been allocated, it is up to the creator of the packet
      to free it using the openqueue_freePacketBuffer() function.

\returns A pointer to the queue entry when it could be allocated, or NULL when
         it could not be allocated (buffer full or not synchronized).
*/
OpenQueueEntry_t* openqueue_getFreePacketBuffer(uint8_t creator) {
   uint8_t i;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();

   // refuse to allocate if we're not in sync
   if (ieee154e_isSynch()==FALSE && creator > COMPONENT_IEEE802154E){
     ENABLE_INTERRUPTS();
     return NULL;
   }

   // if you get here, I will try to allocate a buffer for you

   // if there is no space left for high priority queue, don't reserve
   if (openqueue_isHighPriorityEntryEnough()==FALSE && creator>COMPONENT_SIXTOP_RES){
      ENABLE_INTERRUPTS();
      return NULL;
   }

   // walk through queue and find free entry
   for (i=0;i<QUEUELENGTH;i++) {
      if (openqueue_vars.queue[i].owner==COMPONENT_NULL) {
         openqueue_vars.queue[i].creator=creator;
         openqueue_vars.queue[i].owner=COMPONENT_OPENQUEUE;
         ENABLE_INTERRUPTS();
         return &openqueue_vars.queue[i];
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
owerror_t openqueue_freePacketBuffer(OpenQueueEntry_t* pkt) {
   uint8_t i;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   for (i=0;i<QUEUELENGTH;i++) {
      if (&openqueue_vars.queue[i]==pkt) {
         if (openqueue_vars.queue[i].owner==COMPONENT_NULL) {
            // log the error
            openserial_printCritical(COMPONENT_OPENQUEUE,ERR_FREEING_UNUSED,
                                  (errorparameter_t)0,
                                  (errorparameter_t)0);
         }
         openqueue_reset_entry(&(openqueue_vars.queue[i]));
         ENABLE_INTERRUPTS();
         return E_SUCCESS;
      }
   }
   // log the error
   openserial_printCritical(COMPONENT_OPENQUEUE,ERR_FREEING_ERROR,
                         (errorparameter_t)0,
                         (errorparameter_t)0);
   ENABLE_INTERRUPTS();
   return E_FAIL;
}

/**
\brief Free all the packet buffers created by a specific module.

\param creator The identifier of the component, taken in COMPONENT_*.
*/
void openqueue_removeAllCreatedBy(uint8_t creator) {
    uint8_t i;
    INTERRUPT_DECLARATION();
    DISABLE_INTERRUPTS();
    for (i=0;i<QUEUELENGTH;i++){
        if (
            openqueue_vars.queue[i].creator == creator &&
            openqueue_vars.queue[i].owner   != COMPONENT_IEEE802154E
        ) {
            openqueue_reset_entry(&(openqueue_vars.queue[i]));
        }
    }
    ENABLE_INTERRUPTS();
}

//======= called by RES

OpenQueueEntry_t* openqueue_sixtopGetSentPacket(void) {
   uint8_t i;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   for (i=0;i<QUEUELENGTH;i++) {
      if (openqueue_vars.queue[i].owner==COMPONENT_IEEE802154E_TO_SIXTOP &&
          openqueue_vars.queue[i].creator!=COMPONENT_IEEE802154E) {
         ENABLE_INTERRUPTS();
         return &openqueue_vars.queue[i];
      }
   }
   ENABLE_INTERRUPTS();
   return NULL;
}

OpenQueueEntry_t* openqueue_sixtopGetReceivedPacket(void) {
   uint8_t i;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   for (i=0;i<QUEUELENGTH;i++) {
      if (openqueue_vars.queue[i].owner==COMPONENT_IEEE802154E_TO_SIXTOP &&
          openqueue_vars.queue[i].creator==COMPONENT_IEEE802154E) {
         ENABLE_INTERRUPTS();
         return &openqueue_vars.queue[i];
      }
   }
   ENABLE_INTERRUPTS();
   return NULL;
}

uint8_t openqueue_getNum6PReq(open_addr_t* neighbor){

    uint8_t i;
    uint8_t num6Prequest;

    INTERRUPT_DECLARATION();
    DISABLE_INTERRUPTS();

    num6Prequest = 0;
    for (i=0;i<QUEUELENGTH;i++) {
        if (
            openqueue_vars.queue[i].owner   == COMPONENT_IEEE802154E_TO_SIXTOP  &&
            openqueue_vars.queue[i].creator == COMPONENT_SIXTOP_RES             &&
            openqueue_vars.queue[i].l2_sixtop_messageType == SIXTOP_CELL_REQUEST&&
            packetfunctions_sameAddress(neighbor,&openqueue_vars.queue[i].l2_nextORpreviousHop)
        ) {
            num6Prequest += 1;
        }
    }
    ENABLE_INTERRUPTS();
    return num6Prequest;
}

uint8_t openqueue_getNum6PResp(void){

    uint8_t i;
    uint8_t num6Presponse;

    INTERRUPT_DECLARATION();
    DISABLE_INTERRUPTS();

    num6Presponse = 0;
    for (i=0;i<QUEUELENGTH;i++) {
        if (
            openqueue_vars.queue[i].owner   == COMPONENT_IEEE802154E_TO_SIXTOP  &&
            openqueue_vars.queue[i].creator == COMPONENT_SIXTOP_RES             &&
            openqueue_vars.queue[i].l2_sixtop_messageType == SIXTOP_CELL_RESPONSE
        ) {
            num6Presponse += 1;
        }
    }
    ENABLE_INTERRUPTS();
    return num6Presponse;
}

void openqueue_remove6PrequestToNeighbor(open_addr_t* neighbor){

    uint8_t i;

    INTERRUPT_DECLARATION();
    DISABLE_INTERRUPTS();

    for (i=0;i<QUEUELENGTH;i++) {
        if (
            openqueue_vars.queue[i].owner   == COMPONENT_IEEE802154E_TO_SIXTOP   &&
            openqueue_vars.queue[i].creator == COMPONENT_SIXTOP_RES              &&
            openqueue_vars.queue[i].l2_sixtop_messageType == SIXTOP_CELL_REQUEST &&
            packetfunctions_sameAddress(neighbor,&openqueue_vars.queue[i].l2_nextORpreviousHop)
        ) {
            openqueue_reset_entry(&(openqueue_vars.queue[i]));
        }
    }
    ENABLE_INTERRUPTS();
}

//======= called by IEEE80215E

OpenQueueEntry_t* openqueue_macGet6PResponseAndDownStreamPacket(open_addr_t* toNeighbor) {
    uint8_t i;
    INTERRUPT_DECLARATION();
    DISABLE_INTERRUPTS();

    // first to look the sixtop RES packet
    for (i=0;i<QUEUELENGTH;i++) {
       if (
           openqueue_vars.queue[i].owner==COMPONENT_SIXTOP_TO_IEEE802154E &&
           openqueue_vars.queue[i].creator==COMPONENT_SIXTOP_RES &&
           (
               (
                   toNeighbor->type==ADDR_64B &&
                   packetfunctions_sameAddress(toNeighbor,&openqueue_vars.queue[i].l2_nextORpreviousHop)
               ) || toNeighbor->type==ADDR_ANYCAST
           ) &&
           openqueue_vars.queue[i].l2_sixtop_messageType == SIXTOP_CELL_RESPONSE
       ){
          ENABLE_INTERRUPTS();
          return &openqueue_vars.queue[i];
       }
    }

    // look for a packet which is either from openbridge or forwarding component by source routing
    for (i=0;i<QUEUELENGTH;i++) {
        if (
            openqueue_vars.queue[i].owner==COMPONENT_SIXTOP_TO_IEEE802154E &&
            (
                openqueue_vars.queue[i].creator==COMPONENT_OPENBRIDGE ||
                openqueue_vars.queue[i].l3_useSourceRouting == TRUE   ||
                openqueue_vars.queue[i].is_cjoin_response
            )
        ) {
            ENABLE_INTERRUPTS();
            return &openqueue_vars.queue[i];
        }
    }
    ENABLE_INTERRUPTS();
    return NULL;
}

OpenQueueEntry_t* openqueue_macGet6PRequestOnAnycast(open_addr_t* autonomousUnicastNeighbor){
    uint8_t i;
    INTERRUPT_DECLARATION();
    DISABLE_INTERRUPTS();

    for (i=0;i<QUEUELENGTH;i++) {
       if (
           openqueue_vars.queue[i].owner==COMPONENT_SIXTOP_TO_IEEE802154E &&
           openqueue_vars.queue[i].creator==COMPONENT_SIXTOP_RES &&
           (
               (
                    autonomousUnicastNeighbor == NULL ||
                    (
                        autonomousUnicastNeighbor->type==ADDR_64B &&
                        packetfunctions_sameAddress(autonomousUnicastNeighbor,&openqueue_vars.queue[i].l2_nextORpreviousHop) == FALSE
                    )
                )
           ) &&
           openqueue_vars.queue[i].l2_sixtop_messageType == SIXTOP_CELL_REQUEST
       ){
          ENABLE_INTERRUPTS();
          return &openqueue_vars.queue[i];
       }
    }

    ENABLE_INTERRUPTS();
    return NULL;
}

bool openqueue_isHighPriorityEntryEnough(void) {
    uint8_t i;
    uint8_t numberOfEntry;
    INTERRUPT_DECLARATION();
    DISABLE_INTERRUPTS();

    numberOfEntry = 0;
    for (i=0;i<QUEUELENGTH;i++) {
        if(openqueue_vars.queue[i].creator>COMPONENT_SIXTOP_RES){
            numberOfEntry++;
        }
    }

    if (numberOfEntry>QUEUELENGTH-HIGH_PRIORITY_QUEUE_ENTRY){
      ENABLE_INTERRUPTS();
        return FALSE;
    } else {
      ENABLE_INTERRUPTS();
        return TRUE;
    }
}

OpenQueueEntry_t* openqueue_macGetEBPacket(void) {
   uint8_t i;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   for (i=0;i<QUEUELENGTH;i++) {
      if (openqueue_vars.queue[i].owner==COMPONENT_SIXTOP_TO_IEEE802154E &&
          openqueue_vars.queue[i].creator==COMPONENT_SIXTOP              &&
          packetfunctions_isBroadcastMulticast(&(openqueue_vars.queue[i].l2_nextORpreviousHop))) {
         ENABLE_INTERRUPTS();
         return &openqueue_vars.queue[i];
      }
   }
   ENABLE_INTERRUPTS();
   return NULL;
}

OpenQueueEntry_t* openqueue_macGetKaPacket(open_addr_t* toNeighbor) {
    uint8_t i;
    INTERRUPT_DECLARATION();
    DISABLE_INTERRUPTS();
    for (i=0;i<QUEUELENGTH;i++) {
        if (openqueue_vars.queue[i].owner==COMPONENT_SIXTOP_TO_IEEE802154E &&
            openqueue_vars.queue[i].creator==COMPONENT_SIXTOP              &&
            toNeighbor->type==ADDR_64B                                     &&
            packetfunctions_sameAddress(toNeighbor,&openqueue_vars.queue[i].l2_nextORpreviousHop)
        ) {
            ENABLE_INTERRUPTS();
            return &openqueue_vars.queue[i];
        }
    }
    ENABLE_INTERRUPTS();
    return NULL;
}

OpenQueueEntry_t*  openqueue_macGetDIOPacket(){
    uint8_t i;
    INTERRUPT_DECLARATION();
    DISABLE_INTERRUPTS();
    for (i=0;i<QUEUELENGTH;i++) {
        if (openqueue_vars.queue[i].owner==COMPONENT_SIXTOP_TO_IEEE802154E &&
            openqueue_vars.queue[i].creator==COMPONENT_ICMPv6RPL           &&
            packetfunctions_isBroadcastMulticast(&(openqueue_vars.queue[i].l2_nextORpreviousHop))) {
            ENABLE_INTERRUPTS();
            return &openqueue_vars.queue[i];
        }
    }
    ENABLE_INTERRUPTS();
    return NULL;
}
/**
\Brief replace the upstream packet nexthop payload by given newNextHop address
\param newNextHop.
*/
void openqueue_updateNextHopPayload(open_addr_t* newNextHop){

    uint8_t i,j;
    INTERRUPT_DECLARATION();
    DISABLE_INTERRUPTS();

    for (i=0;i<QUEUELENGTH;i++) {
        if (
            openqueue_vars.queue[i].owner==COMPONENT_SIXTOP_TO_IEEE802154E &&
            (
                newNextHop->type==ADDR_64B &&
                packetfunctions_sameAddress(newNextHop,&openqueue_vars.queue[i].l2_nextORpreviousHop) == FALSE
            )
        ){
            if (
                openqueue_vars.queue[i].creator >= COMPONENT_FORWARDING &&
                openqueue_vars.queue[i].l3_useSourceRouting == FALSE
            ) {
                memcpy(&openqueue_vars.queue[i].l2_nextORpreviousHop, newNextHop, sizeof(open_addr_t));
                for (j=0;j<8;j++) {
                    *((uint8_t*)openqueue_vars.queue[i].l2_nextHop_payload+j) = newNextHop->addr_64b[j];
                }
            }
        }
    }

    ENABLE_INTERRUPTS();
}

OpenQueueEntry_t*  openqueue_macGetNonJoinIPv6Packet(open_addr_t* toNeighbor){
    uint8_t i;
    uint8_t packet_index;
    INTERRUPT_DECLARATION();
    DISABLE_INTERRUPTS();

    packet_index = QUEUELENGTH;
    // first to look the sixtop RES packet
    for (i=0;i<QUEUELENGTH;i++) {
       if (
           openqueue_vars.queue[i].owner==COMPONENT_SIXTOP_TO_IEEE802154E &&
           (
               toNeighbor->type==ADDR_64B &&
               packetfunctions_sameAddress(toNeighbor,&openqueue_vars.queue[i].l2_nextORpreviousHop)
           ) &&
           openqueue_vars.queue[i].creator >= COMPONENT_OPENBRIDGE &&
           openqueue_vars.queue[i].creator != COMPONENT_CJOIN
       ){
            if (packet_index==QUEUELENGTH){
                packet_index = i;
            } else {
                if (openqueue_vars.queue[i].creator<openqueue_vars.queue[packet_index].creator){
                    packet_index = i;
                }
            }
       }
    }

    if (packet_index == QUEUELENGTH){
        ENABLE_INTERRUPTS();
        return NULL;
    } else {
        ENABLE_INTERRUPTS();
        return &openqueue_vars.queue[packet_index];
    }
}

OpenQueueEntry_t*  openqueue_macGet6PandJoinPacket(open_addr_t* toNeighbor){
    uint8_t i;
    INTERRUPT_DECLARATION();
    DISABLE_INTERRUPTS();

    // first to look the sixtop RES packet
    for (i=0;i<QUEUELENGTH;i++) {
       if (
           openqueue_vars.queue[i].owner==COMPONENT_SIXTOP_TO_IEEE802154E &&
           (
               (
                    toNeighbor->type==ADDR_64B &&
                    packetfunctions_sameAddress(toNeighbor,&openqueue_vars.queue[i].l2_nextORpreviousHop)
                ) || toNeighbor->type==ADDR_ANYCAST // in case autonomous unicast cells is located at the same slotoffset of autonomous anycast cell, send 6p and cjoin packets on anycast cell as well
           ) &&
           (
               openqueue_vars.queue[i].creator == COMPONENT_SIXTOP_RES ||
               openqueue_vars.queue[i].creator == COMPONENT_CJOIN
           )
       ){
            ENABLE_INTERRUPTS();
            return &openqueue_vars.queue[i];
       }
    }

    ENABLE_INTERRUPTS();
    return NULL;
}


//=========================== private =========================================

void openqueue_reset_entry(OpenQueueEntry_t* entry) {
   //admin
   entry->creator                      = COMPONENT_NULL;
   entry->owner                        = COMPONENT_NULL;
   entry->payload                      = &(entry->packet[127 - IEEE802154_SECURITY_TAG_LEN]); // Footer is longer if security is used
   entry->length                       = 0;
   entry->is_cjoin_response            = FALSE;
   //l4
   entry->l4_protocol                  = IANA_UNDEFINED;
   entry->l4_protocol_compressed       = FALSE;
   //l3
   entry->l3_destinationAdd.type       = ADDR_NONE;
   entry->l3_sourceAdd.type            = ADDR_NONE;
   entry->l3_useSourceRouting          = FALSE;
   //l2
   entry->l2_nextORpreviousHop.type    = ADDR_NONE;
   entry->l2_frameType                 = IEEE154_TYPE_UNDEFINED;
   entry->l2_retriesLeft               = 0;
   entry->l2_IEListPresent             = 0;
   entry->l2_isNegativeACK             = 0;
   entry->l2_payloadIEpresent          = 0;
   entry->l2_sendOnTxCell              = FALSE;
   //l2-security
   entry->l2_securityLevel             = 0;
}
