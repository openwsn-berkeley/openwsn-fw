#include "opendefs.h"
#include "opentimers.h"
#include "openrandom.h"
#include "packetfunctions.h"
#include "sixtop.h"
#include "iphc.h"
#include "fragment.h"
#include "forwarding.h"
#include "openbridge.h"

//=========================== variables =======================================

fragmentqueue_vars_t fragmentqueue_vars;

//=========================== prototypes ======================================

// it will be public ?
uint16_t fragment_getNewTag(void);

// package management
owerror_t fragment_reservePkt(FragmentQueueEntry_t* buffer, uint8_t fragment);
void fragment_finishPkt(FragmentQueueEntry_t*  buffer,
                        FragmentOffsetEntry_t* fragment,
                        owerror_t              error);
void fragment_tryToSend(FragmentQueueEntry_t* buffer);

// to implement on lower layers
uint8_t fragment_askL2HeaderSize(OpenQueueEntry_t* msg);

// message management
void fragment_setSize(OpenQueueEntry_t* msg, uint16_t size);
void fragment_setTag(OpenQueueEntry_t* msg, uint16_t tag);
void fragment_setOffset(OpenQueueEntry_t* msg, uint8_t offset);
uint16_t fragment_getSize(OpenQueueEntry_t* msg);
uint16_t fragment_getTag(OpenQueueEntry_t* msg);
uint8_t fragment_getOffset(OpenQueueEntry_t* msg);

// buffer management
FragmentQueueEntry_t* fragment_searchBuffer(OpenQueueEntry_t* msg, bool in);
owerror_t fragment_freeBuffer(FragmentQueueEntry_t* buffer);
void fragment_resetBuffer(FragmentQueueEntry_t* buffer);
owerror_t fragment_restartBuffer(FragmentQueueEntry_t* buffer);
uint8_t fragment_bufferIndex(FragmentQueueEntry_t* buffer);

bool fragment_completeRX(FragmentQueueEntry_t* buffer);
// timer
void fragment_disableTimer(FragmentQueueEntry_t* buffer);
void fragment_activateTimer(FragmentQueueEntry_t* buffer);

// action functions
void fragment_action(FragmentQueueEntry_t* buffer);
void fragment_cancel(FragmentQueueEntry_t* buffer);
void fragment_assemble(FragmentQueueEntry_t* buffer);
void fragment_forward(FragmentQueueEntry_t* buffer);
void fragment_openbridge(FragmentQueueEntry_t* buffer);

// implemented in ../crosslayers/openqueue.c
// not published there as only used here
owerror_t openqueue_freePacketBuffer_atomic(OpenQueueEntry_t* pkt);

//=========================== debug ==========================================
//
void fragment_listFragments(FragmentQueueEntry_t* buffer) {
   uint8_t i;

   printf("Frag\tSize\tOffset\t Total=%d\n", buffer->msg->length);
   for ( i = 0; i < buffer->number; i++ ) {
      printf("%d\t%d\t%d\n", i, buffer->list[i].fragment_size, buffer->list[i].fragment_offset);
   }
}

//=========================== public ==========================================

/**
\brief Initialize this module.
*/
void fragment_init() {
   uint8_t i;

   for (i=0;i<FRAGQLENGTH;i++) {
      fragmentqueue_vars.queue[i].in_use  = FRAGMENT_NONE;
      fragmentqueue_vars.queue[i].timerId = TOO_MANY_TIMERS_ERROR;
      fragment_resetBuffer(&fragmentqueue_vars.queue[i]);
   }
   fragmentqueue_vars.tag = openrandom_get16b();
}

/**
\brief Pointer to the Fragment struct at position id

\paran id Buffer position in queue

\returns A pointer to a Fragment buffer struct
*/
FragmentQueueEntry_t* fragment_indexBuffer(uint8_t id) {
   return &(fragmentqueue_vars.queue[id]);
}

