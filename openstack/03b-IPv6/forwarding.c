#include "opendefs.h"
#include "forwarding.h"
#include "iphc.h"
#include "openqueue.h"
#include "openserial.h"
#include "idmanager.h"
#include "packetfunctions.h"
#include "neighbors.h"
#include "icmpv6.h"
#include "icmpv6rpl.h"
#include "openudp.h"
#include "opentcp.h"
#include "debugpins.h"
#include "scheduler.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

void      forwarding_getNextHop(
   open_addr_t*         destination,
   open_addr_t*         addressToWrite
);
owerror_t forwarding_send_internal_RoutingTable(
   OpenQueueEntry_t*    msg,
   ipv6_header_iht*     ipv6_header,
   rpl_option_ht*       rpl_option,
   uint32_t*             flow_label,
   uint8_t              fw_SendOrfw_Rcv
);
owerror_t forwarding_send_internal_SourceRouting(
   OpenQueueEntry_t*    msg,
   ipv6_header_iht*     ipv6_header
);
void      forwarding_createRplOption(
   rpl_option_ht*       rpl_option,
   uint8_t              flags
);

#ifdef FLOW_LABEL_RPL_DOMAIN
void forwarding_createFlowLabel(uint32_t* flow_label,uint8_t flags);
#endif
//=========================== public ==========================================

/**
\brief Initialize this module.
*/
void forwarding_init() {
}

/**
\brief Send a packet originating at this mote.

This function is called by an upper layer, and only concerns packets originated
at this mote.

\param[in,out] msg Packet to send.
*/
owerror_t forwarding_send(OpenQueueEntry_t* msg) {
   ipv6_header_iht      ipv6_header;
   rpl_option_ht        rpl_option;
   open_addr_t*         myprefix;
   open_addr_t*         myadd64;
   uint32_t             flow_label = 0;
   
   // take ownership over the packet
   msg->owner                = COMPONENT_FORWARDING;
   
   // retrieve my prefix and EUI64
   myprefix                  = idmanager_getMyID(ADDR_PREFIX);
   myadd64                   = idmanager_getMyID(ADDR_64B);
   
   // set source address (me)
   msg->l3_sourceAdd.type=ADDR_128B;
   memcpy(&(msg->l3_sourceAdd.addr_128b[0]),myprefix->prefix,8);
   memcpy(&(msg->l3_sourceAdd.addr_128b[8]),myadd64->addr_64b,8);
   
   // initialize IPv6 header
   memset(&ipv6_header,0,sizeof(ipv6_header_iht));
   
   // Set hop limit to the default in-network value as this packet is being
   // sent from upper layer. This is done here as send_internal() is used by
   // forwarding of packets as well which carry a hlim. This value is required
   // to be set to a value as the following function can decrement it.
   ipv6_header.hop_limit     = IPHC_DEFAULT_HOP_LIMIT;
   
   // create the RPL hop-by-hop option

   forwarding_createRplOption(
      &rpl_option,      // rpl_option to fill in
      0x00              // flags
   );

#ifdef FLOW_LABEL_RPL_DOMAIN
   forwarding_createFlowLabel(&flow_label,0x00);
#endif

   return forwarding_send_internal_RoutingTable(
      msg,
      &ipv6_header,
      &rpl_option,
      &flow_label,
      PCKTSEND
   );
}

/**
\brief Indicates a packet has been sent.

\param[in,out] msg   The packet just sent.
\param[in]     error The outcome of sending it.
*/
void forwarding_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   
   // take ownership
   msg->owner = COMPONENT_FORWARDING;
   
   if (msg->creator==COMPONENT_RADIO || msg->creator==COMPONENT_FORWARDING) {
      // this is a relayed packet
      
      // free packet
      openqueue_freePacketBuffer(msg);
   } else {
      // this is a packet created by this mote
      
      // indicate sendDone to upper layer
      switch(msg->l4_protocol) {
         case IANA_TCP:
            opentcp_sendDone(msg,error);
            break;
         case IANA_UDP:
            openudp_sendDone(msg,error);
            break;
         case IANA_ICMPv6:
            icmpv6_sendDone(msg,error);
            break;
         default:
            
            // log error
            openserial_printCritical(
               COMPONENT_FORWARDING,
               ERR_WRONG_TRAN_PROTOCOL,
               (errorparameter_t)msg->l4_protocol,
               (errorparameter_t)0
            );
            
            // free packet
            openqueue_freePacketBuffer(msg);
      }
   }
}

