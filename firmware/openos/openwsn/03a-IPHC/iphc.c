#include "openwsn.h"
#include "iphc.h"
#include "packetfunctions.h"
#include "idmanager.h"
#include "openserial.h"
#include "res.h"
#include "forwarding.h"
#include "neighbors.h"
#include "openbridge.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

owerror_t prependIPv6Header(
   OpenQueueEntry_t*    msg,
   uint8_t              tf,
   uint32_t             value_flowLabel,
   bool                 nh,
   uint8_t              value_nextHeader,
   uint8_t              hlim,
   uint8_t              value_hopLimit,
   bool                 cid,
   bool                 sac,
   uint8_t              sam,
   bool                 m,
   bool                 dac,
   uint8_t              dam,
   open_addr_t*         value_dest,
   open_addr_t*         value_src,
   uint8_t              fw_SendOrfw_Rcv
);
ipv6_header_iht retrieveIPv6Header(OpenQueueEntry_t* msg);
//hop by hop header
void prependIPv6HopByHopHeader(OpenQueueEntry_t* msg,uint8_t nextheader, bool nh, rpl_hopoption_ht *hopbyhop_option);
void retrieveIPv6HopByHopHeader(OpenQueueEntry_t* msg, ipv6_hopbyhop_ht *hopbyhop_header, rpl_hopoption_ht *rpl_option);
//=========================== public ==========================================

void iphc_init() {
}