/**
\brief Adds fragmentation headers.
       Fragments message in multiple packets, if neccessary.

\note  When fragmentation headers must be added, it actually do not send
       fragments: it just build a list of fragments to send, based on their
       sizes and offsets.
       Message can get this point in the form of large packet (assumed it must
       be fragmented) or of normal packet but do not fit into a single frame,
       so its length is tested before shipping

\param msg A pointer to the packet buffer to send.

\returns E_SUCCESS when sending could be started.
\returns E_FAIL when msg cannot be send.
*/
owerror_t fragment_prependHeader(OpenQueueEntry_t* msg) {

   uint8_t               i;
   uint8_t               l2_hsize;
   uint8_t               iphc_hsize;
   uint8_t               max_fragment;
   uint8_t               actual_frag_size;
   uint16_t              actual_sent;
   FragmentQueueEntry_t* buffer;

   l2_hsize     = fragment_askL2HeaderSize(msg);
   max_fragment = 125 - l2_hsize;
   if ( msg->length <= max_fragment && msg->big == NULL )
      return  sixtop_send(msg);

   msg->owner       = COMPONENT_FRAGMENT;
   iphc_hsize       = msg->length - msg->l4_length;
   max_fragment     = 125 - l2_hsize - 4;
   actual_frag_size = max_fragment & 0xF8; // =/8*8;
   // RFC 4944 page 11: "The first link fragment SHALL contain
   // the first fragment header.
   if ( actual_frag_size < iphc_hsize ) {
      openserial_printError(COMPONENT_FRAGMENT, ERR_6LOWPAN_UNSUPPORTED,
                            (errorparameter_t)0,
			    (errorparameter_t)0);
      return E_FAIL;
   }

   buffer = fragment_searchBuffer(msg, FALSE);
   if (buffer== NULL) {
      openserial_printError(COMPONENT_FRAGMENT, ERR_NO_FREE_FRAGMENT_BUFFER,
                             (errorparameter_t)0,
			     (errorparameter_t)0);
      return E_FAIL;
   }
   buffer->msg = msg;
   buffer->datagram_tag  = fragment_getNewTag();
   buffer->datagram_size = msg->length;

   buffer->list[0].fragment_size = actual_frag_size;
   buffer->list[0].state         = FRAGMENT_ASSIGNED;

   actual_sent      = actual_frag_size;
   max_fragment     = 125 - l2_hsize - 5;
   actual_frag_size = max_fragment & 0xF8;
   i = 1;
   while ( actual_sent < msg->length ) {
      // last fragment?
      if ( actual_frag_size > msg->length - actual_sent )
         actual_frag_size = msg->length - actual_sent;
      buffer->list[i].fragment_size   = actual_frag_size;
      buffer->list[i].fragment_offset = actual_sent>>3;
      buffer->list[i].state           = FRAGMENT_ASSIGNED;

      actual_sent += actual_frag_size;
      i++;
   }
   buffer->number  = i;
   buffer->sending = 0;

   //fragment_listFragments(buffer);

   // Start sending the different frames
   fragment_tryToSend(buffer);

   return E_SUCCESS;
}

// Helpful defines
#define BUFFER_EQUAL(x,y) \
        ((x.fragment_offset==y.fragment_offset)&& \
	 (x.fragment_size  ==y.fragment_size))
#define BUFFER_PART_OVERLAP(x,y) \
        ((x.fragment_offset<y.fragment_offset)&& \
	 (y.fragment_offset*8<x.fragment_offset*8+x.fragment_size))
#define BUFFER_OVERLAP(x,y) \
        ((BUFFER_PART_OVERLAP(x,y))||(BUFFER_PART_OVERLAP(y,x)))

/**
\brief Seek dispatch to determine if incoming packet is a fragment.
       If part of a bigger packet, try to reassemble it. If not,
       relay to iphc

\param msg A pointer to the incoming packet.

\note  This function must process the datagram to assemble the message,
       releasing received packets until the message has been received
       completely and proceed with the usual processing.
       Instead, the decision is delayed until it received the initial
       fragment containing the header which is the only being processed.
       This message will only be processed if appropriate treatment has
       been determined the action to take with it.

\returns none.
*/
void fragment_retrieveHeader(OpenQueueEntry_t* msg) {
   uint8_t                 i;
   uint8_t                 dispatch;
   uint8_t                 offset;
   FragmentQueueEntry_t*   buffer;
   FragmentOffsetEntry_t   tempFO;
   bool                    fragn;
   uint16_t                size;

   msg->owner = COMPONENT_FRAGMENT;
   fragn = TRUE;
   dispatch = (*((uint8_t*)(msg->payload)) >> LOWPAN_DISPATCH) & 0x07; // 3b

   switch (dispatch) {
      case LOWPAN_DISPATCH_FRAG1:
         fragn = FALSE;
      case LOWPAN_DISPATCH_FRAGN:
	 break;
      default: // It is not a fragment
	 iphc_receive(msg);
	 return;
   }

   msg->payload[0] &= 0x07; // Filter dispatch
   buffer = fragment_searchBuffer(msg, TRUE);
   if (buffer==NULL) {
      openserial_printError(COMPONENT_FRAGMENT, ERR_NO_FREE_FRAGMENT_BUFFER,
                             (errorparameter_t)0,
			     (errorparameter_t)0);
      // abort
      openqueue_freePacketBuffer(msg);
      return;
   }

   size   = msg->length - (fragn ? 5 : 4);
   offset = fragn ? fragment_getOffset(msg) : 0;
   // data size must be a multiple of 8 or final fragment
   if(!(   (size%8==0)
        || (offset*8+size==buffer->datagram_size))) {
      openserial_printError(COMPONENT_FRAGMENT, ERR_INPUTBUFFER_LENGTH,
                              (errorparameter_t)0,
			      (errorparameter_t)0);
      // abort
      openqueue_freePacketBuffer(msg);
      if ( buffer->number == 0 )
         fragment_freeBuffer(buffer);
      return;
   }

/*
 * Is is possible? A message fragmented on just one fragment.
 *
   if (!fragn && size == buffer->datagram_size) {
      fragment_freeBuffer(buffer);
      packetfunctions_tossHeader(msg, 4);
      iphc_receive(msg);
      return;
   }
*/

   tempFO.fragment_size   = size;
   tempFO.fragment_offset = offset;
   for (i=0;i<buffer->number;i++) {
      if (BUFFER_EQUAL(buffer->list[i],tempFO)) {
         // Duplicated fragment
	 openqueue_freePacketBuffer(msg);
	 return;
      } else if (BUFFER_OVERLAP(buffer->list[i],tempFO)) {
         // Current fragment overlaps previous one
	 // RFC 4944: If a link fragment that overlaps another fragment
	 // is received ... buffer SHALL be discarded.
	 fragment_restartBuffer(buffer);
         // log error
         openserial_printError(COMPONENT_FRAGMENT, ERR_INPUTBUFFER_OVERLAPS,
                                (errorparameter_t)0,
				(errorparameter_t)0);
	 break;
      }
   }
   buffer->list[buffer->number].fragment_offset = offset;
   buffer->list[buffer->number].fragment_size   = size;
   buffer->list[buffer->number].fragment        = msg;
   buffer->list[buffer->number].state = FRAGMENT_RECEIVED;
   buffer->number++;
   packetfunctions_tossHeader(msg,fragn ? 5 : 4);

   // First received fragment: activate timer.
   if (buffer->number==1)
      fragment_activateTimer(buffer);
   // last received fragment = completed message: stop timer
   else if ( fragment_completeRX(buffer) )
      fragment_disableTimer(buffer);

   // Initial fragment
   if (! fragn) {
      buffer->msg = msg;
      iphc_receive(msg);
   // Initial fragment processed: process this one
   } else if (buffer->action != FRAGMENT_ACTION_NONE)
      fragment_action(buffer);
}