/**
\brief Indicates a packet was received.

\param[in,out] msg               The packet just sent.
\param[in]     ipv6_header       The information contained in the IPv6 header.
\param[in]     ipv6_hop_header   The hop-by-hop header present in the packet.
\param[in]     rpl_option        The hop-by-hop options present in the packet.
*/
void forwarding_receive(
      OpenQueueEntry_t*      msg,
      ipv6_header_iht*       ipv6_header,
      ipv6_hopbyhop_iht*     ipv6_hop_header,
      rpl_option_ht*         rpl_option
   ) {
   uint8_t flags;
   uint16_t senderRank;
   
   // take ownership
   msg->owner                     = COMPONENT_FORWARDING;
   
   // determine L4 protocol
   if (ipv6_header->next_header==IANA_IPv6HOPOPT){
      // get information from ipv6_hop_header
      
      msg->l4_protocol            = ipv6_hop_header->nextHeader;
      msg->l4_protocol_compressed = ipv6_hop_header->next_header_compressed;
   } else {
      // get information from ipv6_header
      
      msg->l4_protocol            = ipv6_header->next_header;
      msg->l4_protocol_compressed = ipv6_header->next_header_compressed;
   }
   
   // populate packets metadata with L3 information
   memcpy(&(msg->l3_destinationAdd),&ipv6_header->dest,sizeof(open_addr_t));
   memcpy(&(msg->l3_sourceAdd),     &ipv6_header->src, sizeof(open_addr_t));
   
   if (
         (
            idmanager_isMyAddress(&ipv6_header->dest)
            ||
            packetfunctions_isBroadcastMulticast(&ipv6_header->dest)
         )
         &&
         ipv6_header->next_header!=IANA_IPv6ROUTE
      ) {
      // this packet is for me, no source routing header.

      // indicate received packet to upper layer
      switch(msg->l4_protocol) {
         case IANA_TCP:
            opentcp_receive(msg);
            break;
         case IANA_UDP:
            openudp_receive(msg);
            break;
         case IANA_ICMPv6:
            icmpv6_receive(msg);
            break;
         default:
            
            // log error
            openserial_printError(
               COMPONENT_FORWARDING,ERR_WRONG_TRAN_PROTOCOL,
               (errorparameter_t)msg->l4_protocol,
               (errorparameter_t)1
            );
            
            // free packet
            openqueue_freePacketBuffer(msg);
      }
   } else {
      // this packet is not for me: relay
      
      // change the creator of the packet
      msg->creator = COMPONENT_FORWARDING;
      
      if (ipv6_header->next_header!=IANA_IPv6ROUTE) {
         // no source routing header present
         //check if flow label rpl header
    	 #ifdef FLOW_LABEL_RPL_DOMAIN             
             flags = (uint8_t)((uint32_t)((ipv6_header->flow_label)>>16)&0xFF);
             senderRank = (uint16_t)((uint32_t)(ipv6_header->flow_label)>>8)&0xFFFF;
             senderRank = senderRank*MINHOPRANKINCREASE;//shift it according to HopRank Increase
         #else
    	     flags = rpl_option->flags;
    	     senderRank = rpl_option->senderRank;
    	 #endif

         if ((flags & O_FLAG)!=0){
            // wrong direction
            
            // log error
            openserial_printError(
               COMPONENT_FORWARDING,
               ERR_WRONG_DIRECTION,
               (errorparameter_t)flags,
               (errorparameter_t)senderRank
            );
         }
         

         if (senderRank < neighbors_getMyDAGrank()){
            // loop
            
            // set flag
            #ifdef FLOW_LABEL_RPL_DOMAIN
        	    flags |= R_FLAG;
        	    ipv6_header->flow_label|= ((uint32_t)flags<<16);
            #else
        	    rpl_option->flags |= R_FLAG;
            #endif

            // log error
            openserial_printError(
               COMPONENT_FORWARDING,
               ERR_LOOP_DETECTED,
               (errorparameter_t) senderRank,
               (errorparameter_t) neighbors_getMyDAGrank()
            );
         }
         

         forwarding_createRplOption(rpl_option, rpl_option->flags);
         #ifdef FLOW_LABEL_RPL_DOMAIN
         // do not recreate flow label, relay the same but adding current flags
         //forwarding_createFlowLabel(&(ipv6_header->flow_label),flags);
         #endif
         // resend as if from upper layer
         if (
               forwarding_send_internal_RoutingTable(
                  msg,
                  ipv6_header,
                  rpl_option,
                  &(ipv6_header->flow_label),
                  PCKTFORWARD 
               )==E_FAIL
            ) {
            openqueue_freePacketBuffer(msg);
         }
      } else {
         // source routing header present
         
         if (forwarding_send_internal_SourceRouting(msg, ipv6_header)==E_FAIL) {
            
            // already freed by send_internal
            
            // log error
            openserial_printError(
               COMPONENT_FORWARDING,
               ERR_INVALID_FWDMODE,
               (errorparameter_t)0,
               (errorparameter_t)0
            );
         }
      }
   }
}

