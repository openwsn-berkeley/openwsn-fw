/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:10:42.209152.
*/
#include "openwsn_obj.h"
#include "icmpv6echo_obj.h"
#include "icmpv6_obj.h"
#include "openserial_obj.h"
#include "openqueue_obj.h"
#include "packetfunctions_obj.h"
#include "debugpins_obj.h"

//=========================== variables =======================================

// declaration of global variable _icmpv6echo_vars_ removed during objectification.

//=========================== prototypes ======================================

//=========================== public ==========================================

void icmpv6echo_init(OpenMote* self) {
   (self->icmpv6echo_vars).busySending = FALSE;
   (self->icmpv6echo_vars).seq         = 0;
}

void icmpv6echo_trigger(OpenMote* self) {
   uint8_t number_bytes_from_input_buffer;
   uint8_t input_buffer[16];
   OpenQueueEntry_t* msg;
 
   
   //get command from OpenSerial (16B IPv6 destination address)
   number_bytes_from_input_buffer = openserial_getInputBuffer(self, &(input_buffer[0]),sizeof(input_buffer));
   if (number_bytes_from_input_buffer!=sizeof(input_buffer)) {
// openserial_printError(self, COMPONENT_ICMPv6ECHO,ERR_INPUTBUFFER_LENGTH,
//                            (errorparameter_t)number_bytes_from_input_buffer,
//                            (errorparameter_t)0);
      return;
   };
   (self->icmpv6echo_vars).hisAddress.type  = ADDR_128B;
   memcpy(&((self->icmpv6echo_vars).hisAddress.addr_128b[0]),&(input_buffer[0]),16);
   
   //send
   if ((self->icmpv6echo_vars).busySending==TRUE) {
// openserial_printError(self, COMPONENT_ICMPv6ECHO,ERR_BUSY_SENDING,
//                            (errorparameter_t)0,
//                            (errorparameter_t)0);
   } else {
      (self->icmpv6echo_vars).busySending = TRUE;
      
      msg = openqueue_getFreePacketBuffer(self, COMPONENT_ICMPv6ECHO);
      if (msg==NULL) {
// openserial_printError(self, COMPONENT_ICMPv6ECHO,ERR_NO_FREE_PACKET_BUFFER,
//                               (errorparameter_t)0,
//                               (errorparameter_t)0);
         (self->icmpv6echo_vars).busySending = FALSE;
         return;
      }
      //admin
      msg->creator                               = COMPONENT_ICMPv6ECHO;
      msg->owner                                 = COMPONENT_ICMPv6ECHO;
      //l4
      msg->l4_protocol                           = IANA_ICMPv6;
      msg->l4_sourcePortORicmpv6Type             = IANA_ICMPv6_ECHO_REQUEST;
      //l3
      memcpy(&(msg->l3_destinationAdd),&(self->icmpv6echo_vars).hisAddress,sizeof(open_addr_t));
      //payload
 packetfunctions_reserveHeaderSize(self, msg,4);
 packetfunctions_htonl(self, 0x789abcde,(uint8_t*)(msg->payload));
      //ICMPv6 header
 packetfunctions_reserveHeaderSize(self, msg,sizeof(ICMPv6_ht));
      ((ICMPv6_ht*)(msg->payload))->type         = msg->l4_sourcePortORicmpv6Type;
      ((ICMPv6_ht*)(msg->payload))->code         = 0;
      // Below Identifier might need to be replaced by the identifier used by icmpv6rpl
      // packetfunctions_htons(self, 0x1234       ,(uint8_t*)&((ICMPv6_ht*)(msg->payload))->identifier);
      // Below sequence_number might need to be removed
      // packetfunctions_htons(self, (self->icmpv6echo_vars).seq++ ,(uint8_t*)&((ICMPv6_ht*)(msg->payload))->sequence_number); 
 packetfunctions_calculateChecksum(self, msg,(uint8_t*)&(((ICMPv6_ht*)(msg->payload))->checksum));//do last
      //send
      if ( icmpv6_send(self, msg)!=E_SUCCESS) {
         (self->icmpv6echo_vars).busySending = FALSE;
 openqueue_freePacketBuffer(self, msg);
      }
   }
}

