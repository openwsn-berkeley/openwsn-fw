#include "opendefs.h"
#include "IEEE802154.h"
#include "IEEE802154E.h"
#include "packetfunctions.h"
#include "idmanager.h"
#include "openserial.h"
#include "topology.h"
#include "IEEE802154_security.h"

//=========================== define ==========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

/**
\brief Prepend the IEEE802.15.4 MAC header to a (to be transmitted) packet.

Note that we are writing the field from the end of the header to the beginning.

\param[in,out] msg              The message to append the header to.
\param[in]     frameType        Type of IEEE802.15.4 frame.
\param[in]     payloadIEPresent Is the IE list present?
\param[in]     sequenceNumber   Sequence number of this frame.
\param[in]     nextHop          Address of the next hop
*/
void ieee802154_prependHeader(OpenQueueEntry_t* msg,
                              uint8_t           frameType,
                              bool              payloadIEPresent,
                              uint8_t           sequenceNumber,
                              open_addr_t*      nextHop) {
   uint8_t      temp_8b;
   uint8_t      ielistpresent = IEEE154_IELIST_NO;
   bool         securityEnabled;
   int16_t      timeCorrection;
   uint16_t     timeSyncInfo;
   uint16_t     length_elementid_type;
   bool         headerIEPresent = FALSE;
   uint8_t      destAddrMode = -1;

   securityEnabled = msg->l2_securityLevel == IEEE154_ASH_SLF_TYPE_NOSEC ? 0 : 1;

   msg->l2_payload = msg->payload; // save the position where to start encrypting if security is enabled

   // General IEs here (those that are carried in all packets)
   // add termination IE accordingly
   if (payloadIEPresent == TRUE) {
       ielistpresent = IEEE154_IELIST_YES;
       //add header termination IE (id=0x7e)
       packetfunctions_reserveHeaderSize(msg,TERMINATIONIE_LEN);
       msg->payload[0] = HEADER_TERMINATION_1_IE         & 0xFF;
       msg->payload[1] = (HEADER_TERMINATION_1_IE  >> 8) & 0xFF;


   } else {
       // check whether I have payload, if yes, add header termination IE (0x7F)
       // or ternimation IE will be omitted. For example, Keep alive doesn't have
       // any payload, so there is no ternimation IE for it.
       if (msg->length != 0) {
           //add header termination IE (id=0x7f)if I have header IE list, OR
           // no need for termination IE.
           if (headerIEPresent == TRUE){
               ielistpresent = IEEE154_IELIST_YES;
               packetfunctions_reserveHeaderSize(msg,TERMINATIONIE_LEN);
               msg->payload[0] = HEADER_TERMINATION_2_IE        & 0xFF;
               msg->payload[1] = (HEADER_TERMINATION_2_IE >> 8) & 0xFF;
           } else {
               // no header IE present, no payload IE, no termination IE
           }
       } else {
           // no payload, termination IE is omitted. check whether timeCorrection IE
           // presents.
           if (frameType != IEEE154_TYPE_ACK) {
           } else {
               ielistpresent = IEEE154_IELIST_YES; // I will have a timeCorrection IE later
           }
       }
  }

   if (frameType == IEEE154_TYPE_ACK) {
       timeCorrection = (int16_t)(ieee154e_getTimeCorrection());
       // add the payload to the ACK (i.e. the timeCorrection)
       packetfunctions_reserveHeaderSize(msg,sizeof(uint16_t));
       timeCorrection *= PORT_US_PER_TICK;
       timeSyncInfo  = ((uint16_t)timeCorrection) & 0x0fff;
       if (msg->l2_isNegativeACK){
          timeSyncInfo |= 0x8000;
       }
       msg->payload[0] = (uint8_t)(((timeSyncInfo)   ) & 0xff);
       msg->payload[1] = (uint8_t)(((timeSyncInfo)>>8) & 0xff);

       // add header IE header -- xv poipoi -- pkt is filled in reverse order..
       packetfunctions_reserveHeaderSize(msg,sizeof(uint16_t));
       //create the header for ack IE
       length_elementid_type=sizeof(uint16_t)|
                             (IEEE802154E_ACK_NACK_TIMECORRECTION_ELEMENTID << IEEE802154E_DESC_ELEMENTID_HEADER_IE_SHIFT)|
                             (IEEE802154E_DESC_TYPE_SHORT << IEEE802154E_DESC_TYPE_IE_SHIFT);
       msg->payload[0] = (length_elementid_type)        & 0xFF;
       msg->payload[1] = ((length_elementid_type) >> 8) & 0xFF;
   }

   //if security is enabled, the Auxiliary Security Header need to be added to the IEEE802.15.4 MAC header
   if(securityEnabled){
      IEEE802154_security_prependAuxiliarySecurityHeader(msg);
   }

   // previousHop address (always 64-bit)
   packetfunctions_writeAddress(msg,idmanager_getMyID(ADDR_64B),OW_LITTLE_ENDIAN);
   // nextHop address
   if (packetfunctions_isBroadcastMulticast(nextHop)) {
      //broadcast address is always 16-bit
      packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
      *((uint8_t*)(msg->payload)) = 0xFF;
      packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
      *((uint8_t*)(msg->payload)) = 0xFF;
   } else {
      switch (nextHop->type) {
         case ADDR_16B:
         case ADDR_64B:
            packetfunctions_writeAddress(msg,nextHop,OW_LITTLE_ENDIAN);
            break;
         default:
            openserial_printCritical(COMPONENT_IEEE802154,ERR_WRONG_ADDR_TYPE,
                                  (errorparameter_t)nextHop->type,
                                  (errorparameter_t)1);
      }

   }

   msg->l2_nextHop_payload = msg->payload;

   // destpan -- se page 41 of 15.4-2011 std. DEST PANID only sent as it is equal to SRC PANID
   packetfunctions_writeAddress(msg,idmanager_getMyID(ADDR_PANID),OW_LITTLE_ENDIAN);

   //dsn
   packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
   *((uint8_t*)(msg->payload)) = sequenceNumber;

   //fcf (2nd byte)
   packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
   temp_8b              = 0;
   if (packetfunctions_isBroadcastMulticast(nextHop)) {
      temp_8b          |= IEEE154_ADDR_SHORT              << IEEE154_FCF_DEST_ADDR_MODE;
      destAddrMode      = IEEE154_ADDR_SHORT;
   } else {
      switch (nextHop->type) {
         case ADDR_16B:
            temp_8b    |= IEEE154_ADDR_SHORT              << IEEE154_FCF_DEST_ADDR_MODE;
            destAddrMode= IEEE154_ADDR_SHORT;
            break;
         case ADDR_64B:
            temp_8b    |= IEEE154_ADDR_EXT                << IEEE154_FCF_DEST_ADDR_MODE;
            destAddrMode= IEEE154_ADDR_EXT;
            break;
         // no need for a default, since it would have been caught above.
      }
   }
   temp_8b             |= IEEE154_ADDR_EXT                << IEEE154_FCF_SRC_ADDR_MODE;
   //poipoi xv IE list present
   temp_8b             |= ielistpresent                   << IEEE154_FCF_IELIST_PRESENT;
   temp_8b             |= IEEE154_FRAMEVERSION_2012       << IEEE154_FCF_FRAME_VERSION;
   temp_8b             |= IEEE154_DSN_SUPPRESSION_NO      << IEEE154_FCF_DSN_SUPPRESSION;

   *((uint8_t*)(msg->payload)) = temp_8b;
   //fcf (1st byte)
   packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
   temp_8b              = 0;
   temp_8b             |= frameType                       << IEEE154_FCF_FRAME_TYPE;
   temp_8b             |= securityEnabled                 << IEEE154_FCF_SECURITY_ENABLED;
   temp_8b             |= IEEE154_PENDING_NO_FRAMEPENDING << IEEE154_FCF_FRAME_PENDING;
   if (frameType==IEEE154_TYPE_ACK || packetfunctions_isBroadcastMulticast(nextHop)) {
      temp_8b          |= IEEE154_ACK_NO_ACK_REQ          << IEEE154_FCF_ACK_REQ;
   } else {
      temp_8b          |= IEEE154_ACK_YES_ACK_REQ         << IEEE154_FCF_ACK_REQ;
   }
   if (destAddrMode == IEEE154_ADDR_SHORT) {
       temp_8b         |= IEEE154_PANID_COMPRESSED        << IEEE154_FCF_INTRAPAN;
   } else {
       if (destAddrMode == IEEE154_ADDR_EXT) {
           temp_8b     |= IEEE154_PANID_UNCOMPRESSED      << IEEE154_FCF_INTRAPAN;
       } else {
           // never happens
       }
   }
   *((uint8_t*)(msg->payload)) = temp_8b;
}

