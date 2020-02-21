#include "opendefs.h"
#include "openudp.h"
#include "openserial.h"
#include "packetfunctions.h"
#include "forwarding.h"
#include "openqueue.h"

//=========================== variables =======================================

openudp_vars_t openudp_vars;

//=========================== prototypes ======================================

static void openudp_sendDone_default_handler(OpenQueueEntry_t* msg, owerror_t error);
static void openudp_receive_default_handler(OpenQueueEntry_t* msg);

//=========================== public ==========================================

void openudp_init(void) {
   // initialize the resource linked list
   openudp_vars.resources = NULL;
}

void openudp_register(udp_resource_desc_t* desc) {
   // chain the new resource to head of resource list
   desc->next = openudp_vars.resources;
   openudp_vars.resources = desc;
}

owerror_t openudp_send(OpenQueueEntry_t* msg) {
    uint8_t* checksum_position;
    msg->owner       = COMPONENT_OPENUDP;
    msg->l4_protocol = IANA_UDP;
    msg->l4_payload  = msg->payload;
    msg->l4_length   = msg->length;
    
    msg->l4_protocol_compressed = FALSE; // by default
    uint8_t compType = NHC_UDP_PORTS_INLINE;

    //check if the header can be compressed.
    if (msg->l4_destination_port>=0xf000 && msg->l4_destination_port<0xf100){
        // destination can be compressed 8 bit
        msg->l4_protocol_compressed = TRUE;
        compType = NHC_UDP_PORTS_16S_8D;
        //check now if both can still be more compressed 4b each
        if (
            msg->l4_destination_port       >= 0xf0b0 &&
            msg->l4_destination_port       <= 0xf0bf &&
            msg->l4_sourcePortORicmpv6Type >= 0xf0b0 &&
            msg->l4_sourcePortORicmpv6Type <= 0xf0bf
        ){
            //can be fully compressed
            compType = NHC_UDP_PORTS_4S_4D;
        }
    } else {
        //check source now
        if (msg->l4_sourcePortORicmpv6Type>=0xf000 && msg->l4_sourcePortORicmpv6Type<0xf100){
            //source can be compressed -> 8bit
            msg->l4_protocol_compressed = TRUE;
            compType = NHC_UDP_PORTS_8S_16D;
        }
    }

    // fill in the header in the packet
    if (msg->l4_protocol_compressed){

        //add checksum space in the packet.
        packetfunctions_reserveHeaderSize(msg,2);
        //keep position and calculatre checksum at the end.
        checksum_position = &msg->payload[0];

        //length is always omitted
        /*
         RFC6282 -> The UDP Length field
                   MUST always be elided and is inferred from lower layers using the
                   6LoWPAN Fragmentation header or the IEEE 802.15.4 header.
        */

        switch (compType) {
            case NHC_UDP_PORTS_INLINE:
                // error this is not possible.
                break;
            case NHC_UDP_PORTS_16S_8D:
                // dest port:   0xf0  +  8 bits in-line
                // source port:         16 bits in-line
                packetfunctions_reserveHeaderSize(msg,1);
                msg->payload[0] = (uint8_t) (msg->l4_destination_port & 0x00ff);
                packetfunctions_reserveHeaderSize(msg,2);
                packetfunctions_htons(msg->l4_sourcePortORicmpv6Type,&(msg->payload[0]));
                //write proper LOWPAN_NHC
                packetfunctions_reserveHeaderSize(msg,1);
                msg->payload[0] = NHC_UDP_ID|NHC_UDP_PORTS_16S_8D;
                break;
            case NHC_UDP_PORTS_8S_16D:
                // dest port:   0xf0  + 16 bits in-line
                // source port: 0xf0  +  8 bits in-line
                packetfunctions_reserveHeaderSize(msg,2);
                packetfunctions_htons(msg->l4_destination_port,&(msg->payload[0]));
                packetfunctions_reserveHeaderSize(msg,1);
                msg->payload[0] = (uint8_t) (msg->l4_sourcePortORicmpv6Type & 0x00ff);
                //write proper LOWPAN_NHC
                packetfunctions_reserveHeaderSize(msg,1);
                msg->payload[0] = NHC_UDP_ID|NHC_UDP_PORTS_8S_16D;
                break;
            case NHC_UDP_PORTS_4S_4D:
                // source port: 0xf0b +  4 bits in-line (high 4)
                // dest port:   0xf0b +  4 bits in-line  (low 4)
                packetfunctions_reserveHeaderSize(msg,1);
                msg->payload[0] = (msg->l4_sourcePortORicmpv6Type & 0x000f)<<4;
                msg->payload[0] |= (msg->l4_destination_port & 0x000f);
                //write proper LOWPAN_NHC
                packetfunctions_reserveHeaderSize(msg,1);
                msg->payload[0] = NHC_UDP_ID|NHC_UDP_PORTS_4S_4D;
                break;
        }

        //after filling the packet we calculate the checksum
        packetfunctions_calculateChecksum(msg,checksum_position);

    } else{
        packetfunctions_reserveHeaderSize(msg,sizeof(udp_ht));
        packetfunctions_htons(msg->l4_sourcePortORicmpv6Type,&(msg->payload[0]));
        packetfunctions_htons(msg->l4_destination_port,&(msg->payload[2]));
        //TODO check this as the lenght MUST be ommited.
        packetfunctions_htons(msg->length,&(msg->payload[4]));
        packetfunctions_calculateChecksum(msg,(uint8_t*)&(((udp_ht*)msg->payload)->checksum));
    }
    return forwarding_send(msg);
}