void fragment_sendDone(OpenQueueEntry_t *msg, owerror_t error) {
   uint8_t                i;
   uint8_t                j;
   FragmentOffsetEntry_t* fragment;
   FragmentQueueEntry_t*  buffer;
   INTERRUPT_DECLARATION();

   buffer = NULL;

   msg->owner = COMPONENT_FRAGMENT;

   if (msg->creator!=COMPONENT_FRAGMENT) {
      iphc_sendDone(msg, error);
      return;
   }

   // Search for the fragmented message
   DISABLE_INTERRUPTS();
   for (i=0;i<FRAGQLENGTH;i++)
      if (fragmentqueue_vars.queue[i].in_use != FRAGMENT_NONE)
         for (j=0;j<fragmentqueue_vars.queue[i].number; j++)
             if (fragmentqueue_vars.queue[i].list[j].fragment==msg) {
                buffer   = &(fragmentqueue_vars.queue[i]);
		fragment = &(buffer->list[j]);
             }
   if ( buffer != NULL )
      buffer->sending--;
   ENABLE_INTERRUPTS();

   if ( buffer == NULL ) {
      openserial_printError(COMPONENT_FRAGMENT,ERR_UNEXPECTED_SENDDONE,
                             (errorparameter_t)0,
			     (errorparameter_t)0);
      return;
   }

   // Decide what to do
   fragment_finishPkt(buffer, fragment, error);
}

FragmentQueueEntry_t* fragment_searchBufferFromMsg(OpenQueueEntry_t* msg) {
   uint8_t i;
   INTERRUPT_DECLARATION();

   DISABLE_INTERRUPTS();
   for ( i = 0; i < FRAGQLENGTH; i++ )
      if ( fragmentqueue_vars.queue[i].msg == msg ) {
         ENABLE_INTERRUPTS();
	 return &(fragmentqueue_vars.queue[i]);
      }
   ENABLE_INTERRUPTS();
   return NULL;
}

void fragment_assignAction(FragmentQueueEntry_t* buffer, FragmentAction action) {
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   buffer->action = action;
   ENABLE_INTERRUPTS();

   fragment_action(buffer);
}

//=========================== private =========================================

/**
\brief Returns a fragment tag and increments old one.
*/
uint16_t fragment_getNewTag(void) {
   uint16_t ntag;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   ntag = fragmentqueue_vars.tag;
   fragmentqueue_vars.tag = (ntag==0xFFFF) ? 0 : ntag+1;
   ENABLE_INTERRUPTS();

   return ntag;
}

bool fragment_completeRX(FragmentQueueEntry_t* buffer) {
   uint8_t  i;
   uint16_t received;
   INTERRUPT_DECLARATION();

   received = 0;
   DISABLE_INTERRUPTS();
   for ( i = 0; i < buffer->number; i++ )
      received += buffer->list[i].fragment_size;

   ENABLE_INTERRUPTS();
   return received == buffer->datagram_size;
}