//send from upper layer: I need to add 6LoWPAN header
owerror_t iphc_sendFromForwarding(OpenQueueEntry_t *msg, ipv6_header_iht ipv6_header, rpl_hopoption_ht *hopbyhop_option, uint8_t fw_SendOrfw_Rcv) {
   open_addr_t  temp_dest_prefix;
   open_addr_t  temp_dest_mac64b;
   open_addr_t* p_dest;
   open_addr_t* p_src;  
   open_addr_t  temp_src_prefix;
   open_addr_t  temp_src_mac64b; 
   uint8_t      sam;
   uint8_t      dam;
   uint8_t      nh;
   uint8_t      next_header;
   //option header
  
   // take ownership over the packet
   msg->owner = COMPONENT_IPHC;
   
   // by default, the "next header" field is carried inline
   nh=IPHC_NH_INLINE;
   
   // error checking
   if (idmanager_getIsBridge()==TRUE &&
      packetfunctions_isAllRoutersMulticast(&(msg->l3_destinationAdd))==FALSE) {
      /*openserial_printCritical(COMPONENT_IPHC,ERR_BRIDGE_MISMATCH,
                            (errorparameter_t)0,
                            (errorparameter_t)0);*/
      return E_FAIL;
   }
   
   //discard the packet.. hop limit reached.
   if (ipv6_header.hop_limit==0) {
      /*openserial_printError(COMPONENT_IPHC,ERR_HOP_LIMIT_REACHED,
                            (errorparameter_t)0,
                            (errorparameter_t)0);*/
     return E_FAIL;
   }
   
   packetfunctions_ip128bToMac64b(&(msg->l3_destinationAdd),&temp_dest_prefix,&temp_dest_mac64b);
   //xv poipoi -- get the src prefix as well
   packetfunctions_ip128bToMac64b(&(msg->l3_sourceAdd),&temp_src_prefix,&temp_src_mac64b);
   //XV -poipoi we want to check if the source address prefix is the same as destination prefix
   if (packetfunctions_sameAddress(&temp_dest_prefix,&temp_src_prefix)) {   
   //dest and src on same prefix
      if (neighbors_isStableNeighbor(&(msg->l3_destinationAdd))) {
         //if direct neighbors, MAC nextHop and IP destination indicate same node
         //the source can be ME or another who I am relaying from. If its me then SAM is elided,
         //if not SAM is 64b address 
        if (fw_SendOrfw_Rcv==PCKTFORWARD){
            sam = IPHC_SAM_64B;    //case forwarding a packet
            p_src = &temp_src_mac64b;
            //poipoi xv forcing elided addresses on src routing, this needs to be fixed so any type of address should be supported supported.
        } else if (fw_SendOrfw_Rcv==PCKTSEND){
            sam = IPHC_SAM_ELIDED;
            p_src = NULL;
        } else {
           /*openserial_printCritical(COMPONENT_IPHC,ERR_INVALID_FWDMODE,
                            (errorparameter_t)0,
                            (errorparameter_t)0);*/
        } 
        dam = IPHC_DAM_ELIDED;
        p_dest = NULL;     
      } else {
         //else, not a direct neighbour use 64B address
         sam = IPHC_SAM_64B;
         dam = IPHC_DAM_64B;
         p_dest = &temp_dest_mac64b;      
         p_src  = &temp_src_mac64b; 
      }
   } else {
     //not the same prefix. so the packet travels to another network
     //check if this is a source routing pkt. in case it is then the DAM is elided as it is in the SrcRouting header.
     if(ipv6_header.next_header!=IANA_IPv6ROUTE){ 
      sam = IPHC_SAM_128B;
      dam = IPHC_DAM_128B;
      p_dest = &(msg->l3_destinationAdd);
      p_src = &(msg->l3_sourceAdd);
     }else{
       //source routing
      sam = IPHC_SAM_128B;
      dam = IPHC_DAM_ELIDED; //poipoi xv not true, should not be elided.
      p_dest = NULL;
      p_src = &(msg->l3_sourceAdd);
     }
   }
   //check if we are forwarding a packet and it comes with the next header compressed. We want to preserve that state in the following hop.
   
   if ((fw_SendOrfw_Rcv==PCKTFORWARD) && ipv6_header.next_header_compressed) nh=IPHC_NH_COMPRESSED;
   
   // decrement the packet's hop limit
   ipv6_header.hop_limit--;
   
   //prepend Option hop by hop header except when src routing and dst is not 0xffff -- this is a little trick as src routing is using an option header set to 0x00
   next_header=msg->l4_protocol;
   if (hopbyhop_option->optionType==RPL_HOPBYHOP_HEADER_OPTION_TYPE 
       && packetfunctions_isBroadcastMulticast(&(msg->l3_destinationAdd))==FALSE ){
      prependIPv6HopByHopHeader(msg, msg->l4_protocol, nh, hopbyhop_option);
      //change nh to point to the newly added header
      next_header=IANA_IPv6HOPOPT;// use 0x00 as NH to indicate option header -- see rfc 2460
   }
   //then regular header
   if (prependIPv6Header(msg,
            IPHC_TF_ELIDED,
            0, // value_flowlabel is not copied
            nh,
            next_header, 
            IPHC_HLIM_INLINE,
            ipv6_header.hop_limit,
            IPHC_CID_NO,
            IPHC_SAC_STATELESS,
            sam,
            IPHC_M_NO,
            IPHC_DAC_STATELESS,
            dam,
            p_dest,
            p_src,            
            fw_SendOrfw_Rcv  
            )==E_FAIL) {
      return E_FAIL;
   }
   
   return res_send(msg);
}




//send from bridge: 6LoWPAN header already added by OpenLBR, send as is
owerror_t iphc_sendFromBridge(OpenQueueEntry_t *msg) {
   msg->owner = COMPONENT_IPHC;
   // error checking
   if (idmanager_getIsBridge()==FALSE) {
      /*openserial_printCritical(COMPONENT_IPHC,ERR_BRIDGE_MISMATCH,
                            (errorparameter_t)1,
                            (errorparameter_t)0);*/
      return E_FAIL;
   }
   return res_send(msg);
}

void iphc_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   msg->owner = COMPONENT_IPHC;
   if (msg->creator==COMPONENT_OPENBRIDGE) {
      openbridge_sendDone(msg,error);
   } else {
      forwarding_sendDone(msg,error);
   }
}

