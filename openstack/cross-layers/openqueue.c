#include "opendefs.h"
#include "openqueue.h"
#include "openserial.h"
#include "packetfunctions.h"
#include "IEEE802154E.h"
#include "ieee802154_security_driver.h"
#include "sixtop.h"

//=========================== variables =======================================

openqueue_vars_t openqueue_vars;

//=========================== prototypes ======================================

void openqueue_reset_entry(OpenQueueEntry_t* entry);

//=========================== public ==========================================




//#define _DEBUG_OQ_MEM_



//======= admin

/**
\brief Initialize this module.
*/
void openqueue_init() {
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
bool debugPrint_queue() {
   debugOpenQueueEntry_t output;
   uint8_t  row;

   //push several rows at the same time
   for(row=0; row<QUEUE_NBROWS_OPENSERIALSTATUS; row++){


      openqueue_vars.debugPrintRow         = (openqueue_vars.debugPrintRow+1)%QUEUELENGTH;

      output.row     = openqueue_vars.debugPrintRow;
      output.creator = openqueue_vars.queue[openqueue_vars.debugPrintRow].creator;
      output.owner   = openqueue_vars.queue[openqueue_vars.debugPrintRow].owner;


      //ASN to push (in ASN format)
      output.timeout.byte4 = openqueue_vars.queue[openqueue_vars.debugPrintRow].timeout.byte[4];
      output.timeout.bytes0and1 =
            openqueue_vars.queue[openqueue_vars.debugPrintRow].timeout.byte[0] +
            openqueue_vars.queue[openqueue_vars.debugPrintRow].timeout.byte[1] * 256;
      output.timeout.bytes2and3 =
            openqueue_vars.queue[openqueue_vars.debugPrintRow].timeout.byte[2] +
            openqueue_vars.queue[openqueue_vars.debugPrintRow].timeout.byte[3] *256;


      //track (instance + owner)
      output.trackInstance                  = \
            (uint16_t)openqueue_vars.queue[openqueue_vars.debugPrintRow].l2_track.instance;
      memcpy(
                  &output.trackOwner,
                  &(openqueue_vars.queue[openqueue_vars.debugPrintRow].l2_track.owner),
                  sizeof(open_addr_t)
            );

      //l2 next hop
      memcpy(
             &output.nextHop,
             &(openqueue_vars.queue[openqueue_vars.debugPrintRow].l2_nextORpreviousHop),
             sizeof(open_addr_t)
       );

      openserial_printStatus(
            STATUS_QUEUE,
            (uint8_t*)&output,
            sizeof(debugOpenQueueEntry_t));

   }
   return TRUE;
}

//this represents an invalid timeout
bool openqueue_timeout_is_zero(timeout_t value){
   uint8_t  i;

   for (i=0; i<5; i++)
      if (value.byte[i] != 0)
      return FALSE;

   return(TRUE);
}

//is a >= b AND b != INVALID
bool openqueue_timeout_is_greater(timeout_t a, timeout_t b){
   uint8_t  i;

   //invalid timeout
   if(openqueue_timeout_is_zero(b))
      return(FALSE);

   //for each byte (byte 4 is the biggest)
   for(i=sizeof(timeout_t)-1; i>=0 && i<=sizeof(timeout_t)-1; i--)
      if (a.byte[i] > b.byte[i])
         return(TRUE);
      else if (a.byte[i] < b.byte[i])
         return(FALSE);

   //equal case
   return(TRUE);
}


//returns a - b (or 0 if b > a)
uint64_t openqueue_timeout_diff(timeout_t a, timeout_t b){
   uint8_t  i, max;
   bool     greater = FALSE;
   uint64_t diff = 0;

   max = sizeof(timeout_t) - 1;
   for(i=max ; i>=0 && i<=max ; i--){

      //returns 0 if a < b
      if (!greater && a.byte[i] < b.byte[i])
         return(0);

      // we have the first different byte (a > b)
      else if (a.byte[i] > b.byte[i])
         greater = TRUE;

      //computes the difference
      diff = diff * 256 + (a.byte[i] - b.byte[i]);
   }

   return(diff);
}


//remove the packets which are timeouted in the queue
void openqueue_timeout_drop(void){

   uint8_t     i;
   timeout_t   now;

   return;

   //initialization
   ieee154e_getAsn(now.byte);


   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   for (i=0;i<QUEUELENGTH;i++) {

      if (openqueue_vars.queue[i].creator != COMPONENT_NULL)
         if (!openqueue_timeout_is_zero(openqueue_vars.queue[i].timeout))
            if (openqueue_timeout_is_greater(now, openqueue_vars.queue[i].timeout)){

               openserial_statPktTimeout(&(openqueue_vars.queue[i]));
#ifdef _DEBUG_OQ_MEM_
               char str[150];
               sprintf(str, "rem(timeout), pos=");
               openserial_ncat_uint32_t(str, (uint32_t)i, 150);
               openserial_printf(COMPONENT_OPENQUEUE, str, strlen(str));
#endif

               char str[150];
               sprintf(str, "PKT TIMEOUTED");
               openserial_printf(COMPONENT_OPENQUEUE, str, strlen(str));

               notif_sendDone(&(openqueue_vars.queue[i]), E_FAIL);
               openqueue_reset_entry(&(openqueue_vars.queue[i]));

            }
   }


   ENABLE_INTERRUPTS();

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
   
   // walk through queue and find free entry
   for (i=0;i<QUEUELENGTH;i++) {
      if (openqueue_vars.queue[i].owner == COMPONENT_NULL) {
         bzero(openqueue_vars.queue[i].timeout.byte, sizeof(timeout_t));
         openqueue_vars.queue[i].creator=creator;
         openqueue_vars.queue[i].owner=COMPONENT_OPENQUEUE;
         ENABLE_INTERRUPTS(); 
         return &openqueue_vars.queue[i];
      }
   }
   ENABLE_INTERRUPTS();
   return NULL;
}


//======= called by any component



/**
\brief Fix a timeout (in ms) for a packet

\param entry A pointer to the entry to configure.
\param duration_ms The timer after which this packet should be removed from the queue, even it it was not transmitted.

*/

void openqueue_set_timeout(OpenQueueEntry_t* entry, const uint32_t duration_ms){
   timeout_t     now;
   uint8_t       remainder, i;
   uint64_t      diff;
   timeout_t     duration_asn;

   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();

   //no packet is available
   if (entry == NULL){
      ENABLE_INTERRUPTS();
      return;
   }

   //diff in nb of slots
   //+1 to upper ceil the nb. of slots
   diff = (uint64_t) duration_ms / TSLOTDURATION_MS + 1;

   //offset in ASN format
   bzero(duration_asn.byte, sizeof(timeout_t));
   for(i=sizeof(timeout_t)-1; i>=0  && i<=sizeof(timeout_t)-1; i--){
      duration_asn.byte[i] = (uint8_t)(diff >> (8*i));
      diff -= (uint64_t)duration_asn.byte[4] << (8*i);
   }

   //translates the duration into an ASN
   ieee154e_getAsn(now.byte);
   remainder = 0;
   for(i=0; i<sizeof(timeout_t); i++){
      entry->timeout.byte[i] = duration_asn.byte[i] + now.byte[i] + remainder;
      if (entry->timeout.byte[i] < duration_asn.byte[i] && entry->timeout.byte[i] < now.byte[i])
         remainder = 1;
      else
         remainder = 0;
   }

   ENABLE_INTERRUPTS();
   return;
}



/**
\brief Request a new (free) packet buffer, specifying a timeout (in ms)

\note Once a packet has been allocated, it is up to the creator of the packet
      to free it using the openqueue_freePacketBuffer() function.

\returns A pointer to the queue entry when it could be allocated, or NULL when
         it could not be allocated (buffer full or not synchronized).
*/
OpenQueueEntry_t* openqueue_getFreePacketBuffer_with_timeout(uint8_t creator, const uint32_t duration_ms) {

   // a new entry in the queue
   OpenQueueEntry_t* entry;
   entry = openqueue_getFreePacketBuffer(creator);

   openqueue_set_timeout(entry, duration_ms);
   return(entry);
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
      if (openqueue_vars.queue[i].creator == creator) {

         openqueue_reset_entry(&(openqueue_vars.queue[i]));

#ifdef _DEBUG_OQ_MEM_
         char str[150];
         sprintf(str, "remove (AllCreatedBy), pos=");
         openserial_ncat_uint32_t(str, (uint32_t)i, 150);
         openserial_printf(COMPONENT_OPENQUEUE, str, strlen(str));
#endif

      }
   }
   ENABLE_INTERRUPTS();
}



