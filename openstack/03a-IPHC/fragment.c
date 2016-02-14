#include "opendefs.h"
#include "opentimers.h"
#include "openrandom.h"
#include "packetfunctions.h"
#include "sixtop.h"
#include "iphc.h"
#include "fragment.h"
#include "forwarding.h"
#include "openbridge.h"
#include "openserial.h"

//=========================== variables =======================================

fragmentqueue_vars_t fragmentqueue_vars;

//=========================== prototypes ======================================

uint16_t fragment_getNewTag(void);

// package management
owerror_t fragment_startSend(FragmentQueueEntry_t* buffer);
void fragment_tryToSend(FragmentQueueEntry_t* buffer);
void fragment_finishSend(FragmentQueueEntry_t* buffer, owerror_t error);

// to implement on lower layers?
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
void fragment_cancelToBridge(FragmentQueueEntry_t* buffer);

bool fragment_completeRX(FragmentQueueEntry_t* buffer, FragmentState state);
// timer
void fragment_disableTimer(FragmentQueueEntry_t* buffer);
void fragment_activateTimer(FragmentQueueEntry_t* buffer);
void fragment_timeout_timer_cb(opentimer_id_t id);

// action functions
void fragment_action(FragmentQueueEntry_t* buffer, uint8_t frag);
void fragment_doCancel(FragmentQueueEntry_t* buffer);
void fragment_cancel(FragmentQueueEntry_t* buffer, uint8_t frag);
void fragment_doAssemble(FragmentQueueEntry_t* buffer, FragmentAction action);
void fragment_assemble(FragmentQueueEntry_t* buffer, uint8_t frag);
void fragment_openbridge(FragmentQueueEntry_t* buffer);

owerror_t openqueue_freePacketBuffer_atomic(OpenQueueEntry_t* pkt);

uint8_t fragment_bufferCountFree(void);
//=========================== public ==========================================

/**
\brief Initialize this module.
*/
void fragment_init() {
   uint8_t i;

   for (i=0;i<FRAGQLENGTH;i++) {
      fragmentqueue_vars.queue[i].in_use  = FRAGMENT_NONE;
      fragmentqueue_vars.queue[i].timerId = FRAGMENT_NOTIMER;
      fragment_resetBuffer(&fragmentqueue_vars.queue[i]);
   }
   fragmentqueue_vars.tag = openrandom_get16b();
}

