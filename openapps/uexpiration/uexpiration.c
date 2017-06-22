#include "stdlib.h"
#include "opendefs.h"
#include "uexpiration.h"
#include "openqueue.h"
#include "openserial.h"
#include "packetfunctions.h"
#include "scheduler.h"

//=========================== variables =======================================

uexpiration_vars_t uexpiration_vars;
uint16_t seqno = 0;
static uint16_t d_flag = 0, delay = 0,max_num_pkts = 0,pkt_interval = 0;
OpenQueueEntry_t req;

//=========================== prototypes ======================================

void uexpiration_timer_cb(void);
void uexpiration_task_cb(void);

//=========================== public ==========================================

void uexpiration_init() {
   // clear local variables
   memset(&uexpiration_vars,0,sizeof(uexpiration_vars_t));

   // register at UDP stack
   uexpiration_vars.desc.port              = WKP_UDP_EXPIRATION;
   uexpiration_vars.desc.callbackReceive   = &uexpiration_receive;
   uexpiration_vars.desc.callbackSendDone  = &uexpiration_sendDone;
   openudp_register(&uexpiration_vars.desc);
}

void uexpiration_receive(OpenQueueEntry_t* request) {
   char buffer[5];
   char clr_buffer[5]= "     ";
   uint8_t    index = 0, len = 0, arg_num = 0; 
   uint8_t    arg_len[4];
   
   memcpy(&req, request, sizeof(OpenQueueEntry_t));
 
   while(index<request->length) {
      if(request->payload[index] != ',') {
          len++;
      }
      else{
          arg_len[arg_num] = len;
          arg_num++;
          len = 0;
      }
      index++;
   }
    
   memcpy(&buffer,&request->payload[0],arg_len[0]);   
   pkt_interval =   atoi(buffer);

   memcpy(&buffer,&clr_buffer,5); 
   memcpy(&buffer[5-arg_len[1]],&request->payload[arg_len[0]+1],arg_len[1]);   
   max_num_pkts =   atoi(buffer);  

   memcpy(&buffer,&clr_buffer,5);     
   memcpy(&buffer[5-arg_len[2]],&request->payload[arg_len[0]+arg_len[1]+2],arg_len[2]);   
   delay =   atoi(buffer);
   
   memcpy(&buffer,&clr_buffer,5);     
   memcpy(&buffer[5-arg_len[3]],&request->payload[arg_len[0]+arg_len[1]+arg_len[2]+3],arg_len[3]);   
   d_flag =   atoi(buffer);  
   
   seqno = 0; // Reinitialize on next trigger 
	
	 uexpiration_vars.period = pkt_interval;	  
	 // start periodic timer
   uexpiration_vars.timerId = opentimers_create();
   opentimers_scheduleAbsolute(
       uexpiration_vars.timerId,
       uexpiration_vars.period,
       opentimers_getValue(),
       TIME_MS,
       uexpiration_timer_cb
   );
}

void uexpiration_timer_cb(void){   
   scheduler_push_task(uexpiration_task_cb,TASKPRIO_COAP);
}

void uexpiration_task_cb() {
   uint16_t          temp_l4_destination_port;
   OpenQueueEntry_t* reply;
   
   
   reply = openqueue_getFreePacketBuffer(COMPONENT_UEXPIRATION);
   if (reply==NULL) {
	    openserial_printError(
	       COMPONENT_UEXPIRATION,
	       ERR_NO_FREE_PACKET_BUFFER,
	       (errorparameter_t)0,
	       (errorparameter_t)0
	    );
	    return;
   }
   
   reply->owner                         = COMPONENT_UEXPIRATION;
   reply->creator                       = COMPONENT_UEXPIRATION;
   
   //Deadline header parameters
   reply->max_delay                     = delay; /* Max delay(in ms) before which the packet should reach the receiver */   
   reply->orgination_time_flag          = 1; /* Origination Time present ? */
   reply->drop_flag                     = d_flag; /* Packet to be dropped if time expires */
   
   reply->l4_protocol                   = IANA_UDP;
   temp_l4_destination_port             = req.l4_destination_port;
   reply->l4_destination_port           = req.l4_sourcePortORicmpv6Type;
   reply->l4_sourcePortORicmpv6Type     = temp_l4_destination_port;
   reply->l3_destinationAdd.type        = ADDR_128B;
   memcpy(&reply->l3_destinationAdd.addr_128b[0],&req.l3_sourceAdd.addr_128b[0],16);  
   
   // Seq number in payload
   packetfunctions_reserveHeaderSize(reply,sizeof(uint16_t));
   reply->payload[1] = (uint8_t)((seqno & 0xff00)>>8);
   reply->payload[0] = (uint8_t)(seqno & 0x00ff); 
       
   //To stop periodic txn of data
   if(++seqno > max_num_pkts) {
   		opentimers_destroy(uexpiration_vars.timerId);
   } else {
      opentimers_scheduleAbsolute(
         uexpiration_vars.timerId,
         uexpiration_vars.period,
         opentimers_getValue(),
         TIME_MS,
         uexpiration_timer_cb
     );
   }
   
   if ((openudp_send(reply))==E_FAIL) {
	  openqueue_freePacketBuffer(reply);
   }

}


void uexpiration_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openqueue_freePacketBuffer(msg);
}


//=========================== private =========================================