void icmpv6echo_sendDone(OpenMote* self, OpenQueueEntry_t* msg, owerror_t error) {
   msg->owner = COMPONENT_ICMPv6ECHO;
   if (msg->creator!=COMPONENT_ICMPv6ECHO) {//that was a packet I had not created
// openserial_printError(self, COMPONENT_ICMPv6ECHO,ERR_UNEXPECTED_SENDDONE,
//                            (errorparameter_t)0,
//                            (errorparameter_t)0);
   }
 openqueue_freePacketBuffer(self, msg);
   (self->icmpv6echo_vars).busySending = FALSE;
 }

void icmpv6echo_receive(OpenMote* self, OpenQueueEntry_t* msg) {
   OpenQueueEntry_t* reply;
   msg->owner = COMPONENT_ICMPv6ECHO;
   switch(msg->l4_sourcePortORicmpv6Type) {
      case IANA_ICMPv6_ECHO_REQUEST:
// openserial_printInfo(self, COMPONENT_ICMPv6ECHO,ERR_RCVD_ECHO_REQUEST,
//                               (errorparameter_t)0,
//                               (errorparameter_t)0);
         // get a new openqueuEntry_t for the echo reply
         reply = openqueue_getFreePacketBuffer(self, COMPONENT_ICMPv6ECHO);
         if (reply==NULL) {
// openserial_printError(self, COMPONENT_ICMPv6ECHO,ERR_NO_FREE_PACKET_BUFFER,
//                                  (errorparameter_t)1,
//                                  (errorparameter_t)0);
 openqueue_freePacketBuffer(self, msg);
            return;
         }
         // take ownership over reply
         reply->creator = COMPONENT_ICMPv6ECHO;
         reply->owner   = COMPONENT_ICMPv6ECHO;
         // copy payload from msg to (end of) reply
 packetfunctions_reserveHeaderSize(self, reply,msg->length);
         memcpy(reply->payload,msg->payload,msg->length);
         // copy source of msg in destination of reply
         memcpy(&(reply->l3_destinationAdd),&(msg->l3_sourceAdd),sizeof(open_addr_t));
         // free up msg
 openqueue_freePacketBuffer(self, msg);
         msg = NULL;
         // administrative information for reply
         reply->l4_protocol                   = IANA_ICMPv6;
         reply->l4_sourcePortORicmpv6Type     = IANA_ICMPv6_ECHO_REPLY;
         ((ICMPv6_ht*)(reply->payload))->type = reply->l4_sourcePortORicmpv6Type;
 packetfunctions_calculateChecksum(self, reply,(uint8_t*)&(((ICMPv6_ht*)(reply->payload))->checksum));//do last
         (self->icmpv6echo_vars).busySending = TRUE;
         if ( icmpv6_send(self, reply)!=E_SUCCESS) {
            (self->icmpv6echo_vars).busySending = FALSE;
 openqueue_freePacketBuffer(self, reply);
         }
         break;
      case IANA_ICMPv6_ECHO_REPLY:
// openserial_printInfo(self, COMPONENT_ICMPv6ECHO,ERR_RCVD_ECHO_REPLY,
//                               (errorparameter_t)0,
//                               (errorparameter_t)0);
 openqueue_freePacketBuffer(self, msg);
         break;
      default:
// openserial_printError(self, COMPONENT_ICMPv6ECHO,ERR_UNSUPPORTED_ICMPV6_TYPE,
//                               (errorparameter_t)msg->l4_sourcePortORicmpv6Type,
//                               (errorparameter_t)2);
 openqueue_freePacketBuffer(self, msg);
         break;
   }
}

//=========================== private =========================================