/**
\brief Adds fragmentation headers.
       Fragments message in multiple packets, if neccessary.

\note  When fragmentation headers must be added, it actually do not send
       fragments: it just checks if message must be fragmented.

\param msg A pointer to the packet to send.

\returns E_SUCCESS when sending could be started.
\returns E_FAIL when msg cannot be send.
*/
owerror_t fragment_prependHeader(OpenQueueEntry_t* msg) {
   uint8_t               l2_hsize;
   uint8_t               iphc_hsize;
   uint8_t               max_fragment;
   uint8_t               actual_frag_size;
   FragmentQueueEntry_t* buffer;

   msg->owner   = COMPONENT_FRAGMENT;
   l2_hsize     = fragment_askL2HeaderSize(msg);
   max_fragment = FRAGMENT_DATA_UTIL - l2_hsize;
   if ( msg->length <= max_fragment && msg->big == NULL )
      return  sixtop_send(msg);

   msg->owner       = COMPONENT_FRAGMENT;
   iphc_hsize       = msg->length - msg->l4_length;
   max_fragment    -= FRAGMENT_FRAG1_HL;
   actual_frag_size = max_fragment & 0xF8; // =/8*8;
   // RFC 4944 page 11: "The first link fragment SHALL contain
   // the first fragment header.
   if ( actual_frag_size < iphc_hsize ) {
      openserial_printError(COMPONENT_FRAGMENT, ERR_6LOWPAN_UNSUPPORTED,
                            (errorparameter_t)0,
			    (errorparameter_t)0);
      return E_FAIL;
   }

   // Allocate space for fragmentation
   buffer = fragment_searchBuffer(msg, FALSE);
   if ( buffer == NULL ) {
      openserial_printError(COMPONENT_FRAGMENT, ERR_NO_FREE_FRAGMENT_BUFFER,
                             (errorparameter_t)0,
			     (errorparameter_t)0);
      return E_FAIL;
   }

   buffer->msg = msg;
   return fragment_startSend(buffer);
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
bool fragment_retrieveHeader(OpenQueueEntry_t* msg) {
   uint8_t                 i;
   uint8_t                 dispatch;
   uint8_t                 offset;
   FragmentQueueEntry_t*   buffer;
   FragmentOffsetEntry_t   tempFO;
   bool                    fragn;
   uint16_t                size;
   INTERRUPT_DECLARATION();

   msg->owner = COMPONENT_FRAGMENT;
   fragn = TRUE;
   dispatch = (*((uint8_t*)(msg->payload)) >> IPHC_FRAGMENT) & 0x1F; // 5b

   switch (dispatch) {
      case IPHC_DISPATCH_FRAG1:
         fragn = FALSE;
      case IPHC_DISPATCH_FRAGN:
	 break;
      default: // It is not a fragment
	 return FALSE;
   }

   msg->payload[0] &= 0x07; // Filter dispatch
   buffer = fragment_searchBuffer(msg, TRUE);
   if ( buffer == NULL ) {
      openserial_printError(COMPONENT_FRAGMENT, ERR_NO_FREE_FRAGMENT_BUFFER,
                             (errorparameter_t)0,
			     (errorparameter_t)0);
      // abort
      openqueue_freePacketBuffer(msg);
      return TRUE;
   }

   size   = msg->length - (fragn ? FRAGMENT_FRAGN_HL : FRAGMENT_FRAG1_HL);
   offset = fragn ? fragment_getOffset(msg) : 0;
   // data size must be a multiple of 8 or final fragment
   if(!(   (size%8==0)
        || (offset*8+size==buffer->datagram_size))) {
      openserial_printError(COMPONENT_FRAGMENT, ERR_INPUTBUFFER_LENGTH,
                              (errorparameter_t)msg->length,
			      (errorparameter_t)0);
      // abort
      openqueue_freePacketBuffer(msg);
      if ( buffer->number == 0 )
         fragment_freeBuffer(buffer);
      return TRUE;
   }

/*
 * Is it possible? A message fragmented on just one fragment.
 *
   if (!fragn && size == buffer->datagram_size) {
      fragment_freeBuffer(buffer);
      packetfunctions_tossHeader(msg, 4);
      return FALSE;
   }
*/

   tempFO.fragment_size   = size;
   tempFO.fragment_offset = offset;
   DISABLE_INTERRUPTS();
   for (i=0;i<buffer->number;i++) {
      if (BUFFER_EQUAL(buffer->other.list[i],tempFO)) {
         ENABLE_INTERRUPTS();
         // Duplicated fragment
	 openqueue_freePacketBuffer(msg);
	 return TRUE;
      } else if (BUFFER_OVERLAP(buffer->other.list[i],tempFO)) {
         ENABLE_INTERRUPTS();
         // Current fragment overlaps previous one
	 // RFC 4944: If a link fragment that overlaps another fragment
	 // is received ... buffer SHALL be discarded.
	 if ( buffer->action == FRAGMENT_ACTION_OPENBRIDGE )
            fragment_cancelToBridge(buffer);
	 fragment_restartBuffer(buffer);
         // log error
         openserial_printError(COMPONENT_FRAGMENT, ERR_INPUTBUFFER_OVERLAPS,
                                (errorparameter_t)0,
				(errorparameter_t)0);
         DISABLE_INTERRUPTS();
      }
   }
   buffer->other.list[buffer->number].fragment_offset = offset;
   buffer->other.list[buffer->number].fragment_size   = size;
   buffer->other.list[buffer->number].fragment        = msg;
   buffer->other.list[buffer->number].state = FRAGMENT_RECEIVED;
   buffer->number++;
   dispatch = buffer->number;
   ENABLE_INTERRUPTS();
   packetfunctions_tossHeader(msg, fragn ?FRAGMENT_FRAGN_HL :FRAGMENT_FRAG1_HL);

   // First received fragment: activate timer.
   if ( dispatch == 1 )
      fragment_activateTimer(buffer);
   // last received fragment = completed message: stop timer
   else if ( fragment_completeRX(buffer, FRAGMENT_RECEIVED) )
      fragment_disableTimer(buffer);

   // if message cannot be allocated because it is too large it is
   // marked for cancellation in order to track following fragments
   if ( !fragn && buffer->datagram_size > LARGE_PACKET_SIZE ) {
      buffer->action = FRAGMENT_ACTION_CANCEL;
      buffer->msg = NULL;
      fragn = TRUE;
      openserial_printError(COMPONENT_FRAGMENT, ERR_INPUTBUFFER_LENGTH,
                                (errorparameter_t)buffer->datagram_size,
				(errorparameter_t)1);
   }

   // Initial fragment
   if (! fragn) {
      buffer->msg = msg;
      return FALSE;
   // Initial fragment processed: process this one
   } else
      fragment_action(buffer, dispatch-1);

   return TRUE;
}

void fragment_sendDone(OpenQueueEntry_t *msg, owerror_t error) {
   FragmentQueueEntry_t* buffer;

   msg->owner = COMPONENT_FRAGMENT;

   if ( (buffer = fragment_searchBufferFromMsg(msg))
      && buffer->in_use != FRAGMENT_NONE ) {
      if ( error == E_SUCCESS )
         fragment_tryToSend(buffer);
      else
         fragment_finishSend(buffer, error);
      return;
   }

   openserial_printError(COMPONENT_FRAGMENT,ERR_UNEXPECTED_SENDDONE,
                          (errorparameter_t)0,
                          (errorparameter_t)0);
   openqueue_freePacketBuffer(msg);
   if ( buffer && buffer->in_use != FRAGMENT_NONE )
      fragment_freeBuffer(buffer);
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
   if ( buffer->action == FRAGMENT_ACTION_OPENBRIDGE )
      fragment_cancelToBridge(buffer);
   buffer->action = action;

   switch (action) {
      case FRAGMENT_ACTION_NONE:
         break;
      case FRAGMENT_ACTION_CANCEL:
         fragment_doCancel(buffer);
         break;
      case FRAGMENT_ACTION_ASSEMBLE:
      case FRAGMENT_ACTION_FORWARD:
         fragment_doAssemble(buffer, action);
         break;
      case FRAGMENT_ACTION_OPENBRIDGE:
         fragment_openbridge(buffer);
   }
}

/**
\brief Check if this message from OpenBridge is a fragment and send a
       notification to OpenVisualizer.
*/
void fragment_checkOpenBridge(OpenQueueEntry_t *msg, owerror_t error) {
   uint8_t  dispatch;
   uint8_t  chain[5];
   uint16_t tag;

   // discard L2 info
   packetfunctions_tossHeader(msg, fragment_askL2HeaderSize(msg));
   // check
   dispatch = (msg->payload[0] >> IPHC_FRAGMENT) & 0x1F;
   if ( dispatch != IPHC_DISPATCH_FRAG1
     && dispatch != IPHC_DISPATCH_FRAGN )
      return;

   // send notification
   tag      = fragment_getTag(msg);
   chain[0] = error == E_SUCCESS
              ? FRAGMENT_MOTE2PC_SENDDONE
              : FRAGMENT_MOTE2PC_FAIL;
   chain[1] = FRAGMENT_MOTE2PC_TOMESH;
   chain[2] = (tag & 0xFF00) >> 8;
   chain[3] = tag & 0x00FF;

   openserial_printBridge(&(chain[0]), 4);
}

/**
\brief RFC 4944
       Upon detection of a IEEE 802.15.4 Disassociation event, fragment
       recipients MUST discard all link fragments of all partially
       reassembled payload datagrams, and fragment senders MUST discard all
       not yet transmitted link fragments of all partially transmitted
       payload (e.g., IPv6) datagrams.
*/
void fragment_deleteNeighbor(open_addr_t* neighbor)
{
   uint8_t i;

   for (i=0;i<FRAGQLENGTH;i++)
      if ( fragmentqueue_vars.queue[i].in_use != FRAGMENT_NONE 
       && (packetfunctions_sameAddress(neighbor,
		                       &fragmentqueue_vars.queue[i].src)
        || packetfunctions_sameAddress(neighbor,
                                       &fragmentqueue_vars.queue[i].dst)) ) {
         fragment_assignAction(&(fragmentqueue_vars.queue[i]),
			       FRAGMENT_ACTION_CANCEL);
      }
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

/**
\brief  Checks if buffer is complete

\note   is intended that state could be RECEIVED or FINISHED to check
        if buffer is received completely or it is processed completely

\param  buffer the framentation buffer to check
\param  state  fragments state

\return TRUE   when complete
\return FALSE  when not complete
*/
bool fragment_completeRX(FragmentQueueEntry_t* buffer, FragmentState state) {
   uint8_t  i;
   uint16_t received;
   bool     complete;
   INTERRUPT_DECLARATION();

   received = 0;
   DISABLE_INTERRUPTS();
   for ( i = 0; i < buffer->number; i++ )
      if ( state == buffer->other.list[i].state )
         received += buffer->other.list[i].fragment_size;

   complete = received == buffer->datagram_size;
   ENABLE_INTERRUPTS();
   return complete;
}

/**
\brief Create the proper environment to start sending frames.

\note  Message can get this point in the form of large packet (assumed it must
       be fragmented) or of normal packet but do not fit into a single frame.
       If message does not need to be fragmented if calls sixtop_send, as usual.
       If it must be fragmented, initializes fragmentation data and starts
       fragmentation sending process yielding control to fragment_tryToSend.

\param buffer A fragmentation buffer containing the packet to send.

\returns E_SUCCESS when sending could be started.
\returns E_FAIL when msg cannot be send.
*/
owerror_t fragment_startSend(FragmentQueueEntry_t* buffer) {
   uint8_t               l2_hsize;
   uint8_t               max_fragment;
   uint8_t               actual_frag_size;

   buffer->creator       = buffer->msg->creator;
   buffer->msg->creator  = COMPONENT_FRAGMENT;
   buffer->datagram_size = buffer->msg->length;
   buffer->datagram_tag  = fragment_getNewTag();

   l2_hsize     = fragment_askL2HeaderSize(buffer->msg);
   max_fragment = FRAGMENT_DATA_UTIL - l2_hsize - FRAGMENT_FRAG1_HL;
   actual_frag_size = max_fragment & 0xF8;
   // if not big packet but fragmentation is needed
   if ( buffer->msg->big == NULL ) {
      buffer->msg->big = &(buffer->other.data.excess[0]);
      memcpy(buffer->msg->big, buffer->msg->payload, buffer->msg->length);
      buffer->other.data.payload = buffer->msg->big;
   } else
      buffer->other.data.payload = buffer->msg->payload;
   buffer->other.data.max_fragment_size = (max_fragment + FRAGMENT_FRAG1_HL - FRAGMENT_FRAGN_HL) & 0xF8;
   buffer->other.data.size              = actual_frag_size;
   buffer->other.data.fragn             = FALSE;

   // Start sending the different frames
   fragment_tryToSend(buffer);

   return E_SUCCESS;
}

/**
\brief Generates a fragment and tries to send it
*/
void fragment_tryToSend(FragmentQueueEntry_t* buffer) {
   OpenQueueEntry_t* pkt;
   uint16_t          actual_sent;
   uint8_t           actual_frag_size;
   INTERRUPT_DECLARATION();

   DISABLE_INTERRUPTS();
   actual_sent = buffer->other.data.actual_sent;
   // check if message has been sent
   if ( actual_sent == buffer->datagram_size ) {
      ENABLE_INTERRUPTS();
      fragment_finishSend(buffer, E_SUCCESS);
      return;
   }

   // generate fragment
   actual_frag_size = buffer->other.data.size;
   pkt              = buffer->msg;
   ENABLE_INTERRUPTS();
   pkt->payload     = &(pkt->packet[FRAGMENT_DATA_INIT - actual_frag_size]);
   memcpy(pkt->payload,
          buffer->other.data.payload + actual_sent,
          actual_frag_size * sizeof(uint8_t));
   pkt->length      = actual_frag_size;
   DISABLE_INTERRUPTS();
   if ( buffer->other.data.fragn ) { // offset
      packetfunctions_reserveHeaderSize(pkt, sizeof(uint8_t));
      fragment_setOffset(pkt, actual_sent>>3);
   }
   // tag
   packetfunctions_reserveHeaderSize(pkt, 2 * sizeof(uint8_t));
   fragment_setTag(pkt, buffer->datagram_tag);
   // size
   packetfunctions_reserveHeaderSize(pkt, 2 * sizeof(uint8_t));
   fragment_setSize(pkt, buffer->datagram_size);
   if ( buffer->other.data.fragn )
      pkt->payload[0] |= (IPHC_DISPATCH_FRAGN << IPHC_FRAGMENT);
   else {
      pkt->payload[0] |= (IPHC_DISPATCH_FRAG1 << IPHC_FRAGMENT);
      buffer->other.data.fragn = TRUE;
   }

   actual_sent      = actual_sent + actual_frag_size;
   actual_frag_size = buffer->other.data.max_fragment_size;
   // last fragment?
   if ( actual_frag_size > buffer->datagram_size - actual_sent )
      actual_frag_size = buffer->datagram_size - actual_sent;
   buffer->other.data.actual_sent = actual_sent;
   buffer->other.data.size        = actual_frag_size;
   ENABLE_INTERRUPTS();

   if ( sixtop_send(pkt) == E_FAIL ) {
      fragment_finishSend(buffer, E_FAIL);
   }
}

void fragment_finishSend(FragmentQueueEntry_t* buffer, owerror_t error)
{
   OpenQueueEntry_t* pkt;
   INTERRUPT_DECLARATION();

   DISABLE_INTERRUPTS();
   pkt = buffer->msg;
   pkt->creator = buffer->creator;
   if ( pkt->big == &(buffer->other.data.excess[0]) )
      pkt->big = NULL;
   fragment_resetBuffer(buffer);
   ENABLE_INTERRUPTS();
   forwarding_sendDone(pkt, error);
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
   uint8_t   check;
   INTERRUPT_DECLARATION();

   for ( i = 0; i < FRAGQLENGTH; i++ )
      if ( fragmentqueue_vars.queue[i].in_use == FRAGMENT_NONE ) {
         fragmentqueue_vars.queue[i].in_use = FRAGMENT_RESERVED;
	 return &(fragmentqueue_vars.queue[i]);
      }

   openserial_printError(COMPONENT_FRAGMENT,ERR_NO_FREE_FRAGMENT_BUFFER,
                         (errorparameter_t)0,
			 (errorparameter_t)0);

   // Just happened once on heavy load but...
   // All fragment buffers are occupied with no available timers
   // Try to free possible expired ones
   for ( i = 0; i < FRAGQLENGTH; i++ )
      if ( (fragmentqueue_vars.queue[i].in_use == FRAGMENT_RX
         || fragmentqueue_vars.queue[i].in_use == FRAGMENT_FW)
        && fragmentqueue_vars.queue[i].timerId == TOO_MANY_TIMERS_ERROR ) {
         fragmentqueue_vars.queue[i].timerId = FRAGMENT_NOTIMER-i;
         ENABLE_INTERRUPTS();
	 fragment_timeout_timer_cb(FRAGMENT_NOTIMER-i);
         DISABLE_INTERRUPTS();
      }

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
   src = in ? &(msg->l2_nextORpreviousHop) : idmanager_getMyID(ADDR_64B);
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
                            (errorparameter_t)1,
			    (errorparameter_t)0);
      return E_FAIL;
   }
   fragment_resetBuffer(buffer);

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
   for ( i = 0; i < FRAGMENT_MAX_FRAGMENTS; i++ ) {
       buffer->other.list[i].state = FRAGMENT_NONE;
       buffer->other.list[i].fragment = NULL;
   }

   buffer->in_use = FRAGMENT_NONE;
}

