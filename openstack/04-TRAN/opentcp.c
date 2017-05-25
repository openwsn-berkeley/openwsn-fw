#include "opendefs.h"
#include "opentcp.h"
#include "openserial.h"
#include "openqueue.h"
#include "forwarding.h"
#include "packetfunctions.h"
#include "bsp_timer.h"
#include "scheduler.h"
#include "opentimers.h"
// applications
#include "techo.h"

//=========================== variables =======================================

tcp_vars_t tcp_vars;

//=========================== prototypes ======================================

void prependTCPHeader(OpenQueueEntry_t* msg, bool ack, bool push, bool rst, bool syn, bool fin);
bool containsControlBits(OpenQueueEntry_t* msg, uint8_t ack, uint8_t rst, uint8_t syn, uint8_t fin);
void tcp_change_state(uint8_t new_state);
void opentcp_reset(void);
void opentcp_timer_cb(opentimer_id_t id);

//=========================== public ==========================================

void opentcp_init() {
   // reset local variables
   memset(&tcp_vars,0,sizeof(tcp_vars_t));   
   // reset state machine
   opentcp_reset();
}

owerror_t opentcp_connect(open_addr_t* dest, uint16_t param_tcp_hisPort, uint16_t param_tcp_myPort) {
   //[command] establishment
   OpenQueueEntry_t* tempPkt;
   if (tcp_vars.state!=TCP_STATE_CLOSED) {
      openserial_printError(COMPONENT_OPENTCP,ERR_WRONG_TCP_STATE,
                            (errorparameter_t)tcp_vars.state,
                            (errorparameter_t)0);
      return E_FAIL;
   }
   tcp_vars.myPort  = param_tcp_myPort;
   tcp_vars.hisPort = param_tcp_hisPort;
   memcpy(&tcp_vars.hisIPv6Address,dest,sizeof(open_addr_t));
   //I receive command 'connect', I send SYNC
   tempPkt = openqueue_getFreePacketBuffer(COMPONENT_OPENTCP);
   if (tempPkt==NULL) {
      openserial_printError(COMPONENT_OPENTCP,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      return E_FAIL;
   }
   tempPkt->creator                = COMPONENT_OPENTCP;
   tempPkt->owner                  = COMPONENT_OPENTCP;
   memcpy(&(tempPkt->l3_destinationAdd),&tcp_vars.hisIPv6Address,sizeof(open_addr_t));
   tcp_vars.mySeqNum = TCP_INITIAL_SEQNUM;
   prependTCPHeader(tempPkt,
         TCP_ACK_NO,
         TCP_PSH_NO,
         TCP_RST_NO,
         TCP_SYN_YES,
         TCP_FIN_NO);
   tcp_change_state(TCP_STATE_ALMOST_SYN_SENT);
   return forwarding_send(tempPkt);
}

owerror_t opentcp_send(OpenQueueEntry_t* msg) {             //[command] data
   msg->owner = COMPONENT_OPENTCP;
   if (tcp_vars.state!=TCP_STATE_ESTABLISHED) {
      openserial_printError(COMPONENT_OPENTCP,ERR_WRONG_TCP_STATE,
                            (errorparameter_t)tcp_vars.state,
                            (errorparameter_t)2);
      return E_FAIL;
   }
   if (tcp_vars.dataToSend!=NULL) {
      openserial_printError(COMPONENT_OPENTCP,ERR_BUSY_SENDING,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      return E_FAIL;
   }
   //I receive command 'send', I send data
   msg->l4_protocol          = IANA_TCP;
   msg->l4_sourcePortORicmpv6Type       = tcp_vars.myPort;
   msg->l4_destination_port  = tcp_vars.hisPort;
   msg->l4_payload           = msg->payload;
   msg->l4_length            = msg->length;
   memcpy(&(msg->l3_destinationAdd),&tcp_vars.hisIPv6Address,sizeof(open_addr_t));
   tcp_vars.dataToSend = msg;
   prependTCPHeader(tcp_vars.dataToSend,
         TCP_ACK_YES,
         TCP_PSH_YES,
         TCP_RST_NO,
         TCP_SYN_NO,
         TCP_FIN_NO);
   tcp_vars.mySeqNum += tcp_vars.dataToSend->l4_length;
   tcp_change_state(TCP_STATE_ALMOST_DATA_SENT);
   return forwarding_send(tcp_vars.dataToSend);
}

void opentcp_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   OpenQueueEntry_t* tempPkt;
   msg->owner = COMPONENT_OPENTCP;
   switch (tcp_vars.state) {
      case TCP_STATE_ALMOST_SYN_SENT:                             //[sendDone] establishement
         openqueue_freePacketBuffer(msg);
         tcp_change_state(TCP_STATE_SYN_SENT);
         break;

      case TCP_STATE_ALMOST_SYN_RECEIVED:                         //[sendDone] establishement
         openqueue_freePacketBuffer(msg);
         tcp_change_state(TCP_STATE_SYN_RECEIVED);
         break;

      case TCP_STATE_ALMOST_ESTABLISHED:                          //[sendDone] establishement
         openqueue_freePacketBuffer(msg);
         tcp_change_state(TCP_STATE_ESTABLISHED);
         switch(tcp_vars.myPort) {
            case WKP_TCP_ECHO:
               techo_connectDone(E_SUCCESS);
               break;
            default:
               openserial_printError(COMPONENT_OPENTCP,ERR_UNSUPPORTED_PORT_NUMBER,
                                     (errorparameter_t)tcp_vars.myPort,
                                     (errorparameter_t)0);
               break;
         }
         break;

      case TCP_STATE_ALMOST_DATA_SENT:                            //[sendDone] data
         tcp_change_state(TCP_STATE_DATA_SENT);
         break;

      case TCP_STATE_ALMOST_DATA_RECEIVED:                        //[sendDone] data
         openqueue_freePacketBuffer(msg);
         tcp_change_state(TCP_STATE_ESTABLISHED);
         switch(tcp_vars.myPort) {
            case WKP_TCP_ECHO:
               techo_receive(tcp_vars.dataReceived);
               break;
            default:
               openserial_printError(COMPONENT_OPENTCP,ERR_UNSUPPORTED_PORT_NUMBER,
                                     (errorparameter_t)tcp_vars.myPort,
                                     (errorparameter_t)1);
               openqueue_freePacketBuffer(msg);
               tcp_vars.dataReceived = NULL;
               break;
         }
         break;

      case TCP_STATE_ALMOST_FIN_WAIT_1:                           //[sendDone] teardown
         openqueue_freePacketBuffer(msg);
         tcp_change_state(TCP_STATE_FIN_WAIT_1);
         break;

      case TCP_STATE_ALMOST_CLOSING:                              //[sendDone] teardown
         openqueue_freePacketBuffer(msg);
         tcp_change_state(TCP_STATE_CLOSING);
         break;

      case TCP_STATE_ALMOST_TIME_WAIT:                            //[sendDone] teardown
         openqueue_freePacketBuffer(msg);
         tcp_change_state(TCP_STATE_TIME_WAIT);
         //TODO implement waiting timer
         opentcp_reset();
         break;

      case TCP_STATE_ALMOST_CLOSE_WAIT:                           //[sendDone] teardown
         openqueue_freePacketBuffer(msg);
         tcp_change_state(TCP_STATE_CLOSE_WAIT);
         //I send FIN+ACK
         tempPkt = openqueue_getFreePacketBuffer(COMPONENT_OPENTCP);
         if (tempPkt==NULL) {
            openserial_printError(COMPONENT_OPENTCP,ERR_NO_FREE_PACKET_BUFFER,
                                  (errorparameter_t)0,
                                  (errorparameter_t)0);
            openqueue_freePacketBuffer(msg);
            return;
         }
         tempPkt->creator       = COMPONENT_OPENTCP;
         tempPkt->owner         = COMPONENT_OPENTCP;
         memcpy(&(tempPkt->l3_destinationAdd),&tcp_vars.hisIPv6Address,sizeof(open_addr_t));
         prependTCPHeader(tempPkt,
               TCP_ACK_YES,
               TCP_PSH_NO,
               TCP_RST_NO,
               TCP_SYN_NO,
               TCP_FIN_YES);
         forwarding_send(tempPkt);
         tcp_change_state(TCP_STATE_ALMOST_LAST_ACK);
         break;

      case TCP_STATE_ALMOST_LAST_ACK:                             //[sendDone] teardown
         openqueue_freePacketBuffer(msg);
         tcp_change_state(TCP_STATE_LAST_ACK);
         break;

      default:
         openserial_printError(COMPONENT_OPENTCP,ERR_WRONG_TCP_STATE,
                               (errorparameter_t)tcp_vars.state,
                               (errorparameter_t)3);
         break;
   }
}

void opentcp_receive(OpenQueueEntry_t* msg) {
   OpenQueueEntry_t* tempPkt;
   bool shouldIlisten;
   msg->owner                     = COMPONENT_OPENTCP;
   msg->l4_protocol               = IANA_TCP;
   msg->l4_payload                = msg->payload;
   msg->l4_length                 = msg->length;
   msg->l4_sourcePortORicmpv6Type = packetfunctions_ntohs((uint8_t*)&(((tcp_ht*)msg->payload)->source_port));
   msg->l4_destination_port       = packetfunctions_ntohs((uint8_t*)&(((tcp_ht*)msg->payload)->destination_port));
   if ( 
         tcp_vars.state!=TCP_STATE_CLOSED &&
         (
          msg->l4_destination_port != tcp_vars.myPort  ||
          msg->l4_sourcePortORicmpv6Type      != tcp_vars.hisPort ||
          packetfunctions_sameAddress(&(msg->l3_destinationAdd),&tcp_vars.hisIPv6Address)==FALSE
         )
      ) {
      openqueue_freePacketBuffer(msg);
      return;
   }
   if (containsControlBits(msg,TCP_ACK_WHATEVER,TCP_RST_YES,TCP_SYN_WHATEVER,TCP_FIN_WHATEVER)) {
      //I receive RST[+*], I reset
      opentcp_reset();
      openqueue_freePacketBuffer(msg);
   }
   switch (tcp_vars.state) {
      case TCP_STATE_CLOSED:                                      //[receive] establishement
         switch(msg->l4_destination_port) {
            case WKP_TCP_ECHO:
               shouldIlisten = techo_shouldIlisten();
               break;
            default:
               openserial_printError(COMPONENT_OPENTCP,ERR_UNSUPPORTED_PORT_NUMBER,
                                     (errorparameter_t)msg->l4_sourcePortORicmpv6Type,
                                     (errorparameter_t)2);
               shouldIlisten = FALSE;
               break;
         }
         if ( containsControlBits(msg,TCP_ACK_NO,TCP_RST_NO,TCP_SYN_YES,TCP_FIN_NO) && shouldIlisten==TRUE ) {
                  tcp_vars.myPort = msg->l4_destination_port;
                  //I receive SYN, I send SYN+ACK
                  tcp_vars.hisNextSeqNum = (packetfunctions_ntohl((uint8_t*)&(((tcp_ht*)msg->payload)->sequence_number)))+1;
                  tcp_vars.hisPort       = msg->l4_sourcePortORicmpv6Type;
                  memcpy(&tcp_vars.hisIPv6Address,&(msg->l3_destinationAdd),sizeof(open_addr_t));
                  tempPkt       = openqueue_getFreePacketBuffer(COMPONENT_OPENTCP);
                  if (tempPkt==NULL) {
                     openserial_printError(COMPONENT_OPENTCP,ERR_NO_FREE_PACKET_BUFFER,
                                           (errorparameter_t)0,
                                           (errorparameter_t)0);
                     openqueue_freePacketBuffer(msg);
                     return;
                  }
                  tempPkt->creator       = COMPONENT_OPENTCP;
                  tempPkt->owner         = COMPONENT_OPENTCP;
                  memcpy(&(tempPkt->l3_destinationAdd),&tcp_vars.hisIPv6Address,sizeof(open_addr_t));
                  prependTCPHeader(tempPkt,
                        TCP_ACK_YES,
                        TCP_PSH_NO,
                        TCP_RST_NO,
                        TCP_SYN_YES,
                        TCP_FIN_NO);
                  tcp_vars.mySeqNum++;
                  tcp_change_state(TCP_STATE_ALMOST_SYN_RECEIVED);
                  forwarding_send(tempPkt);
               } else {
                  opentcp_reset();
                  openserial_printError(COMPONENT_OPENTCP,ERR_TCP_RESET,
                                        (errorparameter_t)tcp_vars.state,
                                        (errorparameter_t)0);
               }
         openqueue_freePacketBuffer(msg);
         break;

      case TCP_STATE_SYN_SENT:                                    //[receive] establishement
         if (containsControlBits(msg,TCP_ACK_YES,TCP_RST_NO,TCP_SYN_YES,TCP_FIN_NO)) {
            //I receive SYN+ACK, I send ACK
            tcp_vars.hisNextSeqNum = (packetfunctions_ntohl((uint8_t*)&(((tcp_ht*)msg->payload)->sequence_number)))+1;
            tempPkt = openqueue_getFreePacketBuffer(COMPONENT_OPENTCP);
            if (tempPkt==NULL) {
               openserial_printError(COMPONENT_OPENTCP,ERR_NO_FREE_PACKET_BUFFER,
                                     (errorparameter_t)0,
                                     (errorparameter_t)0);
               openqueue_freePacketBuffer(msg);
               return;
            }
            tempPkt->creator       = COMPONENT_OPENTCP;
            tempPkt->owner         = COMPONENT_OPENTCP;
            memcpy(&(tempPkt->l3_destinationAdd),&tcp_vars.hisIPv6Address,sizeof(open_addr_t));
            prependTCPHeader(tempPkt,
                  TCP_ACK_YES,
                  TCP_PSH_NO,
                  TCP_RST_NO,
                  TCP_SYN_NO,
                  TCP_FIN_NO);
            tcp_change_state(TCP_STATE_ALMOST_ESTABLISHED);
            forwarding_send(tempPkt);
         } else if (containsControlBits(msg,TCP_ACK_NO,TCP_RST_NO,TCP_SYN_YES,TCP_FIN_NO)) {
            //I receive SYN, I send SYN+ACK
            tcp_vars.hisNextSeqNum = (packetfunctions_ntohl((uint8_t*)&(((tcp_ht*)msg->payload)->sequence_number)))+1;
            tempPkt       = openqueue_getFreePacketBuffer(COMPONENT_OPENTCP);
            if (tempPkt==NULL) {
               openserial_printError(COMPONENT_OPENTCP,ERR_NO_FREE_PACKET_BUFFER,
                                     (errorparameter_t)0,
                                     (errorparameter_t)0);
               openqueue_freePacketBuffer(msg);
               return;
            }
            tempPkt->creator       = COMPONENT_OPENTCP;
            tempPkt->owner         = COMPONENT_OPENTCP;
            memcpy(&(tempPkt->l3_destinationAdd),&tcp_vars.hisIPv6Address,sizeof(open_addr_t));
            prependTCPHeader(tempPkt,
                  TCP_ACK_YES,
                  TCP_PSH_NO,
                  TCP_RST_NO,
                  TCP_SYN_YES,
                  TCP_FIN_NO);
            tcp_vars.mySeqNum++;
            tcp_change_state(TCP_STATE_ALMOST_SYN_RECEIVED);
            forwarding_send(tempPkt);
         } else {
            opentcp_reset();
            openserial_printError(COMPONENT_OPENTCP,ERR_TCP_RESET,
                                  (errorparameter_t)tcp_vars.state,
                                  (errorparameter_t)1);
         }
         openqueue_freePacketBuffer(msg);
         break;

      case TCP_STATE_SYN_RECEIVED:                                //[receive] establishement
         if (containsControlBits(msg,TCP_ACK_YES,TCP_RST_NO,TCP_SYN_NO,TCP_FIN_NO)) {
            //I receive ACK, the virtual circuit is established
            tcp_change_state(TCP_STATE_ESTABLISHED);
         } else {
            opentcp_reset();
            openserial_printError(COMPONENT_OPENTCP,ERR_TCP_RESET,
                                  (errorparameter_t)tcp_vars.state,
                                  (errorparameter_t)2);
         }
         openqueue_freePacketBuffer(msg);
         break;

      case TCP_STATE_ESTABLISHED:                                 //[receive] data/teardown
         if (containsControlBits(msg,TCP_ACK_WHATEVER,TCP_RST_NO,TCP_SYN_NO,TCP_FIN_YES)) {
            //I receive FIN[+ACK], I send ACK
            tcp_vars.hisNextSeqNum = (packetfunctions_ntohl((uint8_t*)&(((tcp_ht*)msg->payload)->sequence_number)))+msg->length-sizeof(tcp_ht)+1;
            tempPkt = openqueue_getFreePacketBuffer(COMPONENT_OPENTCP);
            if (tempPkt==NULL) {
               openserial_printError(COMPONENT_OPENTCP,ERR_NO_FREE_PACKET_BUFFER,
                                     (errorparameter_t)0,
                                     (errorparameter_t)0);
               openqueue_freePacketBuffer(msg);
               return;
            }
            tempPkt->creator       = COMPONENT_OPENTCP;
            tempPkt->owner         = COMPONENT_OPENTCP;
            memcpy(&(tempPkt->l3_destinationAdd),&tcp_vars.hisIPv6Address,sizeof(open_addr_t));
            prependTCPHeader(tempPkt,
                  TCP_ACK_YES,
                  TCP_PSH_NO,
                  TCP_RST_NO,
                  TCP_SYN_NO,
                  TCP_FIN_NO);
            forwarding_send(tempPkt);
            tcp_change_state(TCP_STATE_ALMOST_CLOSE_WAIT);
         } else if (containsControlBits(msg,TCP_ACK_WHATEVER,TCP_RST_NO,TCP_SYN_NO,TCP_FIN_NO)) {
            //I receive data, I send ACK
            tcp_vars.hisNextSeqNum = (packetfunctions_ntohl((uint8_t*)&(((tcp_ht*)msg->payload)->sequence_number)))+msg->length-sizeof(tcp_ht);
            tempPkt = openqueue_getFreePacketBuffer(COMPONENT_OPENTCP);
            if (tempPkt==NULL) {
               openserial_printError(COMPONENT_OPENTCP,ERR_NO_FREE_PACKET_BUFFER,
                                     (errorparameter_t)0,
                                     (errorparameter_t)0);
               openqueue_freePacketBuffer(msg);
               return;
            }
            tempPkt->creator       = COMPONENT_OPENTCP;
            tempPkt->owner         = COMPONENT_OPENTCP;
            memcpy(&(tempPkt->l3_destinationAdd),&tcp_vars.hisIPv6Address,sizeof(open_addr_t));
            prependTCPHeader(tempPkt,
                  TCP_ACK_YES,
                  TCP_PSH_NO,
                  TCP_RST_NO,
                  TCP_SYN_NO,
                  TCP_FIN_NO);
            forwarding_send(tempPkt);
            packetfunctions_tossHeader(msg,sizeof(tcp_ht));
            tcp_vars.dataReceived = msg;
            tcp_change_state(TCP_STATE_ALMOST_DATA_RECEIVED);
         } else {
            opentcp_reset();
            openserial_printError(COMPONENT_OPENTCP,ERR_TCP_RESET,
                                  (errorparameter_t)tcp_vars.state,
                                  (errorparameter_t)3);
            openqueue_freePacketBuffer(msg);
         }
         break;

      case TCP_STATE_DATA_SENT:                                   //[receive] data
         if (containsControlBits(msg,TCP_ACK_YES,TCP_RST_NO,TCP_SYN_NO,TCP_FIN_NO)) {
            //I receive ACK, data message sent
            switch(tcp_vars.myPort) {
               case WKP_TCP_ECHO:
                  techo_sendDone(tcp_vars.dataToSend,E_SUCCESS);
                  break;
               default:
                  openserial_printError(COMPONENT_OPENTCP,ERR_UNSUPPORTED_PORT_NUMBER,
                                        (errorparameter_t)tcp_vars.myPort,
                                        (errorparameter_t)3);
                  break;
            }
            tcp_vars.dataToSend = NULL;
            tcp_change_state(TCP_STATE_ESTABLISHED);
         } else if (containsControlBits(msg,TCP_ACK_WHATEVER,TCP_RST_NO,TCP_SYN_NO,TCP_FIN_YES)) {
            //I receive FIN[+ACK], I send ACK
            switch(tcp_vars.myPort) {
               case WKP_TCP_ECHO:
                  techo_sendDone(tcp_vars.dataToSend,E_SUCCESS);
                  break;
               default:
                  openserial_printError(COMPONENT_OPENTCP,ERR_UNSUPPORTED_PORT_NUMBER,
                                        (errorparameter_t)tcp_vars.myPort,
                                        (errorparameter_t)4);
                  break;
            }
            tcp_vars.dataToSend = NULL;
            tcp_vars.hisNextSeqNum = (packetfunctions_ntohl((uint8_t*)&(((tcp_ht*)msg->payload)->sequence_number)))+msg->length-sizeof(tcp_ht)+1;
            tempPkt = openqueue_getFreePacketBuffer(COMPONENT_OPENTCP);
            if (tempPkt==NULL) {
               openserial_printError(COMPONENT_OPENTCP,ERR_NO_FREE_PACKET_BUFFER,
                                     (errorparameter_t)0,
                                     (errorparameter_t)0);
               openqueue_freePacketBuffer(msg);
               return;
            }
            tempPkt->creator       = COMPONENT_OPENTCP;
            tempPkt->owner         = COMPONENT_OPENTCP;
            memcpy(&(tempPkt->l3_destinationAdd),&tcp_vars.hisIPv6Address,sizeof(open_addr_t));
            prependTCPHeader(tempPkt,
                  TCP_ACK_YES,
                  TCP_PSH_NO,
                  TCP_RST_NO,
                  TCP_SYN_NO,
                  TCP_FIN_NO);
            forwarding_send(tempPkt);
            tcp_change_state(TCP_STATE_ALMOST_CLOSE_WAIT);
         } else {
            opentcp_reset();
            openserial_printError(COMPONENT_OPENTCP,ERR_TCP_RESET,
                                  (errorparameter_t)tcp_vars.state,
                                  (errorparameter_t)4);
         }
         openqueue_freePacketBuffer(msg);
         break;

      case TCP_STATE_FIN_WAIT_1:                                  //[receive] teardown
         if (containsControlBits(msg,TCP_ACK_NO,TCP_RST_NO,TCP_SYN_NO,TCP_FIN_YES)) {
            //I receive FIN, I send ACK
            tcp_vars.hisNextSeqNum = (packetfunctions_ntohl((uint8_t*)&(((tcp_ht*)msg->payload)->sequence_number)))+1;
            tempPkt = openqueue_getFreePacketBuffer(COMPONENT_OPENTCP);
            if (tempPkt==NULL) {
               openserial_printError(COMPONENT_OPENTCP,ERR_NO_FREE_PACKET_BUFFER,
                                     (errorparameter_t)0,
                                     (errorparameter_t)0);
               openqueue_freePacketBuffer(msg);
               return;
            }
            tempPkt->creator       = COMPONENT_OPENTCP;
            tempPkt->owner         = COMPONENT_OPENTCP;
            memcpy(&(tempPkt->l3_destinationAdd),&tcp_vars.hisIPv6Address,sizeof(open_addr_t));
            prependTCPHeader(tempPkt,
                  TCP_ACK_YES,
                  TCP_PSH_NO,
                  TCP_RST_NO,
                  TCP_SYN_NO,
                  TCP_FIN_NO);
            forwarding_send(tempPkt);
            tcp_change_state(TCP_STATE_ALMOST_CLOSING);
         } else if (containsControlBits(msg,TCP_ACK_YES,TCP_RST_NO,TCP_SYN_NO,TCP_FIN_YES)) {
            //I receive FIN+ACK, I send ACK
            tcp_vars.hisNextSeqNum = (packetfunctions_ntohl((uint8_t*)&(((tcp_ht*)msg->payload)->sequence_number)))+1;
            tempPkt = openqueue_getFreePacketBuffer(COMPONENT_OPENTCP);
            if (tempPkt==NULL) {
               openserial_printError(COMPONENT_OPENTCP,ERR_NO_FREE_PACKET_BUFFER,
                                     (errorparameter_t)0,
                                     (errorparameter_t)0);
               openqueue_freePacketBuffer(msg);
               return;
            }
            tempPkt->creator       = COMPONENT_OPENTCP;
            tempPkt->owner         = COMPONENT_OPENTCP;
            memcpy(&(tempPkt->l3_destinationAdd),&tcp_vars.hisIPv6Address,sizeof(open_addr_t));
            prependTCPHeader(tempPkt,
                  TCP_ACK_YES,
                  TCP_PSH_NO,
                  TCP_RST_NO,
                  TCP_SYN_NO,
                  TCP_FIN_NO);
            forwarding_send(tempPkt);
            tcp_change_state(TCP_STATE_ALMOST_TIME_WAIT);
         } else if  (containsControlBits(msg,TCP_ACK_YES,TCP_RST_NO,TCP_SYN_NO,TCP_FIN_NO)) {
            //I receive ACK, I will receive FIN later
            tcp_change_state(TCP_STATE_FIN_WAIT_2);
         } else {
            opentcp_reset();
            openserial_printError(COMPONENT_OPENTCP,ERR_TCP_RESET,
                                  (errorparameter_t)tcp_vars.state,
                                  (errorparameter_t)5);
         }
         openqueue_freePacketBuffer(msg);
         break;

      case TCP_STATE_FIN_WAIT_2:                                  //[receive] teardown
         if (containsControlBits(msg,TCP_ACK_WHATEVER,TCP_RST_NO,TCP_SYN_NO,TCP_FIN_YES)) {
            //I receive FIN[+ACK], I send ACK
            tcp_vars.hisNextSeqNum = (packetfunctions_ntohl((uint8_t*)&(((tcp_ht*)msg->payload)->sequence_number)))+1;
            tempPkt = openqueue_getFreePacketBuffer(COMPONENT_OPENTCP);
            if (tempPkt==NULL) {
               openserial_printError(COMPONENT_OPENTCP,ERR_NO_FREE_PACKET_BUFFER,
                                     (errorparameter_t)0,
                                     (errorparameter_t)0);
               openqueue_freePacketBuffer(msg);
               return;
            }
            tempPkt->creator       = COMPONENT_OPENTCP;
            tempPkt->owner         = COMPONENT_OPENTCP;
            memcpy(&(tempPkt->l3_destinationAdd),&tcp_vars.hisIPv6Address,sizeof(open_addr_t));
            prependTCPHeader(tempPkt,
                  TCP_ACK_YES,
                  TCP_PSH_NO,
                  TCP_RST_NO,
                  TCP_SYN_NO,
                  TCP_FIN_NO);
            forwarding_send(tempPkt);
            tcp_change_state(TCP_STATE_ALMOST_TIME_WAIT);
         }
         openqueue_freePacketBuffer(msg);
         break;

      case TCP_STATE_CLOSING:                                     //[receive] teardown
         if (containsControlBits(msg,TCP_ACK_YES,TCP_RST_NO,TCP_SYN_NO,TCP_FIN_NO)) {
            //I receive ACK, I do nothing
            tcp_change_state(TCP_STATE_TIME_WAIT);
            //TODO implement waiting timer
            opentcp_reset();
         }
         openqueue_freePacketBuffer(msg);
         break;

      case TCP_STATE_LAST_ACK:                                    //[receive] teardown
         if (containsControlBits(msg,TCP_ACK_YES,TCP_RST_NO,TCP_SYN_NO,TCP_FIN_NO)) {
            //I receive ACK, I reset
            opentcp_reset();
         }
         openqueue_freePacketBuffer(msg);
         break;

      default:
         openserial_printError(COMPONENT_OPENTCP,ERR_WRONG_TCP_STATE,
                               (errorparameter_t)tcp_vars.state,
                               (errorparameter_t)4);
         break;
   }
}

owerror_t opentcp_close() {    //[command] teardown
   OpenQueueEntry_t* tempPkt;
   if (  tcp_vars.state==TCP_STATE_ALMOST_CLOSE_WAIT ||
         tcp_vars.state==TCP_STATE_CLOSE_WAIT        ||
         tcp_vars.state==TCP_STATE_ALMOST_LAST_ACK   ||
         tcp_vars.state==TCP_STATE_LAST_ACK          ||
         tcp_vars.state==TCP_STATE_CLOSED) {
      //not an error, can happen when distant node has already started tearing down
      return E_SUCCESS;
   }
   //I receive command 'close', I send FIN+ACK
   tempPkt = openqueue_getFreePacketBuffer(COMPONENT_OPENTCP);
   if (tempPkt==NULL) {
      openserial_printError(COMPONENT_OPENTCP,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      return E_FAIL;
   }
   tempPkt->creator       = COMPONENT_OPENTCP;
   tempPkt->owner         = COMPONENT_OPENTCP;
   memcpy(&(tempPkt->l3_destinationAdd),&tcp_vars.hisIPv6Address,sizeof(open_addr_t));
   prependTCPHeader(tempPkt,
         TCP_ACK_YES,
         TCP_PSH_NO,
         TCP_RST_NO,
         TCP_SYN_NO,
         TCP_FIN_YES);
   tcp_vars.mySeqNum++;
   tcp_change_state(TCP_STATE_ALMOST_FIN_WAIT_1);
   return forwarding_send(tempPkt);
}

bool tcp_debugPrint(void) {
   return FALSE;
}

//======= timer

//timer used to reset state when TCP state machine is stuck
void timers_tcp_fired(void) {
   opentcp_reset();
}

//=========================== private =========================================

void prependTCPHeader(OpenQueueEntry_t* msg,
      bool ack,
      bool push,
      bool rst,
      bool syn,
      bool fin) {
   msg->l4_protocol = IANA_TCP;
   packetfunctions_reserveHeaderSize(msg,sizeof(tcp_ht));
   packetfunctions_htons(tcp_vars.myPort        ,(uint8_t*)&(((tcp_ht*)msg->payload)->source_port));
   packetfunctions_htons(tcp_vars.hisPort       ,(uint8_t*)&(((tcp_ht*)msg->payload)->destination_port));
   packetfunctions_htonl(tcp_vars.mySeqNum      ,(uint8_t*)&(((tcp_ht*)msg->payload)->sequence_number));
   packetfunctions_htonl(tcp_vars.hisNextSeqNum ,(uint8_t*)&(((tcp_ht*)msg->payload)->ack_number));
   ((tcp_ht*)msg->payload)->data_offset      = TCP_DEFAULT_DATA_OFFSET;
   ((tcp_ht*)msg->payload)->control_bits     = 0;
   if (ack==TCP_ACK_YES) {
      ((tcp_ht*)msg->payload)->control_bits |= 1 << TCP_ACK;
   } else {
      packetfunctions_htonl(0,(uint8_t*)&(((tcp_ht*)msg->payload)->ack_number));
   }
   if (push==TCP_PSH_YES) {
      ((tcp_ht*)msg->payload)->control_bits |= 1 << TCP_PSH;
   }
   if (rst==TCP_RST_YES) {
      ((tcp_ht*)msg->payload)->control_bits |= 1 << TCP_RST;
   }
   if (syn==TCP_SYN_YES) {
      ((tcp_ht*)msg->payload)->control_bits |= 1 << TCP_SYN;
   }
   if (fin==TCP_FIN_YES) {
      ((tcp_ht*)msg->payload)->control_bits |= 1 << TCP_FIN;
   }
   packetfunctions_htons(TCP_DEFAULT_WINDOW_SIZE    ,(uint8_t*)&(((tcp_ht*)msg->payload)->window_size));
   packetfunctions_htons(TCP_DEFAULT_URGENT_POINTER ,(uint8_t*)&(((tcp_ht*)msg->payload)->urgent_pointer));
   //calculate checksum last to take all header fields into account
   packetfunctions_calculateChecksum(msg,(uint8_t*)&(((tcp_ht*)msg->payload)->checksum));
}

bool containsControlBits(OpenQueueEntry_t* msg, uint8_t ack, uint8_t rst, uint8_t syn, uint8_t fin) {
   bool return_value = TRUE;
   if (ack!=TCP_ACK_WHATEVER){
      return_value = return_value && ((bool)( (((tcp_ht*)msg->payload)->control_bits >> TCP_ACK) & 0x01) == ack);
   }
   if (rst!=TCP_RST_WHATEVER){
      return_value = return_value && ((bool)( (((tcp_ht*)msg->payload)->control_bits >> TCP_RST) & 0x01) == rst);
   }
   if (syn!=TCP_SYN_WHATEVER){
      return_value = return_value && ((bool)( (((tcp_ht*)msg->payload)->control_bits >> TCP_SYN) & 0x01) == syn);
   }
   if (fin!=TCP_FIN_WHATEVER){
      return_value = return_value && ((bool)( (((tcp_ht*)msg->payload)->control_bits >> TCP_FIN) & 0x01) == fin);
   }
   return return_value;
}

void opentcp_reset() {
   tcp_change_state(TCP_STATE_CLOSED);
   tcp_vars.mySeqNum            = TCP_INITIAL_SEQNUM; 
   tcp_vars.hisNextSeqNum       = 0;
   tcp_vars.hisPort             = 0;
   tcp_vars.hisIPv6Address.type = ADDR_NONE;
   tcp_vars.dataToSend          = NULL;
   tcp_vars.dataReceived        = NULL;
   openqueue_removeAllOwnedBy(COMPONENT_OPENTCP);
}

void tcp_change_state(uint8_t new_tcp_state) {
   tcp_vars.state = new_tcp_state;
   if (tcp_vars.state==TCP_STATE_CLOSED) {
      if (tcp_vars.timerStarted==TRUE) {
         opentimers_stop(tcp_vars.timerId);
      }
   } else {
      if (tcp_vars.timerStarted==FALSE) {
         tcp_vars.timerId = opentimers_start(TCP_TIMEOUT,
                                             TIMER_ONESHOT,TIME_MS,
                                             opentcp_timer_cb);
         tcp_vars.timerStarted=TRUE;
      }
      
   }
}

void opentcp_timer_cb(opentimer_id_t id) {
   scheduler_push_task(timers_tcp_fired,TASKPRIO_TCP_TIMEOUT);
}