void openudp_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   udp_resource_desc_t* resource;
   udp_callbackSendDone_cbt udp_send_done_callback_ptr = NULL;

   msg->owner = COMPONENT_OPENUDP;

   // iterate list of registered resources
   resource = openudp_vars.resources;
   while (NULL != resource) {
      if (resource->port == msg->l4_sourcePortORicmpv6Type) {
         // there is a registration for this port, either forward the send completion or simply release the message
        udp_send_done_callback_ptr = (resource->callbackSendDone == NULL) ? openudp_sendDone_default_handler
                                                                          : resource->callbackSendDone;
         break;
      }
      resource = resource->next;
   }

   if (udp_send_done_callback_ptr == NULL) {
      openserial_printError(COMPONENT_OPENUDP,ERR_UNSUPPORTED_PORT_NUMBER,
                            (errorparameter_t)msg->l4_sourcePortORicmpv6Type,
                            (errorparameter_t)5);
      openqueue_freePacketBuffer(msg);
      return;
   }

   // handle send completion
   udp_send_done_callback_ptr(msg, error);
}

void openudp_receive(OpenQueueEntry_t* msg) {
   uint8_t temp_8b;
   udp_resource_desc_t* resource;
   udp_callbackReceive_cbt udp_receive_done_callback_ptr = NULL;

   msg->owner = COMPONENT_OPENUDP;

   if (msg->l4_protocol_compressed==TRUE) {
      // get the UDP header encoding byte
      temp_8b = *((uint8_t*)(msg->payload));
      packetfunctions_tossHeader(msg,sizeof(temp_8b));
      switch (temp_8b & NHC_UDP_PORTS_MASK) {
         case NHC_UDP_PORTS_INLINE:
            // source port:         16 bits in-line
            // dest port:           16 bits in-line
            msg->l4_sourcePortORicmpv6Type  = msg->payload[0]*256+msg->payload[1];
            msg->l4_destination_port        = msg->payload[2]*256+msg->payload[3];
            packetfunctions_tossHeader(msg,2+2);
            break;
         case NHC_UDP_PORTS_16S_8D:
            // source port:         16 bits in-line
            // dest port:   0xf0  +  8 bits in-line
            msg->l4_sourcePortORicmpv6Type  = msg->payload[0]*256+msg->payload[1];
            msg->l4_destination_port        = 0xf000 +            msg->payload[2];
            packetfunctions_tossHeader(msg,2+1);
            break;
         case NHC_UDP_PORTS_8S_16D:
            // source port: 0xf0  +  8 bits in-line
            // dest port:   0xf0  +  8 bits in-line
            msg->l4_sourcePortORicmpv6Type  = 0xf000 +            msg->payload[0];
            msg->l4_destination_port        = msg->payload[1]*256+msg->payload[2];
            packetfunctions_tossHeader(msg,1+2);
            break;
         case NHC_UDP_PORTS_4S_4D:
            // source port: 0xf0b +  4 bits in-line
            // dest port:   0xf0b +  4 bits in-line
            msg->l4_sourcePortORicmpv6Type  = 0xf0b0 + ((msg->payload[0] >> 4) & 0x0f);
            msg->l4_destination_port        = 0xf0b0 + ((msg->payload[0] >> 0) & 0x0f);
            packetfunctions_tossHeader(msg,1);
            break;
      }
   } else {
      msg->l4_sourcePortORicmpv6Type  = msg->payload[0]*256+msg->payload[1];
      msg->l4_destination_port        = msg->payload[2]*256+msg->payload[3];
      packetfunctions_tossHeader(msg,sizeof(udp_ht));
   }
   
   // iterate list of registered resources
   resource = openudp_vars.resources;
   while (NULL != resource) {
      if (resource->port == msg->l4_destination_port) {
        udp_receive_done_callback_ptr = (resource->callbackReceive == NULL) ? openudp_receive_default_handler
                                                                            : resource->callbackReceive;
         break;
      }
      resource = resource->next;
   }

   if (udp_receive_done_callback_ptr == NULL) {
      openserial_printError(COMPONENT_OPENUDP,ERR_UNSUPPORTED_PORT_NUMBER,
                            (errorparameter_t)msg->l4_destination_port,
                            (errorparameter_t)6);
      openqueue_freePacketBuffer(msg);
   } else {
      // forward message to resource
      udp_receive_done_callback_ptr(msg);  
   }
}

bool openudp_debugPrint(void) {
   return FALSE;
}

//=========================== private =========================================

static void openudp_sendDone_default_handler(OpenQueueEntry_t* msg, owerror_t error) {
   openqueue_freePacketBuffer(msg);
}

static void openudp_receive_default_handler(OpenQueueEntry_t* msg) {
   openqueue_freePacketBuffer(msg);
}


