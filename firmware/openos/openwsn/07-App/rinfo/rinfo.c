#include "openwsn.h"
#include "rinfo.h"
#include "opencoap.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "openrandom.h"
#include "board.h"
#include "idmanager.h"

//=========================== defines =========================================

const uint8_t rinfo_path0[] = "i";

//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t desc;
} rinfo_vars_t;

rinfo_vars_t rinfo_vars;

//=========================== prototypes ======================================

error_t rinfo_receive(OpenQueueEntry_t* msg,
                      coap_header_iht*  coap_header,
                      coap_option_iht*  coap_options);
void    rinfo_sendDone(OpenQueueEntry_t* msg,
                       error_t error);

//=========================== public ==========================================

void rinfo_init() {
  
  
   if(idmanager_getIsDAGroot()==TRUE) return; 
   // prepare the resource descriptor for the /temp path
   rinfo_vars.desc.path0len             = sizeof(rinfo_path0)-1;
   rinfo_vars.desc.path0val             = (uint8_t*)(&rinfo_path0);
   rinfo_vars.desc.path1len             = 0;
   rinfo_vars.desc.path1val             = NULL;
   rinfo_vars.desc.componentID          = COMPONENT_RINFO;
   rinfo_vars.desc.callbackRx           = &rinfo_receive;
   rinfo_vars.desc.callbackSendDone     = &rinfo_sendDone;
   
   opencoap_register(&rinfo_vars.desc);
}

//=========================== private =========================================

error_t rinfo_receive(OpenQueueEntry_t* msg,
                      coap_header_iht* coap_header,
                      coap_option_iht* coap_options) {
   error_t outcome;
   
   if (coap_header->Code==COAP_CODE_REQ_GET) {
      
      // reset packet payload
      msg->payload                     = &(msg->packet[127]);
      msg->length                      = 0;
      
      // CoAP payload
      //==radio name
      packetfunctions_reserveHeaderSize(msg,sizeof(infoRadioName)-1);
      memcpy(&msg->payload[0],&infoRadioName,sizeof(infoRadioName)-1);
      //==uC name
      packetfunctions_reserveHeaderSize(msg,1);
      msg->payload[0] = '\n';
      packetfunctions_reserveHeaderSize(msg,sizeof(infouCName)-1);
      memcpy(&msg->payload[0],&infouCName,sizeof(infouCName)-1);
      //==board name
      packetfunctions_reserveHeaderSize(msg,1);
      msg->payload[0] = '\n';
      packetfunctions_reserveHeaderSize(msg,sizeof(infoBoardname)-1);
      memcpy(&msg->payload[0],&infoBoardname,sizeof(infoBoardname)-1);
      //==stackname
      packetfunctions_reserveHeaderSize(msg,1);
      msg->payload[0] = '\n';
      packetfunctions_reserveHeaderSize(msg,sizeof(infoStackName)-1+5);
      memcpy(&msg->payload[0],&infoStackName,sizeof(infoStackName)-1);
      msg->payload[sizeof(infoStackName)-1+5-5] = '0'+OPENWSN_VERSION_MAJOR;
      msg->payload[sizeof(infoStackName)-1+5-4] = '.';
      msg->payload[sizeof(infoStackName)-1+5-3] = '0'+OPENWSN_VERSION_MINOR;
      msg->payload[sizeof(infoStackName)-1+5-2] = '.';
      msg->payload[sizeof(infoStackName)-1+5-1] = '0'+OPENWSN_VERSION_PATCH;
         
      // set the CoAP header
      coap_header->OC                  = 0;
      coap_header->Code                = COAP_CODE_RESP_CONTENT;
      
      outcome                          = E_SUCCESS;
   
   } else {
      // return an error message
      outcome = E_FAIL;
   }
   
   return outcome;
}

void rinfo_sendDone(OpenQueueEntry_t* msg, error_t error) {
   openqueue_freePacketBuffer(msg);
}