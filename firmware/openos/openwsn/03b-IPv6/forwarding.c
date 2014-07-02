#include "openwsn.h"
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

owerror_t forwarding_send_internal_RoutingTable(OpenQueueEntry_t *msg,  ipv6_header_iht ipv6_header, rpl_hopoption_ht hopbyhop_header, uint8_t fw_SendOrfw_Rcv);
void    forwarding_getNextHop_RoutingTable(open_addr_t* destination, open_addr_t* addressToWrite);
owerror_t forwarding_send_internal_SourceRouting(OpenQueueEntry_t *msg, ipv6_header_iht ipv6_header);
void forwarding_createHopByHopOption(rpl_hopoption_ht *hopbyhop_opt, uint8_t flags);

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
   ipv6_header_iht ipv6_header;
   rpl_hopoption_ht hopbyhop_opt;

   open_addr_t*    myprefix;
   open_addr_t*    myadd64;
   
   // take ownership
   msg->owner                = COMPONENT_FORWARDING; 
   
   // retrieve my prefix and EUI64
   myprefix                  = idmanager_getMyID(ADDR_PREFIX);
   myadd64                   = idmanager_getMyID(ADDR_64B);
   
   // set source address (to me)
   msg->l3_sourceAdd.type=ADDR_128B;
   memcpy(&(msg->l3_sourceAdd.addr_128b[0]),myprefix->prefix,8);
   memcpy(&(msg->l3_sourceAdd.addr_128b[8]),myadd64->addr_64b,8);
   
   // initialize IPv6 header
   memset(&ipv6_header,0,sizeof(ipv6_header_iht));
   
   //set hop limit to the default in-network value as this packet is being sent from upper layer.
   //this is done here as send_internal is used by forwarding of packets as well which 
   //carry a hlim. This value is required to be set to a value as the following function can decrement it
   ipv6_header.hop_limit     = IPHC_DEFAULT_HOP_LIMIT;
    //create hop  by hop option
   forwarding_createHopByHopOption(&hopbyhop_opt, 0x00); //flags are 0x00 -- TODO check and define macro   
   
   return forwarding_send_internal_RoutingTable(msg,ipv6_header,hopbyhop_opt,PCKTSEND);
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
      // that is a packet I relayed
      
      // free packet
      openqueue_freePacketBuffer(msg);
   } else {
      // that is a packet I created
      
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
            /*openserial_printCritical(COMPONENT_FORWARDING,ERR_WRONG_TRAN_PROTOCOL,
                                  (errorparameter_t)msg->l4_protocol,
                                  (errorparameter_t)0);*/
            // free packet
            openqueue_freePacketBuffer(msg);
      }
   }
}