/**
\brief Store a fragment in a packet to send it

\note Must be called in interrupted mode
*/
owerror_t fragment_reservePkt(FragmentQueueEntry_t* buffer, uint8_t fragment) {
   OpenQueueEntry_t* pkt;
   uint16_t          actual_sent;
   uint16_t          actual_frag_size;
   uint8_t           offset;
   INTERRUPT_DECLARATION();

   // DISABLE_INTERRUPTS(); unneeded
   if (buffer->list[fragment].state != FRAGMENT_ASSIGNED) {
      ENABLE_INTERRUPTS();
      openserial_printError(COMPONENT_FRAGMENT, ERR_FRAG_RESERVING,
                             (errorparameter_t)0,
			     (errorparameter_t)0);
      DISABLE_INTERRUPTS();
      return E_FAIL;
   }

   buffer->list[fragment].state = FRAGMENT_RESERVING;
   ENABLE_INTERRUPTS();
   pkt = openqueue_getFreePacketBuffer(COMPONENT_FRAGMENT);
   DISABLE_INTERRUPTS();
   if (pkt==NULL) {
      buffer->list[fragment].state = FRAGMENT_ASSIGNED;
      return E_FAIL;
   }
   pkt->creator = COMPONENT_FRAGMENT;
   pkt->owner   = COMPONENT_FRAGMENT;

   actual_frag_size = buffer->list[fragment].fragment_size;
   offset           = buffer->list[fragment].fragment_offset;
   actual_sent      = (fragment == 0) ? 0 : (offset<<3);
   ENABLE_INTERRUPTS();
   if ( actual_frag_size > 0 ) {
      packetfunctions_reserveHeaderSize(pkt, actual_frag_size * sizeof(uint8_t));
      memcpy(pkt->payload, buffer->msg->payload+actual_sent, actual_frag_size * sizeof(uint8_t));
   }
   if ( fragment != 0 ) { // offset
      packetfunctions_reserveHeaderSize(pkt, sizeof(uint8_t));
      fragment_setOffset(pkt, offset);
   }
   // tag
   packetfunctions_reserveHeaderSize(pkt, 2 * sizeof(uint8_t));
   fragment_setTag(pkt, buffer->datagram_tag);
   // size
   packetfunctions_reserveHeaderSize(pkt, 2 * sizeof(uint8_t));
   fragment_setSize(pkt, buffer->datagram_size);
   if ( fragment == 0 )
      pkt->payload[0] |= (LOWPAN_DISPATCH_FRAG1 << LOWPAN_DISPATCH);
   else
      pkt->payload[0] |= (LOWPAN_DISPATCH_FRAGN << LOWPAN_DISPATCH);

   // Add L2 information
   memcpy(&(pkt->l2_nextORpreviousHop), &(buffer->msg->l2_nextORpreviousHop),
          sizeof(open_addr_t));
   DISABLE_INTERRUPTS();
   buffer->list[fragment].fragment = pkt;
   buffer->list[fragment].state = FRAGMENT_RESERVED;

   return E_SUCCESS;
}

/**
\brief Free fragment and determine if try to send again or relay to IPHC

\note It can be called from timeout too when message is an outgoing one.
      In this case, fragment can be NULL.
*/
void fragment_finishPkt(FragmentQueueEntry_t*  buffer,
                        FragmentOffsetEntry_t* fragment,
                        owerror_t              error) {
   bool              sent;
   bool              fwend; // forwarding ended
   OpenQueueEntry_t* pkt;
   INTERRUPT_DECLARATION();

   sent = FALSE;
   DISABLE_INTERRUPTS();
   if (fragment!=NULL) {
      pkt = fragment->fragment;
      fragment->fragment = NULL;
   }
   if ( error == E_FAIL ) {
      if ( buffer->in_use == FRAGMENT_FW )
         buffer->msg->creator = COMPONENT_FORWARDING;
      buffer->in_use = FRAGMENT_FAIL;
   }

   if ( error == E_SUCCESS ) { 
      buffer->sent++;
      fragment->state = FRAGMENT_FINISHED;
      // have all the fragments been sent? 
      sent = (buffer->sent == buffer->number);
   }
   fwend = (buffer->in_use == FRAGMENT_FW) && fragment_completeRX(buffer);
   ENABLE_INTERRUPTS();
   if ( pkt != NULL && pkt != buffer->msg )
      openqueue_freePacketBuffer(pkt);

   // if no error and not finished, send another fragment
   if ( !sent && (buffer->in_use != FRAGMENT_FAIL) )
      fragment_tryToSend(buffer);
   
   // Message cannot be sent or alrady is
   else {
      // Check if previous was wrong
      if ( buffer->in_use == FRAGMENT_FAIL )
         error = E_FAIL;
      // Free Buffer and relay:
      // wait for pending fragments
      if ( fwend || buffer->sending == 0 ) {
         pkt = buffer->msg;
	 if ( buffer->in_use == FRAGMENT_FW )
            pkt->creator = COMPONENT_FORWARDING;
         fragment_freeBuffer(buffer);
         iphc_sendDone(pkt, error);
      }
   }
}