//=========================== private =========================================

/**
\brief Retrieve the next hop's address from routing table.

\param[in]  destination128b  Final IPv6 destination address.
\param[out] addressToWrite64b Location to write the EUI64 of next hop to.
*/
void forwarding_getNextHop(open_addr_t* destination128b, open_addr_t* addressToWrite64b) {
   uint8_t         i;
   open_addr_t     temp_prefix64btoWrite;
   
   if (packetfunctions_isBroadcastMulticast(destination128b)) {
      // IP destination is broadcast, send to 0xffffffffffffffff
      addressToWrite64b->type = ADDR_64B;
      for (i=0;i<8;i++) {
         addressToWrite64b->addr_64b[i] = 0xff;
      }
   } else if (neighbors_isStableNeighbor(destination128b)) {
      // IP destination is 1-hop neighbor, send directly
      packetfunctions_ip128bToMac64b(destination128b,&temp_prefix64btoWrite,addressToWrite64b);
   } else {
      // destination is remote, send to preferred parent
      neighbors_getPreferredParentEui64(addressToWrite64b);
   }
}

/**
\brief Send a packet using the routing table to find the next hop.

\param[in,out] msg             The packet to send.
\param[in]     ipv6_header     The packet's IPv6 header.
\param[in]     rpl_option      The hop-by-hop option to add in this packet.
\param[in]     flow_label      The flowlabel to add in the 6LoWPAN header.
\param[in]     fw_SendOrfw_Rcv The packet is originating from this mote
   (PCKTSEND), or forwarded (PCKTFORWARD).
*/
owerror_t forwarding_send_internal_RoutingTable(
      OpenQueueEntry_t*      msg,
      ipv6_header_iht*       ipv6_header,
      rpl_option_ht*         rpl_option,
      uint32_t*              flow_label,
      uint8_t                fw_SendOrfw_Rcv
   ) {
   
   // retrieve the next hop from the routing table
   forwarding_getNextHop(&(msg->l3_destinationAdd),&(msg->l2_nextORpreviousHop));
   if (msg->l2_nextORpreviousHop.type==ADDR_NONE) {
      openserial_printError(
         COMPONENT_FORWARDING,
         ERR_NO_NEXTHOP,
         (errorparameter_t)0,
         (errorparameter_t)0
      );
      return E_FAIL;
   }
   
   // send to next lower layer
   return iphc_sendFromForwarding(
      msg,
      ipv6_header,
      rpl_option,
      flow_label,
      fw_SendOrfw_Rcv
   );
}