/**
\brief Indicates a packet was received.

\param[in,out] msg         The packet just sent.
\param[in]     ipv6_header The information contained in the 6LoWPAN header.
*/
void forwarding_receive(OpenQueueEntry_t* msg, 
                        ipv6_header_iht ipv6_header, 
                        ipv6_hopbyhop_ht ipv6_hop_header, 
                        rpl_hopoption_ht hop_by_hop_option) {
                          
   uint8_t temp_flags;
   
   // take ownership
   msg->owner                  = COMPONENT_FORWARDING;
   
   
   //contains a 
 if (ipv6_header.next_header==IANA_IPv6HOPOPT){
      // populate packets metadata with l4 information
      msg->l4_protocol            = ipv6_hop_header.nextHeader;
      msg->l4_protocol_compressed = ipv6_hop_header.next_header_compressed; // rfc 6282   
      
      //process HOP BY HOP header
      
      
   }else{
      msg->l4_protocol            = ipv6_header.next_header;
      msg->l4_protocol_compressed = ipv6_header.next_header_compressed; // rfc 6282   
   }
   
     // populate packets metadata with l3 information
   memcpy(&(msg->l3_destinationAdd),&ipv6_header.dest,sizeof(open_addr_t));
   memcpy(&(msg->l3_sourceAdd),     &ipv6_header.src, sizeof(open_addr_t));
   
   
   if (
          (
             idmanager_isMyAddress(&ipv6_header.dest)
             ||
             packetfunctions_isBroadcastMulticast(&ipv6_header.dest)
          )
          &&
          //ipv6 header - next header will be IANA_IPv6HOPOPT or IANA_IPv6ROUTE
          ipv6_header.next_header!=IANA_IPv6ROUTE
       ) {
      // this packet is for me, but no src routing header.
      
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
            /*openserial_printError(COMPONENT_FORWARDING,ERR_WRONG_TRAN_PROTOCOL,
                                  (errorparameter_t)msg->l4_protocol,
                                  (errorparameter_t)1);*/
            openqueue_freePacketBuffer(msg);
      }
   } else {
      // this packet is not for me: relay
   
      // change the creator of the packet
      msg->creator = COMPONENT_FORWARDING;
      //START OF TELEMATICS CODE
      msg->l2_keySource = *(idmanager_getMyID(ADDR_64B));
      //END OF TELEMATICS CODE
      
      if (ipv6_header.next_header!=IANA_IPv6ROUTE) {
         // no source routing header present
          
          //process HOP bY HOP header
          temp_flags = hop_by_hop_option.flags;
          if ((temp_flags & O_FLAG)!=0){
            //error wrong direction
            //what todo? print the error
            /*openserial_printError(COMPONENT_FORWARDING,ERR_WRONG_DIRECTION,
                                  (errorparameter_t)1,
                                  (errorparameter_t)1);*/
          }
          if (hop_by_hop_option.senderRank < neighbors_getMyDAGrank()){
            //wrong rank relation.. loop detected
            temp_flags |= R_FLAG; //set r flag.
            /*openserial_printError(COMPONENT_FORWARDING,ERR_LOOP_DETECTED,
                                  (errorparameter_t) hop_by_hop_option.senderRank,
                                  (errorparameter_t) neighbors_getMyDAGrank());*/
          }
            
          //O flag should always be 0 as this is upstream route.
          
          forwarding_createHopByHopOption(&hop_by_hop_option, temp_flags); 
   
        
         // resend as if from upper layer 
         if (forwarding_send_internal_RoutingTable(msg, ipv6_header,hop_by_hop_option,PCKTFORWARD)==E_FAIL) {
            openqueue_freePacketBuffer(msg);
         }
      } else {
         // source routing header present 
          if (forwarding_send_internal_SourceRouting(msg, ipv6_header)==E_FAIL) {
            //already freed by send_internal if it fails
            //todo change error type to another that says src_route failure.
           /*openserial_printError(COMPONENT_FORWARDING,ERR_INVALID_FWDMODE,
                                  (errorparameter_t)0,
                                  (errorparameter_t)0);*/
         }
      }
   }
}



/**
\brief Send a packet using the routing table to find the next hop.

\param[in,out] msg             The packet to send.
\param[in]     ipv6_header     The packet's IPv6 header.
\param[in]     fw_SendOrfw_Rcv The packet is originating from this mote
   (PCKTSEND), or forwarded (PCKTFORWARD).
*/
owerror_t forwarding_send_internal_RoutingTable(OpenQueueEntry_t* msg, ipv6_header_iht ipv6_header, rpl_hopoption_ht hopbyhop_opt, uint8_t fw_SendOrfw_Rcv) {
   
   // retrieve the next hop from the routing table
   forwarding_getNextHop_RoutingTable(&(msg->l3_destinationAdd),&(msg->l2_nextORpreviousHop));
   if (msg->l2_nextORpreviousHop.type==ADDR_NONE) {
      /*openserial_printError(COMPONENT_FORWARDING,ERR_NO_NEXTHOP,
                            (errorparameter_t)0,
                            (errorparameter_t)0);*/
      return E_FAIL;
   }
   
   // send to next lower layer
   return iphc_sendFromForwarding(msg, ipv6_header, &hopbyhop_opt,fw_SendOrfw_Rcv);
}