void iphc_receive(OpenQueueEntry_t* msg) {
   ipv6_header_iht ipv6_header;
   ipv6_hopbyhop_ht ipv6_hop_header;
   rpl_hopoption_ht hop_by_hop_option;
   
   msg->owner  = COMPONENT_IPHC;
   
   //then regular header
   ipv6_header = retrieveIPv6Header(msg);
   
  
   if (idmanager_getIsBridge()==FALSE ||
      packetfunctions_isBroadcastMulticast(&(ipv6_header.dest))) {
      packetfunctions_tossHeader(msg,ipv6_header.header_length);
      
      if (ipv6_header.next_header==IANA_IPv6HOPOPT){
          //retrieve hop by hop header
          retrieveIPv6HopByHopHeader(msg,&ipv6_hop_header,&hop_by_hop_option);
          //toss the header + option +tlv on it if any
          packetfunctions_tossHeader(msg,IPv6HOP_HDR_LEN+ipv6_hop_header.HdrExtLen);
      }


      forwarding_receive(msg,ipv6_header,ipv6_hop_header,hop_by_hop_option);       //up the internal stack
   } else {
	   openbridge_receive(msg);                   //out to the OpenVisualizer
   }
}

//=========================== private =========================================


void prependIPv6HopByHopHeader(OpenQueueEntry_t *msg,uint8_t nextheader, bool nh, rpl_hopoption_ht *hopbyhop_option){
   
    //copy them in reverse order, first option later header
    packetfunctions_reserveHeaderSize(msg,sizeof(rpl_hopoption_ht));
    memcpy(msg->payload,hopbyhop_option,sizeof(rpl_hopoption_ht));
    
    //hdr len as defined by rfc6282 sect 4.2
    packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
   *((uint8_t*)(msg->payload)) = sizeof(rpl_hopoption_ht);
    
    //next header
    switch (nh) {
      case IPHC_NH_INLINE:
        //add the next header inline
         packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
         *((uint8_t*)(msg->payload)) = nextheader;
       
         //append NHC field on the extension header should be 1110 0000 -- see rfc 6282 sect 4.2
         packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
         *((uint8_t*)(msg->payload)) = NHC_IPv6EXT_ID;
         break;
      case IPHC_NH_COMPRESSED:
         packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
         *((uint8_t*)(msg->payload)) = NHC_IPv6EXT_ID | 0x01; //mark last bit as 1 -- see rfc 6282 sect 4.2
        break;
        /*default:
         openserial_printCritical(COMPONENT_IPHC,ERR_6LOWPAN_UNSUPPORTED,
                               (errorparameter_t)3,
                               (errorparameter_t)nh);*/
   }

}