/**
\brief Send a packet using the source rout to find the next hop.

\note This is always called for packets being forwarded.

How to process the routing header is detailed in
http://tools.ietf.org/html/rfc6554#page-9.

\param[in,out] msg             The packet to send.
\param[in]     ipv6_header     The packet's IPv6 header.
*/
owerror_t forwarding_send_internal_SourceRouting(
      OpenQueueEntry_t* msg,
      ipv6_header_iht*  ipv6_header
   ) {
   uint8_t              local_CmprE;
   uint8_t              local_CmprI;
   uint8_t              numAddr;
   uint8_t              hlen;
   uint8_t              addressposition;
   uint8_t*             runningPointer;
   uint8_t              octetsAddressSize;
   open_addr_t*         prefix;
   rpl_routing_ht*      rpl_routing_hdr;
   rpl_option_ht        rpl_option;
   
   // reset hop-by-hop option
   memset(&rpl_option,0,sizeof(rpl_option_ht));
   
   // get my prefix
   prefix               = idmanager_getMyID(ADDR_PREFIX);
   
   // cast packet to RPL routing header
   rpl_routing_hdr      = (rpl_routing_ht*)(msg->payload);
   
   // point behind the RPL routing header
   runningPointer       = (msg->payload)+sizeof(rpl_routing_ht);
   
   // retrieve CmprE and CmprI
   
   // CmprE 4-bit unsigned integer. Number of prefix octets
   // from the last segment (i.e., segment n) that are
   // elided. For example, an SRH carrying a full IPv6
   // address in Addressesn sets CmprE to 0.
   
   local_CmprE          = rpl_routing_hdr->CmprICmprE & 0x0f;
   local_CmprI          = rpl_routing_hdr->CmprICmprE & 0xf0;
   local_CmprI          = local_CmprI>>4;
   
   numAddr              = (((rpl_routing_hdr->HdrExtLen*8)-rpl_routing_hdr->PadRes-(16-local_CmprE))/(16-local_CmprI))+1;
   
   if (rpl_routing_hdr->SegmentsLeft==0){
      // no more segments left, this is the last hop
      
      // push packet up the stack
      msg->l4_protocol  = rpl_routing_hdr->nextHeader;
      hlen              = rpl_routing_hdr->HdrExtLen; //in 8-octet units
      
      // toss RPL routing header
      packetfunctions_tossHeader(msg,sizeof(rpl_routing_ht));
      
      // toss source route addresses
      octetsAddressSize = LENGTH_ADDR128b - local_CmprE; //total length - number of octets that are elided
      packetfunctions_tossHeader(msg,octetsAddressSize*hlen);
      
      // indicate reception to upper layer
      switch(msg->l4_protocol) {
         case IANA_TCP:
            opentcp_receive(msg);
            break;
         case IANA_UDP:
            openudp_receive(msg);
            break;
         case IANA_ICMPv6:
            icmpv6_receive(msg);
            break;
         default:
            openserial_printError(
               COMPONENT_FORWARDING,
               ERR_WRONG_TRAN_PROTOCOL,
               (errorparameter_t)msg->l4_protocol,
               (errorparameter_t)1
            );
            //not sure that this is correct as iphc will free it?
            openqueue_freePacketBuffer(msg);
            return E_FAIL;
      }
      
      // stop executing here (successful)
      return E_SUCCESS;
   
   } else {
      // this is not the last hop
      
      if (rpl_routing_hdr->SegmentsLeft>numAddr) {
         // error code: there are more segments left than space in source route
         
         // TODO: send ICMPv6 packet (code 0) to originator
         
         openserial_printError(
            COMPONENT_FORWARDING,
            ERR_NO_NEXTHOP,
            (errorparameter_t)0,
            (errorparameter_t)0
         );
         openqueue_freePacketBuffer(msg);
         return E_FAIL;
      
      } else {
         
         // decrement number of segments left
         rpl_routing_hdr->SegmentsLeft--;
         
         // find next hop address in source route
         // addressposition    = numAddr-(rpl_routing_hdr->SegmentsLeft);
         addressposition     = rpl_routing_hdr->SegmentsLeft;
         
         // how many octets have the address?
         if (rpl_routing_hdr->SegmentsLeft > 1){
            // max addr length - number of prefix octets that are elided in the internal route elements
            octetsAddressSize = LENGTH_ADDR128b - local_CmprI;
         } else {
            // max addr length - number of prefix octets that are elided in the internal route elements
            octetsAddressSize = LENGTH_ADDR128b - local_CmprE;
         }
         
         switch(octetsAddressSize) {
            
            case LENGTH_ADDR16b:
               
               // write previous hop
               msg->l2_nextORpreviousHop.type    = ADDR_16B;
               memcpy(
                  &(msg->l2_nextORpreviousHop.addr_16b),
                  runningPointer+((addressposition)*octetsAddressSize),
                  octetsAddressSize
               );
               
               // write next hop
               msg->l3_destinationAdd.type       = ADDR_16B;
               memcpy(
                  &(msg->l3_destinationAdd.addr_16b),
                  runningPointer+((addressposition)*octetsAddressSize),
                  octetsAddressSize
               );
               
               break;
            
            case LENGTH_ADDR64b:
               
               // write previous hop
               msg->l2_nextORpreviousHop.type    = ADDR_64B;
               memcpy(
                  &(msg->l2_nextORpreviousHop.addr_64b),
                  runningPointer+((addressposition)*octetsAddressSize),
                  octetsAddressSize
               );
               
               //this is 128b address as send from forwarding function
               //takes care of reducing it if needed.
               
               //write next hop
               msg->l3_destinationAdd.type       = ADDR_128B;
               memcpy(
                  &(msg->l3_destinationAdd.addr_128b[0]),
                  prefix->prefix,
                  LENGTH_ADDR64b
               );
               
               memcpy(
                  &(msg->l3_destinationAdd.addr_128b[8]),
                  runningPointer+((addressposition)*octetsAddressSize),
                  octetsAddressSize
               );
               
               break;
            
            case LENGTH_ADDR128b:
               
               // write previous hop
               msg->l2_nextORpreviousHop.type    = ADDR_128B;
               memcpy(
                  &(msg->l2_nextORpreviousHop.addr_128b),
                  runningPointer+((addressposition)*octetsAddressSize),
                  octetsAddressSize
               );
               
               // write next hop
               msg->l3_destinationAdd.type       = ADDR_128B;
               memcpy(
                  &(msg->l3_destinationAdd.addr_128b),
                  runningPointer+((addressposition)*octetsAddressSize),
                  octetsAddressSize
               );
               
               break;
            
            default:
               // any other value is not supported by now
               
               openserial_printError(
                  COMPONENT_FORWARDING,
                  ERR_INVALID_PARAM,
                  (errorparameter_t)1,
                  (errorparameter_t)0
               );
               openqueue_freePacketBuffer(msg);
               return E_FAIL;
         }
      }
   }
   
   // send to next lower layer
   return iphc_sendFromForwarding(
      msg,
      ipv6_header,
      &rpl_option,
      &ipv6_header->flow_label,
      PCKTFORWARD
   );
}