/**
\brief Try to send as many fragments as authorized maximum
*/
void fragment_tryToSend(FragmentQueueEntry_t* buffer) {
   uint8_t           i;
   uint8_t           reserved;
   owerror_t         error;
   OpenQueueEntry_t* pkt;
   INTERRUPT_DECLARATION();

   DISABLE_INTERRUPTS();

   // Search for actual used fragments for TX
   reserved = 0;
   if ( buffer->in_use == FRAGMENT_TX )
      for ( i = 0; reserved < FRAGMENT_TX_MAX_PACKETS
                && i < buffer->number; i++ )
         switch (buffer->list[i].state) {
         case FRAGMENT_ASSIGNED: case FRAGMENT_FINISHED:
            break;
         default:
	    reserved++;
         }

   // Try to send as many as FRAGMENT_TX_MAX_PACKETS concurrently
   for ( i = 0; buffer->sending < FRAGMENT_TX_MAX_PACKETS
             && i < buffer->number; i++ ) {
      // Acquire memory for TX fragment
      if ( buffer->in_use == FRAGMENT_TX        // outgoing
        && reserved < FRAGMENT_TX_MAX_PACKETS   // memory in use
        && buffer->list[i].state == FRAGMENT_ASSIGNED ) {
         error = fragment_reservePkt(buffer, i);
	 if ( error == E_SUCCESS )
            reserved++;
      }
      // Try to send a FW or TX fragment
      if ( buffer->list[i].state == FRAGMENT_RESERVED ) {
         pkt = buffer->list[i].fragment;
         buffer->list[i].state = FRAGMENT_SENDING;
         ENABLE_INTERRUPTS();
         error = sixtop_send(pkt);
         DISABLE_INTERRUPTS();
         if ( error == E_SUCCESS ) {
	    buffer->sending++;
	 } else {
            // Disable packet as a fragment cannot be sent
            ENABLE_INTERRUPTS();
	    fragment_finishPkt(buffer, &(buffer->list[i]), error);
	    return;
	 }
      }
   }

   if ( buffer && buffer->in_use == FRAGMENT_TX
     && reserved < FRAGMENT_TX_MAX_PACKETS
     && 0 < buffer->number - buffer->sent )
      if ( buffer->sending == 0 ) {
         pkt = buffer->msg;
         fragment_resetBuffer(buffer);
         ENABLE_INTERRUPTS();
         // sendDone will not call this function again and
         // it will not be able to send the message
	 // To Do: implement an scheduler task to know when an
	 // OpenQueue entry is  free, in order to acquiere it.
         openserial_printError(COMPONENT_FRAGMENT, ERR_NO_FREE_PACKET_BUFFER,
                               (errorparameter_t)0,
                               (errorparameter_t)0);
         iphc_sendDone(pkt, E_FAIL);
	 return;
      }
      // else: Function will be called again from sendDone

   ENABLE_INTERRUPTS();  
}

void fragment_setSize(OpenQueueEntry_t* msg, uint16_t size) {
   packetfunctions_htons(size, msg->payload);
}

void fragment_setTag(OpenQueueEntry_t* msg, uint16_t tag) {
   packetfunctions_htons(tag, msg->payload);
}

void fragment_setOffset(OpenQueueEntry_t* msg, uint8_t offset) {
   msg->payload[0] = offset;
}

uint16_t fragment_getSize(OpenQueueEntry_t* msg) {
   return packetfunctions_ntohs(msg->payload);
}

uint16_t fragment_getTag(OpenQueueEntry_t* msg) {
   return packetfunctions_ntohs((msg->payload) + 2 * sizeof(uint8_t));
}

uint8_t fragment_getOffset(OpenQueueEntry_t* msg) {
   return *((msg->payload) + 4 * sizeof(uint8_t));
}

/**
\brief Request a new (free) fragment buffer.

\returns A pointer to the buffer entry when it could be allocated,
	 or NULL when it could not be allocated (queue full).
*/
FragmentQueueEntry_t* fragment_getFreeBuffer(void) {
   uint8_t   i;

   for (i=0;i<FRAGQLENGTH;i++)
      if (fragmentqueue_vars.queue[i].in_use==FRAGMENT_NONE) {
         fragmentqueue_vars.queue[i].in_use = FRAGMENT_RESERVED;
	 return &(fragmentqueue_vars.queue[i]);
      }

   openserial_printError(COMPONENT_FRAGMENT,ERR_NO_FREE_FRAGMENT_BUFFER,
                         (errorparameter_t)0,
			 (errorparameter_t)0);

   return NULL;
}

/**
\brief Search for a previously-allocated fragment buffer, used for
       a packet, or allocate it.

\note It is used for both (in and out) messages, It assigns addresses
      values, as well as size and tag on incoming messages.

\param msg A pointer to a LowPAN fragmented packet.
\param in  Is it an incoming message?

\returns A pointer to the queue entry when it is found, a new 
	 packet if not or NULL, if not available.
*/
FragmentQueueEntry_t* fragment_searchBuffer(OpenQueueEntry_t* msg, bool in) {
   uint8_t               i;
   uint16_t              tag;
   uint16_t              size;
   open_addr_t*          src;
   open_addr_t*          dst;
   FragmentQueueEntry_t* buffer;
   INTERRUPT_DECLARATION();

   // Adreesses are only based on IEEE 802.15.4 as
   // MESH is not implemented
   src = in ? &(msg->l2_nextORpreviousHop)  : idmanager_getMyID(ADDR_64B);
   dst = in ? idmanager_getMyID(ADDR_64B) : &(msg->l2_nextORpreviousHop);

   DISABLE_INTERRUPTS();
   if ( in ) {
      size = fragment_getSize(msg);
      tag  = fragment_getTag(msg);
      // search for actual fragment buffer
      for (i=0;i<FRAGQLENGTH;i++)
         if ((fragmentqueue_vars.queue[i].in_use == FRAGMENT_RX
           || fragmentqueue_vars.queue[i].in_use == FRAGMENT_FW)
         && size==fragmentqueue_vars.queue[i].datagram_size
         && tag ==fragmentqueue_vars.queue[i].datagram_tag
         && packetfunctions_sameAddress(src, &fragmentqueue_vars.queue[i].src)
         && packetfunctions_sameAddress(dst, &fragmentqueue_vars.queue[i].dst)) {

            ENABLE_INTERRUPTS();
            return &fragmentqueue_vars.queue[i];
      }
   }

   // not found: get a new one
   buffer = fragment_getFreeBuffer();
   if ( buffer!=NULL ) {
      if ( in ) {
         buffer->datagram_size = size;
         buffer->datagram_tag  = tag;
      }
      memcpy(&(buffer->dst), dst, sizeof(open_addr_t));
      memcpy(&(buffer->src), src, sizeof(open_addr_t));
      buffer->in_use = in ? FRAGMENT_RX : FRAGMENT_TX;
      ENABLE_INTERRUPTS();
      return buffer;
   }

   ENABLE_INTERRUPTS();
   return NULL;
}

