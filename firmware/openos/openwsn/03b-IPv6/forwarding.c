#include "openwsn.h"
#include "forwarding.h"
#include "iphc.h"
#include "openqueue.h"
#include "openserial.h"
#include "idmanager.h"
#include "packetfunctions.h"
#include "neighbors.h"
#include "icmpv6.h"
#include "openudp.h"
#include "opentcp.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

void    getNextHop(open_addr_t* destination, open_addr_t* addressToWrite);
error_t fowarding_send_internal(OpenQueueEntry_t *msg,  ipv6_header_iht ipv6_header, uint8_t fw_SendOrfw_Rcv); //>>>>>> diodio
error_t fowarding_send_internal_SourceRouting(OpenQueueEntry_t *msg, ipv6_header_iht ipv6_header);

//=========================== public ==========================================

void forwarding_init() {
}

//send from THIS node.
error_t forwarding_send(OpenQueueEntry_t *msg) { 
  ipv6_header_iht ipv6_header;
  open_addr_t* myprefix=idmanager_getMyID(ADDR_PREFIX);
  open_addr_t* myadd64=idmanager_getMyID(ADDR_64B);
  
  msg->owner = COMPONENT_FORWARDING; 
  //poipoi xv set src address as me.
  
  memcpy(&(msg->l3_sourceAdd.addr_128b[0]),myprefix->prefix,8);
  memcpy(&(msg->l3_sourceAdd.addr_128b[8]),myadd64->addr_64b,8);
  msg->l3_sourceAdd.type=ADDR_128B;
  
  memset(&ipv6_header,0,sizeof(ipv6_header_iht));
  return fowarding_send_internal(msg,ipv6_header,PCKTSEND);
}

void forwarding_sendDone(OpenQueueEntry_t* msg, error_t error) {
  msg->owner = COMPONENT_FORWARDING;
  if (msg->creator==COMPONENT_RADIO || msg->creator==COMPONENT_FORWARDING) {
    //that was a packet I had relayed
    openqueue_freePacketBuffer(msg);
  } else {
    //that was a packet coming from above
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
      openserial_printError(COMPONENT_FORWARDING,ERR_WRONG_TRAN_PROTOCOL,
                            (errorparameter_t)msg->l4_protocol,
                            (errorparameter_t)0);
      // free the corresponding packet buffer
      openqueue_freePacketBuffer(msg);
    }
  }
}

void forwarding_receive(OpenQueueEntry_t* msg, ipv6_header_iht ipv6_header) {
  msg->owner = COMPONENT_FORWARDING;
  msg->l4_protocol            = ipv6_header.next_header;
  msg->l4_protocol_compressed = ipv6_header.next_header_compressed;
  if ((idmanager_isMyAddress(&ipv6_header.dest) 
       || packetfunctions_isBroadcastMulticast(&ipv6_header.dest))
      && ipv6_header.next_header!=SourceFWNxtHdr) {
        //for me and not having src routing header destination address its me.
        memcpy(&(msg->l3_destinationAdd),&ipv6_header.dest,sizeof(open_addr_t));
        memcpy(&(msg->l3_sourceAdd),&ipv6_header.src,sizeof(open_addr_t));
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
          openserial_printError(COMPONENT_FORWARDING,ERR_WRONG_TRAN_PROTOCOL,
                                (errorparameter_t)msg->l4_protocol,
                                (errorparameter_t)1);
        }
      } else { //relay
        memcpy(&(msg->l3_destinationAdd),&ipv6_header.dest,sizeof(open_addr_t));
        //because initially contains source
        memcpy(&(msg->l3_sourceAdd),&ipv6_header.src,sizeof(open_addr_t)); 
        // change the creator to this components (should have been MAC)
        msg->creator = COMPONENT_FORWARDING;
        if(ipv6_header.next_header !=SourceFWNxtHdr) 
        {
          // resend as if from upper layer 
          if (fowarding_send_internal(msg, ipv6_header,PCKTFORWARD)==E_FAIL) {
            openqueue_freePacketBuffer(msg);
          }
        }
        else
        {
          // source route
          if (fowarding_send_internal_SourceRouting(msg, ipv6_header)==E_FAIL) {
            openqueue_freePacketBuffer(msg);
          }
        }
      }
}

//=========================== private =========================================

error_t fowarding_send_internal(OpenQueueEntry_t *msg, ipv6_header_iht ipv6_header, uint8_t fw_SendOrfw_Rcv) {
  getNextHop(&(msg->l3_destinationAdd),&(msg->l2_nextORpreviousHop));
  if (msg->l2_nextORpreviousHop.type==ADDR_NONE) {
    openserial_printError(COMPONENT_FORWARDING,ERR_NO_NEXTHOP,
                          (errorparameter_t)0,
                          (errorparameter_t)0);
    return E_FAIL;
  }
  return iphc_sendFromForwarding(msg, ipv6_header,fw_SendOrfw_Rcv);
}