/**
\brief Create a RPL option.

\param[out] rpl_option A pointer to the structure to fill in.
\param[in]  flags      The flags to indicate in the RPL option.
*/
void forwarding_createRplOption(rpl_option_ht* rpl_option, uint8_t flags) {
   // set the RPL hop-by-hop header
   rpl_option->optionType         = RPL_HOPBYHOP_HEADER_OPTION_TYPE;
   
   // 8-bit field indicating the length of the option, in
   // octets, excluding the Option Type and Opt Data Len fields.
   // 4-bytes, flags+instanceID+senderrank - no sub-tlvs
   rpl_option->optionLen          = 0x04; 
   
   rpl_option->flags              = flags;
   rpl_option->rplInstanceID      = icmpv6rpl_getRPLIntanceID();
   rpl_option->senderRank         = neighbors_getMyDAGrank();
}

#ifdef FLOW_LABEL_RPL_DOMAIN
void forwarding_createFlowLabel(uint32_t* flow_label,uint8_t flags){
     uint8_t instanceId,flrank;
     uint16_t rank;

     instanceId=icmpv6rpl_getRPLIntanceID();
     rank=neighbors_getMyDAGrank();
     flrank=(uint8_t)(rank/MINHOPRANKINCREASE);
     *flow_label = (uint32_t)instanceId | ((uint32_t)flrank<<8) | (uint32_t)flags<<16;
}
#endif