/**
\brief Send a packet using the source rout to find the next hop.

\note This is always called for packets being forwarded.

How to process the routing header is detailed in
http://tools.ietf.org/html/rfc6554#page-9.

\param[in,out] msg             The packet to send.
\param[in]     ipv6_header     The packet's IPv6 header.
*/
owerror_t forwarding_send_internal_SourceRouting(OpenQueueEntry_t *msg, ipv6_header_iht ipv6_header) {
   uint8_t         local_CmprE;
   uint8_t         local_CmprI;
   uint8_t         numAddr;
   uint8_t         hlen;
   uint8_t         addressposition;
   uint8_t*        runningPointer;
   uint8_t         octetsAddressSize;
   open_addr_t*    prefix;
   rpl_routing_ht* rpl_routing_hdr;
  
   rpl_hopoption_ht hopbyhop_opt; 
   
   memset(&hopbyhop_opt,0,sizeof(rpl_hopoption_ht));//reset everything
   
   // get my prefix
   prefix               = idmanager_getMyID(ADDR_PREFIX);
   
   // cast packet to RPL routing header
   rpl_routing_hdr      = (rpl_routing_ht*)(msg->payload);
   
   // point behind the RPL routing header
   runningPointer       = (msg->payload)+sizeof(rpl_routing_ht);
   
   // retrieve CmprE and CmprI
   
   /*CmprE 4-bit unsigned integer. Number of prefix octets
     from the last segment (i.e., segment n) that are
     elided. For example, an SRH carrying a full IPv6
     address in Addressesn sets CmprE to 0.*/
   
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
            /*openserial_printError(COMPONENT_FORWARDING,ERR_WRONG_TRAN_PROTOCOL,
                               (errorparameter_t)msg->l4_protocol,
                               (errorparameter_t)1);*/
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
         
         /*openserial_printError(COMPONENT_FORWARDING,ERR_NO_NEXTHOP,
                            (errorparameter_t)0,
                            (errorparameter_t)0);*/
         openqueue_freePacketBuffer(msg);
         return E_FAIL;
      
      } else {
         
         // decrement number of segments left
         rpl_routing_hdr->SegmentsLeft--;
         
         // find next hop address in source route
         //addressposition    = numAddr-(rpl_routing_hdr->SegmentsLeft);
         addressposition    = rpl_routing_hdr->SegmentsLeft;
         // how many octets have the address? 
         if (rpl_routing_hdr->SegmentsLeft > 1){
              octetsAddressSize = LENGTH_ADDR128b - local_CmprI; //max addr length - number of prefix octets that are elided in the internal route elements
         }else{
              octetsAddressSize = LENGTH_ADDR128b - local_CmprE; //max addr length - number of prefix octets that are elided in the internal route elements
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
               //poipoi xv
               //any other value is not supported by now.
               /*openserial_printError(COMPONENT_FORWARDING,ERR_INVALID_PARAM,
                               (errorparameter_t)1,
                               (errorparameter_t)0);*/
               openqueue_freePacketBuffer(msg);
               return E_FAIL;
         }
      }
   }
   
   // send to next lower layer
   return iphc_sendFromForwarding(msg, ipv6_header,&hopbyhop_opt, PCKTFORWARD);
}

/**
\brief Retrieve the next hop's address from routing table.

\param[in]  destination128b  Final IPv6 destination address.
\param[out] addressToWrite64b Location to write the EUI64 of next hop to.
*/
void forwarding_getNextHop_RoutingTable(open_addr_t* destination128b, open_addr_t* addressToWrite64b) {
   uint8_t i;
   open_addr_t temp_prefix64btoWrite;
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
/*
 * HOP BY HOP HEADER OPTION
 */


void forwarding_createHopByHopOption(rpl_hopoption_ht *hopbyhop_opt, uint8_t flags) {   
        //set the rpl hop by hop header
	hopbyhop_opt->optionType = RPL_HOPBYHOP_HEADER_OPTION_TYPE;
	//8-bit field indicating the length of the option, in
	//octets, excluding the Option Type and Opt Data Len fields.
	hopbyhop_opt->optionLen = 0x04; //4-bytes, flags+instanceID+senderrank - no sub-tlvs
	hopbyhop_opt->flags = flags;
	hopbyhop_opt->rplInstanceID = icmpv6rpl_getRPLIntanceID(); //getit..
	hopbyhop_opt->senderRank = neighbors_getMyDAGrank(); //TODO change to DAGRAnk(rank) instead of rank
}