error_t fowarding_send_internal_SourceRouting(OpenQueueEntry_t *msg, ipv6_header_iht ipv6_header) {
  // It has to be forwarded to dest. so, next hop should be extracted from the message.
  uint8_t local_CmprE;
  uint8_t local_CmprI,numAddr,hlen;
  uint8_t addressposition;
  uint8_t* runningPointer;
  uint8_t octetsAddressSize;
  ipv6_Source_Routing_Header_t * ipv6_Source_Routing_Header;
  
  open_addr_t* prefix=idmanager_getMyID(ADDR_PREFIX);
 
  ipv6_Source_Routing_Header=(ipv6_Source_Routing_Header_t*)(msg->payload);
  
  runningPointer=(msg->payload) + sizeof(ipv6_Source_Routing_Header_t);
  
  // getting local_CmprE and CmprI;
  local_CmprE=ipv6_Source_Routing_Header->CmprICmprE & 0xf;
  local_CmprI= ipv6_Source_Routing_Header->CmprICmprE & 0xf0;
  //local_CmprI>>4; // shifting it by 4.
  local_CmprI=local_CmprI>>4; // shifting it by 4.
  
  //see processing header algorithm in RFC6554 page 9
    
  numAddr=(((ipv6_Source_Routing_Header->HdrExtLen*8)-ipv6_Source_Routing_Header->PadRes -(16-local_CmprE))/(16-local_CmprI))+1;
  
  if(ipv6_Source_Routing_Header->SegmentsLeft==0){
    //we are there!..
    //process the next header in the pkt.. i.e push stack up..
    msg->l4_protocol=ipv6_Source_Routing_Header->nextHeader;
    hlen=ipv6_Source_Routing_Header->HdrExtLen;
    //toss header
    packetfunctions_tossHeader(msg,sizeof(ipv6_Source_Routing_Header_t));
    //toss list of addresses.
    if(local_CmprE==0)
    {
      octetsAddressSize=2;
      //remove 
      packetfunctions_tossHeader(msg,octetsAddressSize*hlen);   
    }
    else if(local_CmprE==8)
    {
      octetsAddressSize=8;
      packetfunctions_tossHeader(msg,octetsAddressSize*hlen);
    }
    else if(local_CmprE==2)
    {
      octetsAddressSize=16;
      packetfunctions_tossHeader(msg,octetsAddressSize*hlen);
    }
    else
    {
      msg->l2_nextORpreviousHop.type = ADDR_NONE;
      //error!
      while(1);
    }
    
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
      openserial_printError(COMPONENT_FORWARDING,ERR_WRONG_TRAN_PROTOCOL,
                            (errorparameter_t)msg->l4_protocol,
                            (errorparameter_t)1);
    }
    
    return E_SUCCESS;
  }else{    
    if(ipv6_Source_Routing_Header->SegmentsLeft>numAddr){
      //not good.. error. 
      //poipoi xv :
      //send and ICMPv6 parameter problem, code 0, to the src address 
      //then discard the packet.  //TODO
      while (1);
    }else{
      //still hops remaining 
      ipv6_Source_Routing_Header->SegmentsLeft--;
      //find the address in the vector.
      addressposition=numAddr-(ipv6_Source_Routing_Header->SegmentsLeft);
      
      if(local_CmprE==0)
      {
        msg->l2_nextORpreviousHop.type = ADDR_16B;
        msg->l3_destinationAdd.type = ADDR_16B;
        octetsAddressSize=2;
        memcpy(&(msg->l2_nextORpreviousHop.addr_16b),runningPointer+((addressposition-1)*octetsAddressSize),octetsAddressSize);
        memcpy(&(msg->l3_destinationAdd.addr_16b),runningPointer+((addressposition-1)*octetsAddressSize),octetsAddressSize);
      }
      else if(local_CmprE==8)
      {
        msg->l2_nextORpreviousHop.type = ADDR_64B;
        msg->l3_destinationAdd.type = ADDR_128B;
        octetsAddressSize=8;
        memcpy(&(msg->l2_nextORpreviousHop.addr_64b),runningPointer+((addressposition-1)*octetsAddressSize),octetsAddressSize);
     
        memcpy(&(msg->l3_destinationAdd.addr_128b[0]),prefix->prefix,8);
        memcpy(&(msg->l3_destinationAdd.addr_128b[8]),runningPointer+((addressposition-1)*octetsAddressSize),octetsAddressSize);
      }
      else if(local_CmprE==2)
      {
        msg->l2_nextORpreviousHop.type = ADDR_128B;
        msg->l3_destinationAdd.type = ADDR_128B;
        
        octetsAddressSize=16;
        memcpy(&(msg->l2_nextORpreviousHop.addr_128b),runningPointer+((addressposition-1)*octetsAddressSize),octetsAddressSize);
        memcpy(&(msg->l3_destinationAdd.addr_128b),runningPointer+((addressposition-1)*octetsAddressSize),octetsAddressSize);
      }
      else
      {
        msg->l2_nextORpreviousHop.type = ADDR_NONE;
        //error!
        while(1);
      }
    }
  }
  
  if (msg->l2_nextORpreviousHop.type==ADDR_NONE) {
    openserial_printError(COMPONENT_FORWARDING,ERR_NO_NEXTHOP,
                          (errorparameter_t)0,
                          (errorparameter_t)0);
    return E_FAIL;
  }
  return iphc_sendFromForwarding(msg, ipv6_header,PCKTFORWARD);
}

void getNextHop(open_addr_t* destination128b, open_addr_t* addressToWrite64b) {
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