/**
\brief Free a previously-allocated fragment buffer.

\param buffer A pointer to the previously-allocated packet buffer.

\returns E_SUCCESS when the freeing was succeful.
\returns E_FAIL when the specified buffer NULL or was not used.
*/
owerror_t fragment_freeBuffer(FragmentQueueEntry_t* buffer) {
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   if ( buffer == NULL || buffer->in_use == FRAGMENT_NONE ) {
      ENABLE_INTERRUPTS();
      openserial_printError(COMPONENT_FRAGMENT,ERR_FREEING_ERROR,
                            (errorparameter_t)0,
			    (errorparameter_t)0);
      return E_FAIL;
   }
   fragment_resetBuffer(buffer);

   ENABLE_INTERRUPTS();
   return E_SUCCESS;
}

/**
\brief Discard actual message
*/
owerror_t fragment_restartBuffer(FragmentQueueEntry_t* buffer) {
   uint8_t i;
   INTERRUPT_DECLARATION();

   DISABLE_INTERRUPTS();
   if ( buffer == NULL || buffer->in_use == FRAGMENT_NONE ) {
      ENABLE_INTERRUPTS();
      openserial_printError(COMPONENT_FRAGMENT,ERR_FREEING_ERROR,
                            (errorparameter_t)0,
			    (errorparameter_t)0);
      return E_FAIL;
   }
   fragment_disableTimer(buffer);
   for ( i = 0; i < buffer->number; i++ ) {
      if ( buffer->list[i].fragment ) {
         if ( buffer->list[i].fragment == buffer->msg )
            buffer->msg = NULL;
         openqueue_freePacketBuffer_atomic(buffer->list[i].fragment);
         buffer->list[i].fragment = NULL;
      }
      buffer->list[i].state = FRAGMENT_NONE;
   }
   if ( buffer->msg ) {
      buffer->msg->payload += buffer->msg->length;
      buffer->msg->length   = 0;
   }
   buffer->number           = 0;
   buffer->sending          = 0;
   buffer->processed        = 0;
   buffer->action           = FRAGMENT_ACTION_NONE;
   ENABLE_INTERRUPTS();
   return E_SUCCESS;
}

/**
\brief Initializes buffer contents in atomic context
*/
void fragment_resetBuffer(FragmentQueueEntry_t* buffer) {
   uint8_t i;

   fragment_disableTimer(buffer);
   buffer->msg           = NULL;
   buffer->action        = FRAGMENT_ACTION_NONE;
   buffer->datagram_size = 0;
   buffer->number        = 0;
   buffer->processed     = 0;
   for ( i = 0; i < FRAGMENT_MAX_FRAGMENTS; i++ ) {
       buffer->list[i].state = FRAGMENT_NONE;
       buffer->list[i].fragment = NULL;
   }

   buffer->in_use = FRAGMENT_NONE;
}

#define fragment_finish_fragment(B,f) \
   openqueue_freePacketBuffer_atomic(B->list[f].fragment); \
   B->list[f].fragment = NULL; B->processed++; \
   B->list[f].state = FRAGMENT_FINISHED

inline void fragment_terminate(FragmentQueueEntry_t* buffer) {
   uint8_t i;
   INTERRUPT_DECLARATION();

   DISABLE_INTERRUPTS();
   for ( i = 0; i < buffer->number; i++ )
      if ( buffer->list[i].state == FRAGMENT_RECEIVED
        || buffer->list[i].state == FRAGMENT_PROCESSED 
	|| buffer->list[i].state == FRAGMENT_SENDING )
         fragment_finish_fragment(buffer, i);
   if ( buffer->msg )
      openqueue_freePacketBuffer_atomic(buffer->msg);
   ENABLE_INTERRUPTS();
}

void fragment_disableTimer(FragmentQueueEntry_t* buffer) {
   if (buffer!=NULL && buffer->timerId!=TOO_MANY_TIMERS_ERROR) {
      opentimers_stop(buffer->timerId); 
      buffer->timerId=TOO_MANY_TIMERS_ERROR;
   }
}

void fragment_timeout_timer_cb(opentimer_id_t id) {
   uint8_t i;
   INTERRUPT_DECLARATION();

   openserial_printError(COMPONENT_FRAGMENT, ERR_EXPIRED_TIMER,
                          (errorparameter_t)0,
			  (errorparameter_t)0);
   DISABLE_INTERRUPTS();
   for (i=0;i<FRAGQLENGTH;i++)
      if (fragmentqueue_vars.queue[i].timerId==id) {
         fragmentqueue_vars.queue[i].timerId=TOO_MANY_TIMERS_ERROR;
         ENABLE_INTERRUPTS();
         fragment_terminate(&(fragmentqueue_vars.queue[i]));
	 if (fragmentqueue_vars.queue[i].in_use == FRAGMENT_RX)
            fragment_freeBuffer(&(fragmentqueue_vars.queue[i]));
	 else if (fragmentqueue_vars.queue[i].in_use == FRAGMENT_FW)
            fragment_finishPkt(&(fragmentqueue_vars.queue[i]), NULL, E_FAIL);

	 return;
      }
   ENABLE_INTERRUPTS();
}