/**
\brief Free this packet entry (called by components for which a packet has been enqueued for a too long time (e.g.))

\param the entry to remove in the queue
*/

void openqueue_removeEntry(OpenQueueEntry_t* entry){
   openqueue_reset_entry(entry);
}


/**
\brief Free all the packet buffers owned by a specific module.

\param owner The identifier of the component, taken in COMPONENT_*.
*/
void openqueue_removeAllOwnedBy(uint8_t owner) {
   uint8_t i;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   for (i=0;i<QUEUELENGTH;i++){
      if (openqueue_vars.queue[i].owner==owner) {
         openqueue_reset_entry(&(openqueue_vars.queue[i]));
      }
   }
   ENABLE_INTERRUPTS();
}

/**
\brief Count the number of packets in the queue with a specific track.

\param id of the track.
\returns the number of packets with track
*/

 uint8_t openqueue_count_track(track_t track) {
   uint8_t i;
   uint8_t resVal = 0;

   for (i=0;i<QUEUELENGTH;i++){
      if(
            sixtop_is_trackequal(openqueue_vars.queue[i].l2_track, track)
            &&
            openqueue_vars.queue[i].creator != COMPONENT_NULL
            )
         resVal++;
   }
   return(resVal);
}


//======= called by RES

OpenQueueEntry_t* openqueue_sixtopGetSentPacket() {
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

OpenQueueEntry_t* openqueue_sixtopGetReceivedPacket() {
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

//======= called by IEEE80215E

OpenQueueEntry_t* openqueue_macGetDataPacket(open_addr_t* toNeighbor, track_t *track) {
   uint8_t i;
   OpenQueueEntry_t *entry = NULL;

   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   if (toNeighbor->type==ADDR_64B) {
      // a neighbor is specified, look for a packet unicast to that neigbhbor
      for (i=0;i<QUEUELENGTH;i++) {

         if (openqueue_vars.queue[i].owner==COMPONENT_SIXTOP_TO_IEEE802154E &&
               packetfunctions_sameAddress_debug(toNeighbor, &openqueue_vars.queue[i].l2_nextORpreviousHop,COMPONENT_OPENQUEUE) &&
               (openqueue_vars.queue[i].l2_track.instance == track->instance) &&
               //either NULL track of the correct one
               (
                     (track->instance == TRACK_BESTEFFORT)
                     ||
                     (packetfunctions_sameAddress_debug(&(openqueue_vars.queue[i].l2_track.owner), &(track->owner),COMPONENT_OPENQUEUE))
               )&&
               ((entry == NULL) || (openqueue_timeout_is_greater(entry->timeout, openqueue_vars.queue[i].timeout)))
         ) {
            entry = &(openqueue_vars.queue[i]);
         }
      }
   } else if (toNeighbor->type==ADDR_ANYCAST) {
      // anycast case: look for a packet which is either not created by RES
      // or an KA (created by RES, but not broadcast)
      for (i=0;i<QUEUELENGTH;i++) {
         if ((openqueue_vars.queue[i].l2_track.instance == TRACK_BESTEFFORT) &&
               (openqueue_vars.queue[i].owner == COMPONENT_SIXTOP_TO_IEEE802154E) &&
               (openqueue_vars.queue[i].creator != COMPONENT_SIXTOP ||
                  (
                     openqueue_vars.queue[i].creator==COMPONENT_SIXTOP &&
                     packetfunctions_isBroadcastMulticast_debug(&(openqueue_vars.queue[i].l2_nextORpreviousHop), 71)==FALSE
                  )
                ) &&
                ((entry == NULL) || (openqueue_timeout_is_greater(entry->timeout, openqueue_vars.queue[i].timeout)))
              ) {
            entry = &(openqueue_vars.queue[i]);
         }
      }
   }
   ENABLE_INTERRUPTS();
   return entry;
}


OpenQueueEntry_t* openqueue_macGetEBPacket() {
   uint8_t i;

   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   for (i=0;i<QUEUELENGTH;i++) {
      if (openqueue_vars.queue[i].owner==COMPONENT_SIXTOP_TO_IEEE802154E &&
          openqueue_vars.queue[i].creator==COMPONENT_SIXTOP              &&
          packetfunctions_isBroadcastMulticast_debug(&(openqueue_vars.queue[i].l2_nextORpreviousHop),72)) {
         ENABLE_INTERRUPTS();
         return &openqueue_vars.queue[i];
      }
   }
   ENABLE_INTERRUPTS();
   return NULL;
}


//returns the i^th packet of the queue (called by OTF to walk enough resource is allocated for the present queue)
OpenQueueEntry_t* openqueue_getPacket(uint8_t i) {
   return (&(openqueue_vars.queue[i]));
}





//not enough space for non prioritar packets
bool openqueue_overflow_for_data(void){
   uint8_t  nb = 0;
   uint8_t  i;

   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   for (i=0;i<QUEUELENGTH;i++)
      if (openqueue_vars.queue[i].creator == COMPONENT_NULL)
         nb++;
   ENABLE_INTERRUPTS();

   //for debug
 /*  if(nb <= QUEUELENGTH_RESERVED)
      openserial_printError(
               COMPONENT_OPENQUEUE,
               ERR_OPENQUEUE_BUFFER_OVERFLOW,
               (errorparameter_t)nb,
               (errorparameter_t)QUEUELENGTH_RESERVED
            );
*/
   return(nb <= QUEUELENGTH_RESERVED);
}




//=========================== private =========================================

void openqueue_reset_entry(OpenQueueEntry_t* entry) {
   //admin
   entry->creator                      = COMPONENT_NULL;
   entry->owner                        = COMPONENT_NULL;
   entry->payload                      = &(entry->packet[127 - IEEE802154_SECURITY_TAG_LEN]); // Footer is longer if security is used
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
   entry->l2_payloadIEpresent          = 0;
   //entry->l2_track.owner.type          = ADDR_NONE;
   //entry->l2_track.instance            = TRACK_BESTEFFORT;
   bzero(&(entry->l2_track), sizeof(entry->l2_track));
   bzero(entry->timeout.byte, sizeof(timeout_t));
   //l2-security
   entry->l2_securityLevel             = 0;
}






//make a local copy of the entry to push it to openbridge
/*
OpenQueueEntry_t* openqueue_copy_for_openbridge(OpenQueueEntry_t* pkt){

   memcpy(&(openqueue_vars.openbridge), pkt, sizeof(OpenQueueEntry_t));
   return(&(openqueue_vars.openbridge));
}
*/
