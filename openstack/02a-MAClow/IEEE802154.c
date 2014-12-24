#include "opendefs.h"
#include "IEEE802154.h"
#include "packetfunctions.h"
#include "idmanager.h"
#include "openserial.h"
#include "topology.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

/**
\brief Prepend the IEEE802.15.4 MAC header to a (to be transmitted) packet.

Note that we are writing the field from the end of the header to the beginning.

\param[in,out] msg              The message to append the header to.
\param[in]     frameType        Type of IEEE802.15.4 frame.
\param[in]     ielistpresent    Is the IE list present�
\param[in]     frameVersion     IEEE802.15.4 frame version.
\param[in]     securityEnabled  Is security enabled on this frame?
\param[in]     sequenceNumber   Sequence number of this frame.
\param[in]     nextHop          Address of the next hop
*/
void ieee802154_prependHeader(OpenQueueEntry_t* msg,
                              uint8_t           frameType,
                              uint8_t           ielistpresent,
                              uint8_t           frameVersion,
                              bool              securityEnabled,
                              uint8_t           sequenceNumber,
                              open_addr_t*      nextHop) {
   uint8_t temp_8b;
   
   //General IEs here (those that are carried in all packets) -- None by now.
   
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
   } else {
      switch (nextHop->type) {
         case ADDR_16B:
            temp_8b    |= IEEE154_ADDR_SHORT              << IEEE154_FCF_DEST_ADDR_MODE;
            break;
         case ADDR_64B:
            temp_8b    |= IEEE154_ADDR_EXT                << IEEE154_FCF_DEST_ADDR_MODE;
            break;
         // no need for a default, since it would have been caught above.
      }
   }
   temp_8b             |= IEEE154_ADDR_EXT                << IEEE154_FCF_SRC_ADDR_MODE;
   //poipoi xv IE list present
   temp_8b             |= ielistpresent                   << IEEE154_FCF_IELIST_PRESENT;
   temp_8b             |= frameVersion                    << IEEE154_FCF_FRAME_VERSION;
     
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
   temp_8b             |= IEEE154_PANID_COMPRESSED      << IEEE154_FCF_INTRAPAN;
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
   uint8_t temp_8b;
   
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

   if (ieee802514_header->ieListPresent==TRUE && ieee802514_header->frameVersion!=IEEE154_FRAMEVERSION){
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
   // sequenceNumber
   if (ieee802514_header->headerLength>msg->length) { return; } // no more to read!
   ieee802514_header->dsn  = *((uint8_t*)(msg->payload)+ieee802514_header->headerLength);
   ieee802514_header->headerLength += 1;
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
   
   if (ieee802514_header->ieListPresent==TRUE && ieee802514_header->frameVersion!=IEEE154_FRAMEVERSION){
       return; //invalid packet accordint to p.64 IEEE15.4e
   }
   
   // apply topology filter
   if (topology_isAcceptablePacket(ieee802514_header)==FALSE) {
      // the topology filter does accept this packet, return
      return;
   }
   // if you reach this, the header is valid
   ieee802514_header->valid=TRUE;
}

//=========================== private =========================================