/**
\brief Discard actual message (re-using buffer)
*/
owerror_t fragment_restartBuffer(FragmentQueueEntry_t* buffer) {
   uint8_t i;
   INTERRUPT_DECLARATION();

   DISABLE_INTERRUPTS();
   if ( buffer == NULL || buffer->in_use == FRAGMENT_NONE ) {
      ENABLE_INTERRUPTS();
      openserial_printError(COMPONENT_FRAGMENT,ERR_FREEING_ERROR,
                            (errorparameter_t)2,
			    (errorparameter_t)0);
      return E_FAIL;
   }
   fragment_disableTimer(buffer);
   for ( i = 0; i < buffer->number; i++ ) {
      if ( buffer->other.list[i].fragment ) {
         if ( buffer->other.list[i].fragment == buffer->msg )
            buffer->msg = NULL;
         openqueue_freePacketBuffer_atomic(buffer->other.list[i].fragment);
         buffer->other.list[i].fragment = NULL;
      }
      buffer->other.list[i].state = FRAGMENT_NONE;
   }
   buffer->number           = 0;
   buffer->action           = FRAGMENT_ACTION_NONE;
   ENABLE_INTERRUPTS();
   return E_SUCCESS;
}

void fragment_disableTimer(FragmentQueueEntry_t* buffer) {
   if (buffer!=NULL && buffer->timerId!=FRAGMENT_NOTIMER) {
      opentimers_stop(buffer->timerId); 
      buffer->timerId=FRAGMENT_NOTIMER;
   }
}