/**
\brief Retreieve the IEEE802.15.4 MAC header from a (just received) packet.

Note We are writing the fields from the begnning of the header to the end.

\param[in,out] msg            The message just received.
\param[out] ieee802514_header The internal header to write the data to.
*/
void ieee802154_retrieveHeader(OpenQueueEntry_t*      msg,
                               ieee802154_header_iht* ieee802514_header) {
   uint8_t  temp_8b;
   uint16_t temp_16b;
   uint16_t len;
   uint8_t  gr_elem_id;
   uint8_t  byte0;
   uint8_t  byte1;
   int16_t  timeCorrection;
   uint16_t timeSyncInfo;
   // by default, let's assume the header is not valid, in case we leave this
   // function because the packet ends up being shorter than the header.
   ieee802514_header->valid=FALSE;

   ieee802514_header->headerLength = 0;
   // fcf, byte 1
   if (ieee802514_header->headerLength>msg->length) { return; } // no more to read!
   temp_8b = *((uint8_t*)(msg->payload)+ieee802514_header->headerLength);
   ieee802514_header->frameType         = (temp_8b >> IEEE154_FCF_FRAME_TYPE      ) & 0x07;//3b
   ieee802514_header->securityEnabled   = (temp_8b >> IEEE154_FCF_SECURITY_ENABLED) & 0x01;//1b
   ieee802514_header->framePending      = (temp_8b >> IEEE154_FCF_FRAME_PENDING   ) & 0x01;//1b
   ieee802514_header->ackRequested      = (temp_8b >> IEEE154_FCF_ACK_REQ         ) & 0x01;//1b
   ieee802514_header->panIDCompression  = (temp_8b >> IEEE154_FCF_INTRAPAN        ) & 0x01;//1b
   ieee802514_header->headerLength += 1;
   // fcf, byte 2
   if (ieee802514_header->headerLength>msg->length) { return; } // no more to read!
   temp_8b = *((uint8_t*)(msg->payload)+ieee802514_header->headerLength);
   //poipoi xv IE list present
   ieee802514_header->ieListPresent  = (temp_8b >> IEEE154_FCF_IELIST_PRESENT     ) & 0x01;//1b
   ieee802514_header->frameVersion   = (temp_8b >> IEEE154_FCF_FRAME_VERSION      ) & 0x03;//2b
   ieee802514_header->dsn_suppressed = (temp_8b >> IEEE154_FCF_DSN_SUPPRESSION    ) & 0x01;//1b

   if (ieee802514_header->ieListPresent==TRUE && ieee802514_header->frameVersion!=IEEE154_FRAMEVERSION_2012){
       return; //invalid packet accordint to p.64 IEEE15.4e
   }

   switch ( (temp_8b >> IEEE154_FCF_DEST_ADDR_MODE ) & 0x03 ) {
      case IEEE154_ADDR_NONE:
         ieee802514_header->dest.type = ADDR_NONE;
         break;
      case IEEE154_ADDR_SHORT:
         ieee802514_header->dest.type = ADDR_16B;
         break;
      case IEEE154_ADDR_EXT:
         ieee802514_header->dest.type = ADDR_64B;
         break;
      default:
         openserial_printError(COMPONENT_IEEE802154,ERR_IEEE154_UNSUPPORTED,
                               (errorparameter_t)1,
                               (errorparameter_t)(temp_8b >> IEEE154_FCF_DEST_ADDR_MODE ) & 0x03);
         return; // this is an invalid packet, return
   }
   switch ( (temp_8b >> IEEE154_FCF_SRC_ADDR_MODE ) & 0x03 ) {
      case IEEE154_ADDR_NONE:
         ieee802514_header->src.type = ADDR_NONE;
         break;
      case IEEE154_ADDR_SHORT:
         ieee802514_header->src.type = ADDR_16B;
         break;
      case IEEE154_ADDR_EXT:
         ieee802514_header->src.type = ADDR_64B;
         break;
      default:
         openserial_printError(COMPONENT_IEEE802154,ERR_IEEE154_UNSUPPORTED,
                               (errorparameter_t)2,
                               (errorparameter_t)(temp_8b >> IEEE154_FCF_SRC_ADDR_MODE ) & 0x03);
         return; // this is an invalid packet, return
   }
   ieee802514_header->headerLength += 1;

   if (ieee802514_header->dsn_suppressed==FALSE) {
       // sequenceNumber
       if (ieee802514_header->headerLength>msg->length) { return; } // no more to read!
       ieee802514_header->dsn  = *((uint8_t*)(msg->payload)+ieee802514_header->headerLength);
       ieee802514_header->headerLength += 1;
   }

   // panID
   if (ieee802514_header->headerLength>msg->length) { return; } // no more to read!
   packetfunctions_readAddress(((uint8_t*)(msg->payload)+ieee802514_header->headerLength),
                               ADDR_PANID,
                               &ieee802514_header->panid,
                               OW_LITTLE_ENDIAN);
   ieee802514_header->headerLength += 2;
   // dest
   if (ieee802514_header->headerLength>msg->length) { return; } // no more to read!
   switch (ieee802514_header->dest.type) {
      case ADDR_NONE:
         break;
      case ADDR_16B:
         packetfunctions_readAddress(
             ((uint8_t*)(msg->payload)+ieee802514_header->headerLength),
             ADDR_16B,
             &ieee802514_header->dest,
             OW_LITTLE_ENDIAN
         );
         ieee802514_header->headerLength += 2;
         if (ieee802514_header->headerLength>msg->length) {  return; } // no more to read!
         break;
      case ADDR_64B:
         packetfunctions_readAddress(((uint8_t*)(msg->payload)+ieee802514_header->headerLength),
                                     ADDR_64B,
                                     &ieee802514_header->dest,
                                     OW_LITTLE_ENDIAN);
         ieee802514_header->headerLength += 8;
         if (ieee802514_header->headerLength>msg->length) {  return; } // no more to read!
         break;
      // no need for a default, since case would have been caught above
   }
   //src
   switch (ieee802514_header->src.type) {
      case ADDR_NONE:
         break;
      case ADDR_16B:
         packetfunctions_readAddress(((uint8_t*)(msg->payload)+ieee802514_header->headerLength),
                                     ADDR_16B,
                                     &ieee802514_header->src,
                                     OW_LITTLE_ENDIAN);
         ieee802514_header->headerLength += 2;
         if (ieee802514_header->headerLength>msg->length) {  return; } // no more to read!
         break;
      case ADDR_64B:
         packetfunctions_readAddress(((uint8_t*)(msg->payload)+ieee802514_header->headerLength),
                                     ADDR_64B,
                                     &ieee802514_header->src,
                                     OW_LITTLE_ENDIAN);
         ieee802514_header->headerLength += 8;
         if (ieee802514_header->headerLength>msg->length) {  return; } // no more to read!
         break;
         // no need for a default, since case would have been caught above
   }

   if (ieee802514_header->ieListPresent==TRUE && ieee802514_header->frameVersion!=IEEE154_FRAMEVERSION_2012){
       return; //invalid packet accordint to p.64 IEEE15.4e
   }

   // parse security header if security is supported locally
   if (ieee802514_header->securityEnabled && IEEE802154_SECURITY_SUPPORTED) {
       IEEE802154_security_retrieveAuxiliarySecurityHeader(msg,ieee802514_header);
   }
   else if (ieee802514_header->securityEnabled && IEEE802154_SECURITY_SUPPORTED==0) {
      return; // security not supported
   }

   // remove termination IE accordingly
   if (ieee802514_header->ieListPresent == TRUE) {
       while(1) {
           // I have IE in frame. phase the IE in header first
           temp_8b  = *((uint8_t*)(msg->payload)+ieee802514_header->headerLength);
           ieee802514_header->headerLength += 1;
           temp_16b = temp_8b | (*((uint8_t*)(msg->payload)+ieee802514_header->headerLength) << 8);
           ieee802514_header->headerLength += 1;
           // stop when I got a header termination IE
           if (temp_16b == HEADER_TERMINATION_1_IE) {
               // I have payloadIE following
               msg->l2_payloadIEpresent = TRUE;
               break;
           }
           if (temp_16b == HEADER_TERMINATION_2_IE) {
               // I have payload following
               msg->l2_payloadIEpresent = FALSE;
               break;
           }
           if ((temp_16b & IEEE802154E_DESC_TYPE_PAYLOAD_IE) != IEEE802154E_DESC_TYPE_PAYLOAD_IE) {
               // only process header IE here
               len          = temp_16b & IEEE802154E_DESC_LEN_HEADER_IE_MASK;
               gr_elem_id   = (temp_16b & IEEE802154E_DESC_ELEMENTID_HEADER_IE_MASK)>>IEEE802154E_DESC_ELEMENTID_HEADER_IE_SHIFT;

               switch(gr_elem_id){
                   case IEEE802154E_ACK_NACK_TIMECORRECTION_ELEMENTID:
                       // timecorrection IE
                       byte0 = *((uint8_t*)(msg->payload)+ieee802514_header->headerLength);
                       byte1 = *((uint8_t*)(msg->payload)+ieee802514_header->headerLength+1);
                       timeSyncInfo    = (uint16_t)byte1<<8 | (uint16_t)byte0;
                       // negative ACK or not
                       if (timeSyncInfo & 0x8000){
                            msg->l2_isNegativeACK = TRUE;
                       } else {
                            msg->l2_isNegativeACK = FALSE;
                       }
                       // negative timeCorrection or not, cast from 12 to 16 bit signed integer
                       if (timeSyncInfo & 0x0800){
                           timeCorrection = timeSyncInfo | 0xf000;
                       } else {
                           timeCorrection = timeSyncInfo & 0x0fff;
                       }
                       timeCorrection  = ((int16_t)timeCorrection / (PORT_SIGNED_INT_WIDTH)PORT_US_PER_TICK);

                       ieee802514_header->timeCorrection = timeCorrection;
                       ieee802514_header->headerLength  += len;

                       // Record the position where we should start decrypting the ACK, if security is enabled
                       msg->l2_payload = &msg->payload[ieee802514_header->headerLength];
                       break;
                   default:
                       break;
               }
           }
           if (ieee802514_header->headerLength==msg->length) {
               // nothing left, no payloadIE, no payload, this is the end of packet!

               ieee802514_header->valid=TRUE;
               return;
           }
           if (ieee802514_header->headerLength>msg->length) {  return; } // no more to read!
       }
   }

   // Record the position where we should start decrypting if security is enabled
   msg->l2_payload = &msg->payload[ieee802514_header->headerLength];

   // apply topology filter
   if (topology_isAcceptablePacket(ieee802514_header)==FALSE) {
      // the topology filter does accept this packet, return
      return;
   }
   // if you reach this, the header is valid
   ieee802514_header->valid=TRUE;
}

//=========================== private =========================================