owerror_t prependIPv6Header(
      OpenQueueEntry_t* msg,
      uint8_t tf,
      uint32_t value_flowLabel,
      bool nh,
      uint8_t value_nextHeader,
      uint8_t hlim,
      uint8_t value_hopLimit,
      bool cid,
      bool sac,
      uint8_t sam,
      bool m,
      bool dac,
      uint8_t dam,
      open_addr_t* value_dest,
      open_addr_t* value_src,
      uint8_t fw_SendOrfw_Rcv
   ) {
   
   uint8_t temp_8b;
   
   //destination address
   switch (dam) {
      case IPHC_DAM_ELIDED:
         break;
      case IPHC_DAM_16B:
         if (value_dest->type!=ADDR_16B) {
            /*openserial_printCritical(COMPONENT_IPHC,ERR_WRONG_ADDR_TYPE,
                                  (errorparameter_t)value_dest->type,
                                  (errorparameter_t)0);*/
            return E_FAIL;
         };
         packetfunctions_writeAddress(msg,value_dest,OW_BIG_ENDIAN);
         break;
      case IPHC_DAM_64B:
         if (value_dest->type!=ADDR_64B) {
            /*openserial_printCritical(COMPONENT_IPHC,ERR_WRONG_ADDR_TYPE,
                                  (errorparameter_t)value_dest->type,
                                  (errorparameter_t)1);*/
            return E_FAIL;
         };
         packetfunctions_writeAddress(msg,value_dest,OW_BIG_ENDIAN);
         break;
      case IPHC_DAM_128B:
         if (value_dest->type!=ADDR_128B) {
            /*openserial_printCritical(COMPONENT_IPHC,ERR_WRONG_ADDR_TYPE,
                                  (errorparameter_t)value_dest->type,
                                  (errorparameter_t)2);*/
            return E_FAIL;
         };
         packetfunctions_writeAddress(msg,value_dest,OW_BIG_ENDIAN);
         break;
      default:
         /*openserial_printCritical(COMPONENT_IPHC,ERR_6LOWPAN_UNSUPPORTED,
                               (errorparameter_t)0,
                               (errorparameter_t)dam);*/
         return E_FAIL;
   }
   //source address
   switch (sam) {
      case IPHC_SAM_ELIDED:
         break;
      case IPHC_SAM_16B:
        if(fw_SendOrfw_Rcv==PCKTSEND)
        {
         packetfunctions_writeAddress(msg, (idmanager_getMyID(ADDR_16B)),OW_BIG_ENDIAN);
        }
        if(fw_SendOrfw_Rcv==PCKTFORWARD)
        {
            if (value_src->type!=ADDR_16B) {
                /*openserial_printCritical(COMPONENT_IPHC,ERR_WRONG_ADDR_TYPE,
                                      (errorparameter_t)value_src->type,
                                      (errorparameter_t)0);*/
                return E_FAIL;
            } 
            packetfunctions_writeAddress(msg,value_src,OW_BIG_ENDIAN);
        }
         break;
      case IPHC_SAM_64B:
        if(fw_SendOrfw_Rcv==PCKTSEND)
        {
          packetfunctions_writeAddress(msg, (idmanager_getMyID(ADDR_64B)),OW_BIG_ENDIAN);
        }
         if(fw_SendOrfw_Rcv==PCKTFORWARD)
        {
            if (value_src->type!=ADDR_64B) {
                /*openserial_printCritical(COMPONENT_IPHC,ERR_WRONG_ADDR_TYPE,
                                      (errorparameter_t)value_src->type,
                                      (errorparameter_t)1);*/
                return E_FAIL;
            }      
            packetfunctions_writeAddress(msg, value_src,OW_BIG_ENDIAN);
        }
         break;
      case IPHC_SAM_128B:
        if(fw_SendOrfw_Rcv==PCKTSEND)
        {
         packetfunctions_writeAddress(msg, (idmanager_getMyID(ADDR_64B)),OW_BIG_ENDIAN);
         packetfunctions_writeAddress(msg, (idmanager_getMyID(ADDR_PREFIX)),OW_BIG_ENDIAN);
        }
        if(fw_SendOrfw_Rcv==PCKTFORWARD)
        {
            if (value_src->type!=ADDR_128B) {
                /*openserial_printCritical(COMPONENT_IPHC,ERR_WRONG_ADDR_TYPE,
                                      (errorparameter_t)value_src->type,
                                      (errorparameter_t)2);*/
                return E_FAIL;
             }
           packetfunctions_writeAddress(msg,value_src,OW_BIG_ENDIAN);
        }
         break;
      default:
         /*openserial_printCritical(COMPONENT_IPHC,ERR_6LOWPAN_UNSUPPORTED,
                               (errorparameter_t)1,
                               (errorparameter_t)sam);*/
         return E_FAIL;
   }
   //hop limit
   switch (hlim) {
      case IPHC_HLIM_INLINE:
         packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
         *((uint8_t*)(msg->payload)) = value_hopLimit;
         break;
      case IPHC_HLIM_1:
      case IPHC_HLIM_64:
      case IPHC_HLIM_255:
         break;
      default:
         /*openserial_printCritical(COMPONENT_IPHC,ERR_6LOWPAN_UNSUPPORTED,
                               (errorparameter_t)2,
                               (errorparameter_t)hlim);*/
         return E_FAIL;
   }
   //next header
   switch (nh) {
      case IPHC_NH_INLINE:
         packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
         *((uint8_t*)(msg->payload)) = value_nextHeader;
         break;
      case IPHC_NH_COMPRESSED:
         //do nothing, the next header will be there
        break;
      default:
         /*openserial_printCritical(COMPONENT_IPHC,ERR_6LOWPAN_UNSUPPORTED,
                               (errorparameter_t)3,
                               (errorparameter_t)nh);*/
         return E_FAIL;
   }
   //flowlabel
   switch (tf) {
      case IPHC_TF_3B:
         packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
         *((uint8_t*)(msg->payload)) = ((uint32_t)(value_flowLabel & 0x000000ff) >> 0);
         packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
         *((uint8_t*)(msg->payload)) = ((uint32_t)(value_flowLabel & 0x0000ff00) >> 8);
         packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
         *((uint8_t*)(msg->payload)) = ((uint32_t)(value_flowLabel & 0x00ff0000) >> 16);
         break;            
      case IPHC_TF_ELIDED:
         break;
      case IPHC_TF_4B:
         //unsupported
      case IPHC_TF_1B:
         //unsupported
      default:
         /*openserial_printCritical(COMPONENT_IPHC,ERR_6LOWPAN_UNSUPPORTED,
                               (errorparameter_t)4,
                               (errorparameter_t)tf);*/
         return E_FAIL;
   }
   //header
   temp_8b  = 0;
   temp_8b |= cid                 << IPHC_CID;
   temp_8b |= sac                 << IPHC_SAC;
   temp_8b |= sam                 << IPHC_SAM;
   temp_8b |= m                   << IPHC_M;
   temp_8b |= dac                 << IPHC_DAC;
   temp_8b |= dam                 << IPHC_DAM;
   packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
   *((uint8_t*)(msg->payload)) = temp_8b;
   temp_8b  = 0;
   temp_8b |= IPHC_DISPATCH_IPHC  << IPHC_DISPATCH;
   temp_8b |= tf                  << IPHC_TF;
   temp_8b |= nh                  << IPHC_NH;
   temp_8b |= hlim                << IPHC_HLIM;
   packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
   *((uint8_t*)(msg->payload)) = temp_8b;
   return E_SUCCESS;
}



