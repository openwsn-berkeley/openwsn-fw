#include "openwsn.h"
#include "icmpv6router.h"
#include "icmpv6.h"
#include "openserial.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "idmanager.h"

//=========================== variables =======================================

typedef struct {
bool        busySending;
open_addr_t hisAddress;
uint16_t    seq;
} icmpv6router_vars_t;

icmpv6router_vars_t icmpv6router_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void icmpv6router_init() {
   icmpv6router_vars.busySending = FALSE;
   icmpv6router_vars.seq         = 0;
}

void icmpv6router_trigger() {
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
   icmpv6router_vars.hisAddress.type  = ADDR_128B;
   memcpy(&(icmpv6router_vars.hisAddress.addr_128b[0]),&(input_buffer[0]),16);
   //send
   if (icmpv6router_vars.busySending==TRUE) {
      openserial_printError(COMPONENT_ICMPv6ROUTER,ERR_BUSY_SENDING,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
   } else {
      icmpv6router_vars.busySending = TRUE;
      
      msg = openqueue_getFreePacketBuffer(COMPONENT_ICMPv6ROUTER);
      if (msg==NULL) {
         openserial_printError(COMPONENT_ICMPv6ROUTER,ERR_NO_FREE_PACKET_BUFFER,
                               (errorparameter_t)0,
                               (errorparameter_t)0);
         return;
      }
      //admin
      msg->creator                               = COMPONENT_ICMPv6ROUTER;
      msg->owner                                 = COMPONENT_ICMPv6ROUTER;
      //l4
      msg->l4_protocol                           = IANA_ICMPv6;
      msg->l4_sourcePortORicmpv6Type             = IANA_ICMPv6_RS;
      //l3
      msg->l3_destinationORsource.type           = ADDR_128B;
      msg->l3_destinationORsource.addr_128b[0]   = 0xff;
      msg->l3_destinationORsource.addr_128b[1]   = 0x02;
      msg->l3_destinationORsource.addr_128b[2]   = 0x00;
      msg->l3_destinationORsource.addr_128b[3]   = 0x00;
      msg->l3_destinationORsource.addr_128b[4]   = 0x00;
      msg->l3_destinationORsource.addr_128b[5]   = 0x00;
      msg->l3_destinationORsource.addr_128b[6]   = 0x00;
      msg->l3_destinationORsource.addr_128b[7]   = 0x00;
      msg->l3_destinationORsource.addr_128b[8]   = 0x00;
      msg->l3_destinationORsource.addr_128b[9]   = 0x00;
      msg->l3_destinationORsource.addr_128b[10]  = 0x00;
      msg->l3_destinationORsource.addr_128b[11]  = 0x00;
      msg->l3_destinationORsource.addr_128b[12]  = 0x00;
      msg->l3_destinationORsource.addr_128b[13]  = 0x00;
      msg->l3_destinationORsource.addr_128b[14]  = 0x00;
      msg->l3_destinationORsource.addr_128b[15]  = 0x02;
      //ICMPv6 header
      packetfunctions_reserveHeaderSize(msg,sizeof(ICMPv6_ht));
      ((ICMPv6_ht*)(msg->payload))->type         = msg->l4_sourcePortORicmpv6Type;
      ((ICMPv6_ht*)(msg->payload))->code         = 0;
      packetfunctions_htons(0x1234       ,(uint8_t*)&((ICMPv6_ht*)(msg->payload))->identifier);
      packetfunctions_htons(icmpv6router_vars.seq++ ,(uint8_t*)&((ICMPv6_ht*)(msg->payload))->sequence_number); 
      packetfunctions_calculateChecksum(msg,(uint8_t*)&(((ICMPv6_ht*)(msg->payload))->checksum));//call last
      //send
      if (icmpv6_send(msg)!=E_SUCCESS) {
         icmpv6router_vars.busySending = FALSE;
         openqueue_freePacketBuffer(msg);
      }
   }
}

void icmpv6router_sendDone(OpenQueueEntry_t* msg, error_t error) {
   msg->owner = COMPONENT_ICMPv6ROUTER;
   if (msg->creator!=COMPONENT_ICMPv6ROUTER) {//that was a packet I had not created
      openserial_printError(COMPONENT_ICMPv6ROUTER,ERR_UNEXPECTED_SENDDONE,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
   }
   openqueue_freePacketBuffer(msg);
   icmpv6router_vars.busySending = FALSE;
}

void icmpv6router_receive(OpenQueueEntry_t* msg) {
   open_addr_t temp_prefix;
   msg->owner = COMPONENT_ICMPv6ROUTER;
   //toss ICMPv6 header
   packetfunctions_tossHeader(msg,sizeof(ICMPv6_RA_ht));
   //record prefix
   if (  ((ICMPv6_64bprefix_option_ht*)(msg->payload))->option_type   == IANA_ICMPv6_RA_PREFIX_INFORMATION &&
         ((ICMPv6_64bprefix_option_ht*)(msg->payload))->option_length == 4                                 &&
         ((ICMPv6_64bprefix_option_ht*)(msg->payload))->prefix_length == 64                                &&
         msg->length>=sizeof(ICMPv6_64bprefix_option_ht) ) {
      temp_prefix.type = ADDR_PREFIX;
      memcpy(  &(temp_prefix.prefix[0]),
            &(((ICMPv6_64bprefix_option_ht*)(msg->payload))->prefix[0]),
            8);
      idmanager_setMyID(&temp_prefix);
   }
   //toss packet
   openqueue_freePacketBuffer(msg);
}

//=========================== private =========================================