void fragment_activateTimer(FragmentQueueEntry_t* buffer) {
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   if (buffer->timerId==TOO_MANY_TIMERS_ERROR)
      buffer->timerId = opentimers_start(FRAGMENT_TIMEOUT_MS,
                     TIMER_ONESHOT, TIME_MS, fragment_timeout_timer_cb);
   // I am not checking TOO_MANY_TIMERS_ERROR
   ENABLE_INTERRUPTS();
}

uint8_t fragment_bufferIndex(FragmentQueueEntry_t* buffer) {
   uint8_t i;

   for (i=0;i<FRAGQLENGTH;i++)
      if ( buffer == &(fragmentqueue_vars.queue[i]) )
         return i;

   // Never must be reached
   return -1;
}

// action functions
void fragment_action(FragmentQueueEntry_t* buffer) {
   switch (buffer->action) {
      case FRAGMENT_ACTION_NONE:
         break;
      case FRAGMENT_ACTION_CANCEL:
         fragment_cancel(buffer);
         break;
      case FRAGMENT_ACTION_ASSEMBLE:
         fragment_assemble(buffer);
         break;
      case FRAGMENT_ACTION_FORWARD:
         fragment_forward(buffer);
         break;
      case FRAGMENT_ACTION_OPENBRIDGE:
         fragment_openbridge(buffer);
   }
}

void fragment_cancel(FragmentQueueEntry_t* buffer) {
   uint8_t  i;
   uint16_t received;
   INTERRUPT_DECLARATION();

   DISABLE_INTERRUPTS();
   if ( buffer->processed == 0 ) { // First entry
      for (i=0; i<buffer->number; i++)
         if ( buffer->list[i].fragment == buffer->msg ) { // FRAG1
            buffer->processed++;
            buffer->list[i].state = FRAGMENT_FINISHED;
	 }
   } 
   for (i=0; i<buffer->number; i++) {
      if (buffer->list[i].state == FRAGMENT_RECEIVED)
         fragment_finish_fragment(buffer, i);
      if (buffer->list[i].state == FRAGMENT_FINISHED)
         received += buffer->list[i].fragment_size;
   }
   ENABLE_INTERRUPTS();
   if (received == buffer->datagram_size) {
      openqueue_freePacketBuffer(buffer->msg);
      fragment_freeBuffer(buffer);
   }
}

void fragment_assemble(FragmentQueueEntry_t* buffer) {
   uint8_t           i;
   uint16_t          received;
   OpenQueueEntry_t* msg;
   INTERRUPT_DECLARATION();

   DISABLE_INTERRUPTS();
   if ( buffer->processed == 0 ) { // First entry
      for (i=0; i<buffer->number; i++)
         if ( buffer->list[i].fragment == buffer->msg ) { // FRAG1
            // determine actual offset and msg size
	    buffer->offset = buffer->list[i].fragment_size
                           - buffer->msg->length;
            received = buffer->datagram_size - buffer->offset;
            buffer->processed++;
            buffer->list[i].state = FRAGMENT_FINISHED;
	 }
      if ( received >= 125 ) { // ask for a large packet
         ENABLE_INTERRUPTS();
         if ( openqueue_toBigPacket(buffer->msg, received) == NULL ) {
            openserial_printError(COMPONENT_FRAGMENT,
                                  ERR_NO_FREE_PACKET_BUFFER,
                                  (errorparameter_t)0,
                                  (errorparameter_t)0);
            buffer->action = FRAGMENT_ACTION_CANCEL;
	    return;
         }
      } else { // re-use packet
         memmove(&(buffer->msg->packet[127]) - received,
                   buffer->msg->payload, received);
	 buffer->msg->payload = &(buffer->msg->packet[127]) - received;
	 buffer->sending = 0;
      }
      buffer->msg->length = received;
   }

   received = 0;
   for (i=0; i<buffer->number; i++) {
      if (buffer->list[i].state == FRAGMENT_RECEIVED) {
         // Assemble
	 memcpy(buffer->msg->payload - buffer->offset
                      +(buffer->list[i].fragment_offset<<3),
                buffer->list[i].fragment->payload,
                buffer->list[i].fragment_size);
	 buffer->list[i].state = FRAGMENT_PROCESSED;
      }
      if (buffer->list[i].state == FRAGMENT_PROCESSED)
         fragment_finish_fragment(buffer, i);
      if (buffer->list[i].state == FRAGMENT_FINISHED)
         received += buffer->list[i].fragment_size;
   }
   ENABLE_INTERRUPTS();
   if (received == buffer->datagram_size) {
      msg = buffer->msg;
      fragment_freeBuffer(buffer);
      forwarding_toUpperLayer(msg);
   }
}