void fragment_timeout_timer_cb(opentimer_id_t id) {
   uint8_t               i;
   uint8_t               j;
   FragmentQueueEntry_t* buffer;
   OpenQueueEntry_t*     msg;
   INTERRUPT_DECLARATION();

   openserial_printError(COMPONENT_FRAGMENT, ERR_EXPIRED_TIMER,
                          (errorparameter_t)0,
			  (errorparameter_t)0);
   DISABLE_INTERRUPTS();
   for (i=0;i<FRAGQLENGTH;i++)
      if (fragmentqueue_vars.queue[i].timerId==id) {
	 buffer = &(fragmentqueue_vars.queue[i]);
         if ( buffer->action == FRAGMENT_ACTION_OPENBRIDGE ) {
	    buffer->action = FRAGMENT_ACTION_CANCEL;
            ENABLE_INTERRUPTS();
            fragment_cancelToBridge(buffer);
            DISABLE_INTERRUPTS();
	 }
         buffer->action = FRAGMENT_ACTION_CANCEL;
         for (j = 0; j < buffer->number
		  && buffer->other.list[j].state != FRAGMENT_FINISHED;
	      j++ ) {
            msg = buffer->other.list[j].fragment;
            if ( buffer->msg && buffer->msg == msg
                             && buffer->in_use != FRAGMENT_FW )
               msg = NULL;
	    buffer->other.list[j].state = FRAGMENT_FINISHED;
	    buffer->other.list[j].fragment = NULL;
	    if ( msg )
               openqueue_freePacketBuffer_atomic(msg);
	 }
	 msg = NULL;
         if ( buffer->in_use == FRAGMENT_FW )
            msg = buffer->msg;
         fragment_resetBuffer(buffer);
	 ENABLE_INTERRUPTS();
         // Notify error to upper layer when forwarding
	 if ( msg )
            forwarding_sendDone(msg, E_FAIL);
	 return;
      }
   ENABLE_INTERRUPTS();
}