void retrieveIPv6HopByHopHeader(OpenQueueEntry_t *msg,ipv6_hopbyhop_ht *hopbyhop_header, rpl_hopoption_ht *rpl_option){
  uint8_t temp_8b;
  
  hopbyhop_header->headerlen=0;
   
  hopbyhop_header->lowpan_nhc = *((uint8_t*)(msg->payload)+ hopbyhop_header->headerlen);
  hopbyhop_header->headerlen += sizeof(uint8_t);   
  
   //next header
   switch ( hopbyhop_header->lowpan_nhc & NHC_HOP_NH_MASK) {
      case IPHC_NH_INLINE:
         // Full 8 bits for Next Header are carried in-line
         hopbyhop_header->next_header_compressed = FALSE;
         hopbyhop_header->nextHeader = *((uint8_t*)(msg->payload)+hopbyhop_header->headerlen);
         hopbyhop_header->headerlen+= sizeof(uint8_t);   
         break;
      case IPHC_NH_COMPRESSED:
         // the Next header field is compressed and the next header is encoded
         // using LOWPAN_NHC, which is discussed in Section 4.1 of RFC6282
         // we don't parse anything here, but will look at the (compressed)
         // next header after having parsed all address fields.
         hopbyhop_header->next_header_compressed = TRUE;
         break;
      default:
         /*openserial_printError(COMPONENT_IPHC,ERR_6LOWPAN_UNSUPPORTED,
                               (errorparameter_t)7,
                               (errorparameter_t)hopbyhop_header->lowpan_nhc);*/
         break;
   }
   
   //len of options
   hopbyhop_header->HdrExtLen =*((uint8_t*)(msg->payload)+hopbyhop_header->headerlen);
   hopbyhop_header->headerlen+= sizeof(uint8_t);  
   //copy the options
   memcpy(rpl_option,((uint8_t*)(msg->payload)+hopbyhop_header->headerlen),sizeof(rpl_hopoption_ht));
   hopbyhop_header->headerlen+= sizeof(rpl_hopoption_ht);  
   
   //now in case nh compressed:
    /*
   During the parsing of the nh field, we found that the next header was
   compressed. We now identify which next (compressed) header this is, and
   populate the hopbyhop_header.nextHeader field accordingly. It's the role of the
   appropriate transport module to decompress the header.
   */
   if (hopbyhop_header->next_header_compressed==TRUE) {
      temp_8b   = *((uint8_t*)(msg->payload)+ hopbyhop_header->headerlen);
      if    ( (temp_8b & NHC_UDP_MASK) == NHC_UDP_ID) {
         hopbyhop_header->nextHeader = IANA_UDP;
      }else {
         // the next header could be an IPv6 extension header, or misformed
         hopbyhop_header->nextHeader = IANA_UNDEFINED;
         /*openserial_printError(COMPONENT_IPHC,ERR_6LOWPAN_UNSUPPORTED,
                               (errorparameter_t)14,
                               (errorparameter_t)hopbyhop_header->nextHeader);*/
      }
   }
}


