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
   ipv6_header_iht*     ipv6_outer_header,
   ipv6_header_iht*     ipv6_inner_header,
   rpl_option_ht*       rpl_option,
   uint32_t*             flow_label,
   uint8_t              fw_SendOrfw_Rcv
);
owerror_t forwarding_send_internal_SourceRouting(
   OpenQueueEntry_t*    msg,
   ipv6_header_iht*     ipv6_outer_header,
   ipv6_header_iht*     ipv6_inner_header
);
void      forwarding_createRplOption(
   rpl_option_ht*       rpl_option,
   uint8_t              flags
);


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
   ipv6_header_iht      ipv6_outer_header;
   ipv6_header_iht      ipv6_inner_header;
   rpl_option_ht        rpl_option;
   open_addr_t*         myprefix;
   open_addr_t*         myadd64;
   uint32_t             flow_label = 0;
   
   open_addr_t          temp_dest_prefix;
   open_addr_t          temp_dest_mac64b;
   open_addr_t*         p_dest;
   open_addr_t*         p_src;  
   open_addr_t          temp_src_prefix;
   open_addr_t          temp_src_mac64b; 
   uint8_t              sam;
   uint8_t              m;
   uint8_t              dam;
   
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
   memset(&ipv6_outer_header,0,sizeof(ipv6_header_iht));
   memset(&ipv6_inner_header,0,sizeof(ipv6_header_iht));
   
   // Set hop limit to the default in-network value as this packet is being
   // sent from upper layer. This is done here as send_internal() is used by
   // forwarding of packets as well which carry a hlim. This value is required
   // to be set to a value as the following function can decrement it.
   ipv6_outer_header.hop_limit     = IPHC_DEFAULT_HOP_LIMIT;
   
   // create the RPL hop-by-hop option

   forwarding_createRplOption(
      &rpl_option,      // rpl_option to fill in
      0x00              // flags
   );

   packetfunctions_ip128bToMac64b(&(msg->l3_destinationAdd),&temp_dest_prefix,&temp_dest_mac64b);
   //xv poipoi -- get the src prefix as well
   packetfunctions_ip128bToMac64b(&(msg->l3_sourceAdd),&temp_src_prefix,&temp_src_mac64b);
   //XV -poipoi we want to check if the source address prefix is the same as destination prefix
   if (packetfunctions_sameAddress(&temp_dest_prefix,&temp_src_prefix)) {
         // same prefix use 64B address
         sam = IPHC_SAM_ELIDED;
         dam = IPHC_DAM_64B;
         p_dest = &temp_dest_mac64b;      
         p_src  = NULL; 
   } else {
     //not the same prefix. so the packet travels to another network
     //check if this is a source routing pkt. in case it is then the DAM is elided as it is in the SrcRouting header.
     if (packetfunctions_isBroadcastMulticast(&(msg->l3_destinationAdd))==FALSE){
          sam = IPHC_SAM_ELIDED;
          dam = IPHC_DAM_128B;
          p_dest = &(msg->l3_destinationAdd);
          p_src = NULL;
     } else {
         // this is DIO, source address elided, multicast bit is set
          sam = IPHC_SAM_ELIDED;
          m   = IPHC_M_YES;
          dam = IPHC_DAM_ELIDED;
          p_dest = &(msg->l3_destinationAdd);
          p_src = &(msg->l3_sourceAdd);
     }
   }

   
   // inner header is required only when the destination address is NOT broadcast address
   if (packetfunctions_isBroadcastMulticast(&(msg->l3_destinationAdd)) == FALSE) {
       //IPHC inner header and NHC IPv6 header will be added at here
        iphc_prependIPv6Header(msg,
                    IPHC_TF_ELIDED,
                    flow_label, // value_flowlabel
                    IPHC_NH_INLINE,
                    msg->l4_protocol, 
                    IPHC_HLIM_64,
                    ipv6_outer_header.hop_limit,
                    IPHC_CID_NO,
                    IPHC_SAC_STATELESS,
                    sam,
                    m,
                    IPHC_DAC_STATELESS,
                    dam,
                    p_dest,
                    p_src,            
                    PCKTSEND  
                    );
       // both of them are compressed
       ipv6_outer_header.next_header_compressed = TRUE;
   }

   return forwarding_send_internal_RoutingTable(
      msg,
      &ipv6_outer_header,
      &ipv6_inner_header,
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
      ipv6_header_iht*       ipv6_outer_header,
      ipv6_header_iht*       ipv6_inner_header,
      ipv6_hopbyhop_iht*     ipv6_hop_header,
      rpl_option_ht*         rpl_option
   ) {
   uint8_t flags;
   uint16_t senderRank;
   
   // take ownership
   msg->owner                     = COMPONENT_FORWARDING;
   
   // determine L4 protocol
   if (ipv6_outer_header->next_header==IANA_IPv6HOPOPT){
      // get information from ipv6_hop_header
      
      msg->l4_protocol            = ipv6_hop_header->nextHeader;
      msg->l4_protocol_compressed = ipv6_hop_header->next_header_compressed;
   } else {
      // get information from ipv6_header
      
      msg->l4_protocol            = ipv6_outer_header->next_header;
      msg->l4_protocol_compressed = ipv6_outer_header->next_header_compressed;
   }
   
   // populate packets metadata with L3 information
   memcpy(&(msg->l3_destinationAdd),&ipv6_inner_header->dest,sizeof(open_addr_t));
   memcpy(&(msg->l3_sourceAdd),     &ipv6_outer_header->src, sizeof(open_addr_t));

   if (
         (
            idmanager_isMyAddress(&(msg->l3_destinationAdd))
            ||
            packetfunctions_isBroadcastMulticast(&(msg->l3_destinationAdd))
         )
         &&
         ipv6_outer_header->next_header!=IANA_IPv6ROUTE
   ) {
      // this packet is for me, no source routing header.
      if (packetfunctions_isBroadcastMulticast(&(msg->l3_destinationAdd))==FALSE) {
          msg->l4_protocol = ipv6_inner_header->next_header;
          msg->l4_protocol_compressed = ipv6_inner_header->next_header_compressed;
          // toss iphc inner header
          packetfunctions_tossHeader(msg,ipv6_inner_header->header_length);
      }
          
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
      
      if (ipv6_outer_header->next_header!=IANA_IPv6ROUTE) {
         // no source routing header present
         //check if flow label rpl header

 	     flags = rpl_option->flags;
  	     senderRank = rpl_option->senderRank;

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
            // loop detected
            // set flag
       	    rpl_option->flags |= R_FLAG;

            // log error
            openserial_printError(
               COMPONENT_FORWARDING,
               ERR_LOOP_DETECTED,
               (errorparameter_t) senderRank,
               (errorparameter_t) neighbors_getMyDAGrank()
            );
         }
         

         forwarding_createRplOption(rpl_option, rpl_option->flags);
         // resend as if from upper layer
         if (
               forwarding_send_internal_RoutingTable(
                  msg,
                  ipv6_outer_header,
                  ipv6_inner_header,
                  rpl_option,
                  &(ipv6_outer_header->flow_label),
                  PCKTFORWARD 
               )==E_FAIL
            ) {
            openqueue_freePacketBuffer(msg);
         }
      } else {
         // source routing header present
         
         if (forwarding_send_internal_SourceRouting(msg,ipv6_outer_header,ipv6_inner_header)==E_FAIL) {
            
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
      ipv6_header_iht*       ipv6_outer_header,
      ipv6_header_iht*       ipv6_inner_header,
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
      ipv6_outer_header,
      ipv6_inner_header,
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
      ipv6_header_iht*  ipv6_outer_header,
      ipv6_header_iht*  ipv6_inner_header
   ) {
   uint8_t              temp_8b;
   uint8_t              size;
   uint8_t              sizeUnit;
   uint8_t              hlen = 0;
   open_addr_t*         nexthop;
   rpl_option_ht        rpl_option;
   
   memset(&rpl_option,0,sizeof(rpl_option_ht));
   nexthop = NULL;

   temp_8b = *((uint8_t*)(msg->payload)+ hlen);
   size = temp_8b & RH3_6LOTH_SIXE_MASK;
   hlen += 1;
   
   temp_8b = *((uint8_t*)(msg->payload)+ hlen);
   hlen += 1;
   memcpy(nexthop,&(ipv6_inner_header->dest),sizeof(open_addr_t));
   switch(temp_8b){
   case 0:
       sizeUnit = 1;
       switch(nexthop->type){
       case ADDR_64B:  
           nexthop->addr_64b[7]=*((uint8_t*)(msg->payload)+ hlen);
           break;
       case ADDR_128B:
           nexthop->addr_128b[15]=*((uint8_t*)(msg->payload)+ hlen);
           break;
       default:
           // log error
           printf("Wrong nexthop type %d \n",nexthop->type);
       } 
       break;
   case 1:
       sizeUnit = 2;
       switch(nexthop->type){
       case ADDR_64B:  
           memcpy(&(nexthop->addr_64b[6]),(uint8_t*)(msg->payload+hlen),sizeUnit);
           break;
       case ADDR_128B:
           memcpy(&(nexthop->addr_128b[14]),(uint8_t*)(msg->payload+hlen),sizeUnit);
           break;
       default:
           // log error
           printf("Wrong nexthop type %d \n",nexthop->type);
       }
       break;
   case 2:
       sizeUnit = 4;
       switch(nexthop->type){
       case ADDR_64B:  
           memcpy(&(nexthop->addr_64b[4]),(uint8_t*)(msg->payload+hlen),sizeUnit);
           break;
       case ADDR_128B:
           memcpy(&(nexthop->addr_128b[12]),(uint8_t*)(msg->payload+hlen),sizeUnit);
           break;
       default:
           // log error
           printf("Wrong nexthop type %d \n",nexthop->type);
       }
       break;
   case 3:
       sizeUnit = 8;
       switch(nexthop->type){
       case ADDR_64B:  
           memcpy(&(nexthop->addr_64b[0]),(uint8_t*)(msg->payload+hlen),sizeUnit);
           break;
       case ADDR_128B:
           memcpy(&(nexthop->addr_128b[8]),(uint8_t*)(msg->payload+hlen),sizeUnit);
           break;
       default:
           // log error
           printf("Wrong nexthop type %d \n",nexthop->type);
       }
       break;
   case 4:
       sizeUnit = 16;
       packetfunctions_readAddress(((uint8_t*)(msg->payload+hlen)),ADDR_128B,nexthop,OW_BIG_ENDIAN);
       break;
   }
   hlen += sizeUnit;
   if (packetfunctions_sameAddress(nexthop,idmanager_getMyID(nexthop->type))){
       size--;
       if (size!=0){
           *((uint8_t*)(msg->payload+hlen-2)) = CRITICAL_6LORH | size;
           *((uint8_t*)(msg->payload+hlen-1)) = temp_8b;
           packetfunctions_tossHeader(msg,sizeUnit);
           
           switch(nexthop->type){
           case ADDR_64B:
               memcpy(&(nexthop->addr_64b[8-sizeUnit]),(uint8_t*)(msg->payload+hlen),sizeUnit);
               memcpy(&msg->l2_nextORpreviousHop,nexthop,sizeof(open_addr_t));
               break;
           case ADDR_128B:
               memcpy(&(nexthop->addr_64b[16-sizeUnit]),(uint8_t*)(msg->payload+hlen),sizeUnit);
               memcpy(&msg->l2_nextORpreviousHop,nexthop,sizeof(open_addr_t));
               msg->l2_nextORpreviousHop.type = ADDR_64B;
               break;
           }
       } else{
           packetfunctions_tossHeader(msg,hlen);
           hlen = 0;
           temp_8b = *(uint8_t*)(msg->payload);
           hlen += 1;
           if(temp_8b & FORMAT_6LORH_MASK == CRITICAL_6LORH){
               size = temp_8b & RH3_6LOTH_SIXE_MASK;
               temp_8b = *(uint8_t*)(msg->payload+hlen);
               hlen += 1;
               if(temp_8b <= RH3_6LOTH_TYPE_4){
                   // there is another RH3 6LoRH
                   switch(temp_8b){
                   case 0:
                       sizeUnit=1;
                       break;
                   case 1:
                       sizeUnit=2;
                       break;
                   case 2:
                       sizeUnit=4;
                       break;
                   case 3:
                       sizeUnit=8;
                       break;
                   case 4:
                       sizeUnit=16;
                       break;
                   }
                   switch(nexthop->type){
                   case ADDR_64B:
                       memcpy(&(nexthop->addr_64b[8-sizeUnit]),(uint8_t*)(msg->payload+hlen),sizeUnit);
                       memcpy(&msg->l2_nextORpreviousHop,nexthop,sizeof(open_addr_t));
                       break;
                   case ADDR_128B:
                       memcpy(&(nexthop->addr_64b[16-sizeUnit]),(uint8_t*)(msg->payload+hlen),sizeUnit);
                       memcpy(&msg->l2_nextORpreviousHop,nexthop,sizeof(open_addr_t));
                       msg->l2_nextORpreviousHop.type = ADDR_64B;
                       break;
                   }
               } else{
                   // some other 6lorh
               }
           } else {
               // there is no RH3 anymore, next hop is destination
               packetfunctions_ip128bToMac64b(&ipv6_inner_header->dest,nexthop,&msg->l2_nextORpreviousHop);
           }
       }
       memcpy(&msg->l3_destinationAdd,&ipv6_inner_header->dest,sizeof(open_addr_t));
       memcpy(&msg->l3_sourceAdd,&ipv6_outer_header->src,sizeof(open_addr_t));
   } else {
       // log error
       printf("wrong address in RH3!\n");
   }
   
   // send to next lower layer
   return iphc_sendFromForwarding(
      msg,
      ipv6_outer_header,
      ipv6_inner_header,
      &rpl_option,
      &ipv6_outer_header->flow_label,
      PCKTFORWARD
   );
}


/**
\brief Create a RPL option.

\param[out] rpl_option A pointer to the structure to fill in.
\param[in]  flags      The flags to indicate in the RPL option.
*/
void forwarding_createRplOption(rpl_option_ht* rpl_option, uint8_t flags) {
    uint8_t I,K;
    rpl_option->optionType         = RPL_HOPBYHOP_HEADER_OPTION_TYPE;
    rpl_option->rplInstanceID      = icmpv6rpl_getRPLIntanceID();
    rpl_option->senderRank         = neighbors_getMyDAGrank();
   
    if (rpl_option->rplInstanceID == 0){
       I = 1;
    } else {
       I = 0;
    }
    
    if (rpl_option->senderRank & 0x00FF == 0){
        K = 1;
    } else {
        K = 0;
    }
    
    rpl_option->flags              = flags | (I<<1) | K;
}