void fragment_activateTimer(FragmentQueueEntry_t* buffer) {
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   if (buffer->timerId==FRAGMENT_NOTIMER)
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

void fragment_cancelToBridge(FragmentQueueEntry_t* buffer) {
   uint8_t chain[14];

   chain[0] = FRAGMENT_MOTE2PC_FAIL;
   chain[1] = FRAGMENT_MOTE2PC_FROMMESH;
   chain[2] = (buffer->datagram_tag & 0xFF00) >> 8;
   chain[3] = buffer->datagram_tag & 0x00FF;
   chain[4] = (buffer->datagram_size & 0x0700) >> 8;
   chain[5] = buffer->datagram_size & 0x00FF;
   memcpy(&(chain[6]),buffer->src.addr_64b,LENGTH_ADDR64b);

   openserial_printBridge(&(chain[0]), 14);
}

// action functions
void fragment_action(FragmentQueueEntry_t* buffer, uint8_t frag) {
   switch (buffer->action) {
      case FRAGMENT_ACTION_NONE:
         break;
      case FRAGMENT_ACTION_CANCEL:
         fragment_cancel(buffer, frag);
         break;
      case FRAGMENT_ACTION_ASSEMBLE:
      case FRAGMENT_ACTION_FORWARD:
         fragment_assemble(buffer, frag);
         break;
      case FRAGMENT_ACTION_OPENBRIDGE:
         fragment_openbridge(buffer);
   }
}

void fragment_doCancel(FragmentQueueEntry_t* buffer) {
   uint8_t  i;

   if ( buffer->in_use == FRAGMENT_TX ) {
      fragment_finishSend(buffer, E_FAIL);
      return;
   }

   for ( i = 0; i < buffer->number; i++ )
      fragment_cancel(buffer, i);
}

void fragment_cancel(FragmentQueueEntry_t* buffer, uint8_t frag) {
   OpenQueueEntry_t* msg;
   INTERRUPT_DECLARATION();

   DISABLE_INTERRUPTS();
   msg = buffer->other.list[frag].fragment;
   if ( msg == buffer->msg && buffer->in_use != FRAGMENT_FW ) // FRAG1
      msg = NULL;
   buffer->other.list[frag].state    = FRAGMENT_FINISHED;
   buffer->other.list[frag].fragment = NULL;
   ENABLE_INTERRUPTS();
   if ( msg )
      openqueue_freePacketBuffer(msg);

   if ( fragment_completeRX(buffer, FRAGMENT_FINISHED) ) {
      if ( buffer->in_use != FRAGMENT_FW )
         fragment_freeBuffer(buffer);
      else // Notify error to upper layer when forwarding
         fragment_finishSend(buffer, E_FAIL);
   }
}

void fragment_doAssemble(FragmentQueueEntry_t* buffer, FragmentAction action) {
   uint8_t  i;
   uint8_t  frag1;
   uint16_t received;
   INTERRUPT_DECLARATION();

   DISABLE_INTERRUPTS();
   if ( action == FRAGMENT_ACTION_FORWARD )
      buffer->in_use = FRAGMENT_FW;
   for ( i = buffer->number-1; i >= 0 &&
             buffer->other.list[i].fragment != buffer->msg; // FRAG1
         i--);
   frag1 = i;
   // determine actual offset and msg size
   buffer->offset = buffer->other.list[frag1].fragment_size
                  - buffer->msg->length;
   received = buffer->datagram_size - buffer->offset;

   if ( received >= FRAGMENT_DATA_UTIL ) { // ask for a large packet
      ENABLE_INTERRUPTS();
      if ( openqueue_toBigPacket(buffer->msg, received) == NULL ) {
         openserial_printError(COMPONENT_FRAGMENT,
                               ERR_NO_FREE_PACKET_BUFFER,
                               (errorparameter_t)0,
                               (errorparameter_t)0);
         fragment_assignAction(buffer, FRAGMENT_ACTION_CANCEL);
         return;
      }
      DISABLE_INTERRUPTS();
   } else { // re-use packet
      memmove(&(buffer->msg->packet[FRAME_DATA_PLOAD]) - received,
                buffer->msg->payload, buffer->msg->length);
      buffer->msg->payload = &(buffer->msg->packet[FRAME_DATA_PLOAD]) - received;
   }
   buffer->other.list[frag1].state = FRAGMENT_FINISHED;
   if ( action == FRAGMENT_ACTION_FORWARD )
      buffer->msg->l4_length = received - buffer->msg->length;
   buffer->msg->length = received;
   received = buffer->number;
   ENABLE_INTERRUPTS();
=======
   buffer->msg->length = received;
   if ( action == FRAGMENT_ACTION_FORWARD )
      buffer->msg->l4_length = received - buffer->other.list[frag1].fragment_size;
>>>>>>> 5dfb36458e95155a4a0fde0de0d3a1048bca76f1

   // Process pending fragments, if any
   if ( received > 1 )
      for ( i = 0; i < received; i++ )
         if ( i != frag1 )
            fragment_assemble(buffer, i);
}

void fragment_assemble(FragmentQueueEntry_t* buffer, uint8_t frag) {
   FragmentState     state;
   OpenQueueEntry_t* msg;
   INTERRUPT_DECLARATION();

// buffer->other.list[i].state == FRAGMENT_RECEIVED
   // Assemble
   DISABLE_INTERRUPTS();
   memcpy(buffer->msg->payload - buffer->offset
                               +(buffer->other.list[frag].fragment_offset<<3),
          buffer->other.list[frag].fragment->payload,
          buffer->other.list[frag].fragment_size);
// buffer->other.list[i].state = FRAGMENT_PROCESSED;
   msg = buffer->other.list[frag].fragment;
   buffer->other.list[frag].fragment = NULL;
   buffer->other.list[frag].state = FRAGMENT_FINISHED;
   ENABLE_INTERRUPTS();
   openqueue_freePacketBuffer(msg);

   if ( fragment_completeRX(buffer, FRAGMENT_FINISHED) ) {
      msg   = buffer->msg;
      state = buffer->in_use;
      if ( state == FRAGMENT_RX ) {
         fragment_freeBuffer(buffer);
         forwarding_toUpperLayer(msg);
      } else { // forwarding
         buffer->in_use = FRAGMENT_TX;
         fragment_startSend(buffer);
      }
   }
}

// Restore fragmentation header and send it to openbridge

void fragment_openbridge(FragmentQueueEntry_t* buffer) {
   uint8_t           i;
   uint8_t           length;
   uint16_t          received;
   OpenQueueEntry_t* msg;
   bool              finished;
   INTERRUPT_DECLARATION();

   received = 0;
   DISABLE_INTERRUPTS();
   for (i=0; i<buffer->number; i++) {
      if (buffer->other.list[i].state == FRAGMENT_RECEIVED) {
         msg = buffer->other.list[i].fragment;
         // check fragment kind
	 if ( msg == buffer->msg ) {
            buffer->msg = NULL;
	    length = FRAGMENT_FRAG1_HL;
	 } else
            length = FRAGMENT_FRAGN_HL;
	 // restore lost octets
	 packetfunctions_reserveHeaderSize(msg, length);
	 msg->payload[0] |= ((length == FRAGMENT_FRAG1_HL ? IPHC_DISPATCH_FRAG1 : IPHC_DISPATCH_FRAGN) << IPHC_FRAGMENT);
         buffer->other.list[i].state = FRAGMENT_PROCESSED;
//      }
//      if (buffer->other.list[i].state == FRAGMENT_PROCESSED) {
//         msg = buffer->other.list[i].fragment;
         ENABLE_INTERRUPTS();
	 openbridge_receive(msg);
         DISABLE_INTERRUPTS();
	 // msg have been freed by openbridge
	 buffer->other.list[i].fragment = NULL;
	 buffer->other.list[i].state = FRAGMENT_FINISHED;
      }
      if (buffer->other.list[i].state == FRAGMENT_FINISHED)
         received += buffer->other.list[i].fragment_size;
   }
   finished = received == buffer->datagram_size;
   ENABLE_INTERRUPTS();
   if ( finished )
      fragment_freeBuffer(buffer);
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