ipv6_header_iht retrieveIPv6Header(OpenQueueEntry_t* msg) {
   uint8_t         temp_8b;
   open_addr_t     temp_addr_16b;
   open_addr_t     temp_addr_64b;
   ipv6_header_iht ipv6_header;
   uint8_t         dispatch;
   uint8_t         tf;
   bool            nh;
   uint8_t         hlim;
   //bool            cid;
   //bool            sac;
   uint8_t         sam;
  // bool            m;
   //bool          dac;
   uint8_t         dam;
   
   ipv6_header.header_length = 0;
   /*openserial_printError(COMPONENT_IPHC,ERR_6LOWPAN_UNSUPPORTED,
                         (errorparameter_t)*(uint8_t*)(msg->payload),
                         (errorparameter_t)301);*/

   //header
   temp_8b   = *((uint8_t*)(msg->payload)+ipv6_header.header_length);
   dispatch  = (temp_8b >> IPHC_DISPATCH)  & 0x07;//3b
   tf        = (temp_8b >> IPHC_TF)        & 0x03;//2b
   nh        = (temp_8b >> IPHC_NH)        & 0x01;//1b
   hlim      = (temp_8b >> IPHC_HLIM)      & 0x03;//2b
   ipv6_header.header_length += sizeof(uint8_t);
   temp_8b   = *((uint8_t*)(msg->payload)+ipv6_header.header_length);
   //cid       = (temp_8b >> IPHC_CID)       & 0x01;//1b unused
   //sac       = (temp_8b >> IPHC_SAC)       & 0x01;//1b unused
   sam       = (temp_8b >> IPHC_SAM)       & 0x03;//2b
   //m         = (temp_8b >> IPHC_M)         & 0x01;//1b unused
   //dac       = (temp_8b >> IPHC_DAC)       & 0x01;//1b unused
   dam       = (temp_8b >> IPHC_DAM)       & 0x03;//2b
   ipv6_header.header_length += sizeof(uint8_t);
   //dispatch
   switch (dispatch) {
      case IPHC_DISPATCH_IPHC:
         break;            
      /*default:
         openserial_printError(COMPONENT_IPHC,ERR_6LOWPAN_UNSUPPORTED,
                               (errorparameter_t)5,
                               (errorparameter_t)dispatch);
         break;*/
   }
   //flowlabel
   switch (tf) {
      case IPHC_TF_3B:
         ipv6_header.flow_label  = ((uint32_t) *((uint8_t*)(msg->payload)+ipv6_header.header_length))<<0;
         ipv6_header.header_length += sizeof(uint8_t);
         ipv6_header.flow_label |= ((uint32_t) *((uint8_t*)(msg->payload)+ipv6_header.header_length))<<8;
         ipv6_header.header_length += sizeof(uint8_t);
         ipv6_header.flow_label |= ((uint32_t) *((uint8_t*)(msg->payload)+ipv6_header.header_length))<<16;
         ipv6_header.header_length += sizeof(uint8_t);
         break;            
      case IPHC_TF_ELIDED:
         ipv6_header.flow_label  = 0;
         break;
      case IPHC_TF_4B:
         //unsupported
      case IPHC_TF_1B:
         //unsupported
      default:
         /*openserial_printError(COMPONENT_IPHC,ERR_6LOWPAN_UNSUPPORTED,
                               (errorparameter_t)6,
                               (errorparameter_t)tf);*/
         break;
   }
   //next header
   switch (nh) {
      case IPHC_NH_INLINE:
         // Full 8 bits for Next Header are carried in-line
         ipv6_header.next_header_compressed = FALSE;
         ipv6_header.next_header = *((uint8_t*)(msg->payload)+ipv6_header.header_length);
         ipv6_header.header_length += sizeof(uint8_t);
      
         break;
      case IPHC_NH_COMPRESSED:
         // the Next header field is compressed and the next header is encoded
         // using LOWPAN_NHC, which is discussed in Section 4.1 of RFC6282
         // we don't parse anything here, but will look at the (compressed)
         // next header after having parsed all address fields.
         ipv6_header.next_header_compressed = TRUE;
         break;
      default:
         openserial_printError(COMPONENT_IPHC,ERR_6LOWPAN_UNSUPPORTED,
                               (errorparameter_t)7,
                               (errorparameter_t)nh);
         break;
   }
   //hop limit
   switch (hlim) {
      case IPHC_HLIM_INLINE:
         ipv6_header.hop_limit = *((uint8_t*)(msg->payload+ipv6_header.header_length));
         ipv6_header.header_length += sizeof(uint8_t);
         break;
      case IPHC_HLIM_1:
         ipv6_header.hop_limit = 1;
         break;
      case IPHC_HLIM_64:
         ipv6_header.hop_limit = 64;
         break;
      case IPHC_HLIM_255:
         ipv6_header.hop_limit = 255;
         break;
      default:
         /*openserial_printError(COMPONENT_IPHC,ERR_6LOWPAN_UNSUPPORTED,
                               (errorparameter_t)8,
                               (errorparameter_t)hlim);*/
         break;
   }
   //source address
   switch (sam) {
      case IPHC_SAM_ELIDED:
         packetfunctions_mac64bToIp128b(idmanager_getMyID(ADDR_PREFIX),&(msg->l2_nextORpreviousHop),&ipv6_header.src);
         break;
      case IPHC_SAM_16B:
         packetfunctions_readAddress(((uint8_t*)(msg->payload+ipv6_header.header_length)),ADDR_16B,&temp_addr_16b,OW_BIG_ENDIAN);
         ipv6_header.header_length += 2*sizeof(uint8_t);
         packetfunctions_mac16bToMac64b(&temp_addr_16b,&temp_addr_64b);
         packetfunctions_mac64bToIp128b(idmanager_getMyID(ADDR_PREFIX),&temp_addr_64b,&ipv6_header.src);
         break;
      case IPHC_SAM_64B:
         packetfunctions_readAddress(((uint8_t*)(msg->payload+ipv6_header.header_length)),ADDR_64B,&temp_addr_64b,OW_BIG_ENDIAN);
         ipv6_header.header_length += 8*sizeof(uint8_t);
         packetfunctions_mac64bToIp128b(idmanager_getMyID(ADDR_PREFIX),&temp_addr_64b,&ipv6_header.src);
         break;
      case IPHC_SAM_128B:
         packetfunctions_readAddress(((uint8_t*)(msg->payload+ipv6_header.header_length)),ADDR_128B,&ipv6_header.src,OW_BIG_ENDIAN);
         ipv6_header.header_length += 16*sizeof(uint8_t);
         break;
      default:
         /*openserial_printError(COMPONENT_IPHC,ERR_6LOWPAN_UNSUPPORTED,
                               (errorparameter_t)9,
                               (errorparameter_t)sam);*/
         break;
   }
   //destination address
   switch (dam) {
      case IPHC_DAM_ELIDED:
         packetfunctions_mac64bToIp128b(idmanager_getMyID(ADDR_PREFIX),idmanager_getMyID(ADDR_64B),&(ipv6_header.dest));
         break;
      case IPHC_DAM_16B:
         packetfunctions_readAddress(((uint8_t*)(msg->payload+ipv6_header.header_length)),ADDR_16B,&temp_addr_16b,OW_BIG_ENDIAN);
         ipv6_header.header_length += 2*sizeof(uint8_t);
         packetfunctions_mac16bToMac64b(&temp_addr_16b,&temp_addr_64b);
         packetfunctions_mac64bToIp128b(idmanager_getMyID(ADDR_PREFIX),&temp_addr_64b,&ipv6_header.dest);
         break;
      case IPHC_DAM_64B:
         packetfunctions_readAddress(((uint8_t*)(msg->payload+ipv6_header.header_length)),ADDR_64B,&temp_addr_64b,OW_BIG_ENDIAN);
         ipv6_header.header_length += 8*sizeof(uint8_t);
         packetfunctions_mac64bToIp128b(idmanager_getMyID(ADDR_PREFIX),&temp_addr_64b,&ipv6_header.dest);
         break;
      case IPHC_DAM_128B:
         packetfunctions_readAddress(((uint8_t*)(msg->payload+ipv6_header.header_length)),ADDR_128B,&ipv6_header.dest,OW_BIG_ENDIAN);
         ipv6_header.header_length += 16*sizeof(uint8_t);
         break;
      default:
         /*openserial_printError(COMPONENT_IPHC,ERR_6LOWPAN_UNSUPPORTED,
                               (errorparameter_t)10,
                               (errorparameter_t)dam);*/
         break;
   }
   /*
   During the parsing of the nh field, we found that the next header was
   compressed. We now identify which next (compressed) header this is, and
   populate the ipv6_header.next_header field accordingly. It's the role of the
   appropriate transport module to decompress the header.
   */
   if (ipv6_header.next_header_compressed==TRUE) {
      temp_8b   = *((uint8_t*)(msg->payload)+ipv6_header.header_length);
      if    ( (temp_8b & NHC_UDP_MASK) == NHC_UDP_ID) {
         ipv6_header.next_header = IANA_UDP;
      }else if ( (temp_8b & NHC_IPv6EXT_MASK) == NHC_IPv6EXT_ID){
        if( temp_8b & NHC_IPv6HOP_MASK == NHC_IPv6HOP_VAL){
          //it is hop by hop header
          ipv6_header.next_header = IANA_IPv6HOPOPT;
        }else{
          // the next header could be another IPv6 extension header
          ipv6_header.next_header = IANA_UNDEFINED;
          /*openserial_printError(COMPONENT_IPHC,ERR_6LOWPAN_UNSUPPORTED,
                               (errorparameter_t)11,
                               (errorparameter_t)ipv6_header.next_header);*/
        }
      }else {
         // the next header could be an IPv6 extension header, or misformed
         ipv6_header.next_header = IANA_UNDEFINED;
         /*openserial_printError(COMPONENT_IPHC,ERR_6LOWPAN_UNSUPPORTED,
                               (errorparameter_t)12,
                               (errorparameter_t)ipv6_header.next_header);*/
      }
   }
   
   return ipv6_header;
}
