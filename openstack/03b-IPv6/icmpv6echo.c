#include "opendefs.h"
#include "icmpv6echo.h"
#include "icmpv6.h"
#include "openserial.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "debugpins.h"

//=========================== variables =======================================

icmpv6echo_vars_t icmpv6echo_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void icmpv6echo_init() {
   icmpv6echo_vars.busySending = FALSE;
   icmpv6echo_vars.seq         = 0;
}

void icmpv6echo_trigger() {
   uint8_t number_bytes_from_input_buffer;
   uint8_t input_buffer[16];
   OpenQueueEntry_t* msg;
 
   
   //get command from OpenSerial (16B IPv6 destination address)
   number_bytes_from_input_buffer = openserial_getInputBuffer(&(input_buffer[0]),sizeof(input_buffer));
   if (number_bytes_from_input_buffer!=sizeof(input_buffer)) {
      openserial_printError(COMPONENT_ICMPv6ECHO,ERR_INPUTBUFFER_LENGTH,
                            (errorparameter_t)number_bytes_from_input_buffer,
                            (errorparameter_t)0);
      return;
   };
   icmpv6echo_vars.hisAddress.type  = ADDR_128B;
   memcpy(&(icmpv6echo_vars.hisAddress.addr_128b[0]),&(input_buffer[0]),16);
   
   //send
   if (icmpv6echo_vars.busySending==TRUE) {
      openserial_printError(COMPONENT_ICMPv6ECHO,ERR_BUSY_SENDING,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
   } else {
      icmpv6echo_vars.busySending = TRUE;
      
      msg = openqueue_getFreePacketBuffer(COMPONENT_ICMPv6ECHO);
      if (msg==NULL) {
         openserial_printError(COMPONENT_ICMPv6ECHO,ERR_NO_FREE_PACKET_BUFFER,
                               (errorparameter_t)0,
                               (errorparameter_t)0);
         icmpv6echo_vars.busySending = FALSE;
         return;
      }
      //admin
      msg->creator                               = COMPONENT_ICMPv6ECHO;
      msg->owner                                 = COMPONENT_ICMPv6ECHO;
      //l4
      msg->l4_protocol                           = IANA_ICMPv6;
      msg->l4_sourcePortORicmpv6Type             = IANA_ICMPv6_ECHO_REQUEST;
      //l3
      memcpy(&(msg->l3_destinationAdd),&icmpv6echo_vars.hisAddress,sizeof(open_addr_t));
      //payload
      packetfunctions_reserveHeaderSize(msg,4);
      packetfunctions_htonl(0x789abcde,(uint8_t*)(msg->payload));
      //ICMPv6 header
      packetfunctions_reserveHeaderSize(msg,sizeof(ICMPv6_ht));
      ((ICMPv6_ht*)(msg->payload))->type         = msg->l4_sourcePortORicmpv6Type;
      ((ICMPv6_ht*)(msg->payload))->code         = 0;
      // Below Identifier might need to be replaced by the identifier used by icmpv6rpl
      // packetfunctions_htons(0x1234       ,(uint8_t*)&((ICMPv6_ht*)(msg->payload))->identifier);
      // Below sequence_number might need to be removed
      // packetfunctions_htons(icmpv6echo_vars.seq++ ,(uint8_t*)&((ICMPv6_ht*)(msg->payload))->sequence_number); 
      packetfunctions_calculateChecksum(msg,(uint8_t*)&(((ICMPv6_ht*)(msg->payload))->checksum));//do last
      //send
      if (icmpv6_send(msg)!=E_SUCCESS) {
         icmpv6echo_vars.busySending = FALSE;
         openqueue_freePacketBuffer(msg);
      }
   }
}

void icmpv6echo_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   msg->owner = COMPONENT_ICMPv6ECHO;
   if (msg->creator!=COMPONENT_ICMPv6ECHO) {//that was a packet I had not created
      openserial_printError(COMPONENT_ICMPv6ECHO,ERR_UNEXPECTED_SENDDONE,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
   }
   openqueue_freePacketBuffer(msg);
   icmpv6echo_vars.busySending = FALSE;
 }

void icmpv6echo_receive(OpenQueueEntry_t* msg) {
   OpenQueueEntry_t* reply;
   msg->owner = COMPONENT_ICMPv6ECHO;
   switch(msg->l4_sourcePortORicmpv6Type) {
      case IANA_ICMPv6_ECHO_REQUEST:
         openserial_printInfo(COMPONENT_ICMPv6ECHO,ERR_RCVD_ECHO_REQUEST,
                               (errorparameter_t)0,
                               (errorparameter_t)0);
         // get a new openqueuEntry_t for the echo reply
         reply = openqueue_getFreePacketBuffer(COMPONENT_ICMPv6ECHO);
         if (reply==NULL) {
            openserial_printError(COMPONENT_ICMPv6ECHO,ERR_NO_FREE_PACKET_BUFFER,
                                  (errorparameter_t)1,
                                  (errorparameter_t)0);
            openqueue_freePacketBuffer(msg);
            return;
         }
         // take ownership over reply
         reply->creator = COMPONENT_ICMPv6ECHO;
         reply->owner   = COMPONENT_ICMPv6ECHO;
         // copy payload from msg to (end of) reply
         packetfunctions_reserveHeaderSize(reply,msg->length);
         memcpy(reply->payload,msg->payload,msg->length);
         // copy source of msg in destination of reply
         memcpy(&(reply->l3_destinationAdd),&(msg->l3_sourceAdd),sizeof(open_addr_t));
         // free up msg
         openqueue_freePacketBuffer(msg);
         msg = NULL;
         // administrative information for reply
         reply->l4_protocol                   = IANA_ICMPv6;
         reply->l4_sourcePortORicmpv6Type     = IANA_ICMPv6_ECHO_REPLY;
         ((ICMPv6_ht*)(reply->payload))->type = reply->l4_sourcePortORicmpv6Type;
         packetfunctions_calculateChecksum(reply,(uint8_t*)&(((ICMPv6_ht*)(reply->payload))->checksum));//do last
         icmpv6echo_vars.busySending = TRUE;
         if (icmpv6_send(reply)!=E_SUCCESS) {
            icmpv6echo_vars.busySending = FALSE;
            openqueue_freePacketBuffer(reply);
         }
         break;
      case IANA_ICMPv6_ECHO_REPLY:
         openserial_printInfo(COMPONENT_ICMPv6ECHO,ERR_RCVD_ECHO_REPLY,
                               (errorparameter_t)0,
                               (errorparameter_t)0);
         openqueue_freePacketBuffer(msg);
         break;
      default:
         openserial_printError(COMPONENT_ICMPv6ECHO,ERR_UNSUPPORTED_ICMPV6_TYPE,
                               (errorparameter_t)msg->l4_sourcePortORicmpv6Type,
                               (errorparameter_t)2);
         openqueue_freePacketBuffer(msg);
         break;
   }
}

//=========================== private =========================================