void fragment_forward(FragmentQueueEntry_t* buffer) {
   uint8_t i;
   uint8_t offset;
   OpenQueueEntry_t* msg;
   INTERRUPT_DECLARATION();

   DISABLE_INTERRUPTS();
   if ( buffer->in_use == FRAGMENT_RX ) { // First entry
      buffer->in_use = FRAGMENT_FW;
      for (i=0; i<buffer->number; i++)
         if ( buffer->list[i].fragment == buffer->msg ) { // FRAG1
            msg = buffer->msg;
            // Take ownership to process it when sendDone
	    msg->owner   = COMPONENT_FRAGMENT;
	    msg->creator = COMPONENT_FRAGMENT;
	    buffer->new_size = buffer->datagram_size
                             - buffer->list[i].fragment_size
                             + msg->length;
            buffer->new_tag  = fragment_getNewTag();
            packetfunctions_reserveHeaderSize(msg, 2 * sizeof(uint8_t));
            fragment_setTag(msg, buffer->new_tag);
            packetfunctions_reserveHeaderSize(msg, 2 * sizeof(uint8_t));
            fragment_setSize(msg, buffer->new_size);
            msg->payload[0] |= (LOWPAN_DISPATCH_FRAG1 << LOWPAN_DISPATCH);
            buffer->list[i].state = FRAGMENT_RESERVED;
	 }
   } 
   for (i=0; i<buffer->number; i++) {
      if (buffer->list[i].state == FRAGMENT_RECEIVED) {
         msg = buffer->list[i].fragment;
	 msg->owner   = COMPONENT_FRAGMENT;
	 msg->creator = COMPONENT_FRAGMENT;
	 offset = buffer->datagram_size - buffer->new_size;
	 offset = buffer->list[i].fragment_offset - (offset>>3);
         packetfunctions_reserveHeaderSize(msg, sizeof(uint8_t));
         fragment_setOffset(msg, offset);
         packetfunctions_reserveHeaderSize(msg, 2 * sizeof(uint8_t));
         fragment_setTag(msg, buffer->new_tag);
         packetfunctions_reserveHeaderSize(msg, 2 * sizeof(uint8_t));
         fragment_setSize(msg, buffer->new_size);
         msg->payload[0] |= (LOWPAN_DISPATCH_FRAGN << LOWPAN_DISPATCH);
	 buffer->list[i].state = FRAGMENT_RESERVED;
         // Add L2 information
         memcpy(&(msg->l2_nextORpreviousHop),
                &(buffer->msg->l2_nextORpreviousHop), sizeof(open_addr_t));
      }
   }
   ENABLE_INTERRUPTS();

   fragment_tryToSend(buffer);
}

void fragment_openbridge(FragmentQueueEntry_t* buffer) {
   uint8_t           i;
   uint16_t          received;
   OpenQueueEntry_t* msg;
   INTERRUPT_DECLARATION();

   if ( buffer->processed == 0 ) { // First entry
      DISABLE_INTERRUPTS();
      for (i=0; i<buffer->number; i++)
         if ( buffer->list[i].fragment == buffer->msg ) { // FRAG1
            // determine actual offset and msg size
	    buffer->offset = buffer->list[i].fragment_size
                           - buffer->msg->length;
            received = buffer->datagram_size - buffer->offset;
            buffer->processed++;
            buffer->list[i].state = FRAGMENT_FINISHED;
	 }
      if ( received < 125 ) { // re-use
         memmove(&(buffer->msg->packet[127]) - received,
                   buffer->msg->payload, received);
	 buffer->msg->payload = &(buffer->msg->packet[127]) - received;
      } else {
	 buffer->action = FRAGMENT_ACTION_CANCEL;
         ENABLE_INTERRUPTS();
	 return;
      }
      buffer->msg->length = received;
   }

   received = 0;
   for (i=0; i<buffer->number; i++) {
      if (buffer->list[i].state == FRAGMENT_RECEIVED) {
         // Assemble
	 memcpy(buffer->msg->payload - buffer->offset
                      +(buffer->list[i].fragment_offset<<3),
                buffer->list[i].fragment->payload,
                buffer->list[i].fragment_size);
	 buffer->list[i].state = FRAGMENT_PROCESSED;
      }
      if (buffer->list[i].state == FRAGMENT_PROCESSED)
         fragment_finish_fragment(buffer, i);
      if (buffer->list[i].state == FRAGMENT_FINISHED)
         received += buffer->list[i].fragment_size;
   }
   ENABLE_INTERRUPTS();
   if (received == buffer->datagram_size) {
      msg = buffer->msg;
      fragment_freeBuffer(buffer);
      openbridge_receive(msg);
   }
}

// Determines L2 header size
uint8_t fragment_askL2HeaderSize(OpenQueueEntry_t* msg) {
   uint8_t hsize;

   uint8_t askAddressSize(open_addr_t* addr) {
      switch (addr->type) {
         case ADDR_16B: case ADDR_PANID:
            return 2;
	 case ADDR_64B: case ADDR_PREFIX:
	    return 8;
	 case ADDR_128B:
	    return 16;
	 default:
	    return 0;
      }
   }

   // Begin
   hsize = askAddressSize(idmanager_getMyID(ADDR_64B));
   if (packetfunctions_isBroadcastMulticast(&(msg->l2_nextORpreviousHop)))
      hsize += 2; //broadcast address is always 16-bit
   else
      switch (msg->l2_nextORpreviousHop.type) {
         case ADDR_16B: case ADDR_64B:
            hsize += askAddressSize(&(msg->l2_nextORpreviousHop));
	    break;
      }

   hsize += askAddressSize(idmanager_getMyID(ADDR_PANID));
   hsize += 1; //dsn
   hsize += 2; //fcf

   return hsize;
}

