#include "openwsn.h"
#include "icmpv6rpl.h"
#include "icmpv6.h"
#include "openserial.h"
#include "openqueue.h"
#include "neighbors.h"
#include "packetfunctions.h"
#include "openrandom.h"
#include "scheduler.h"
#include "idmanager.h"
#include "opentimers.h"


//=========================== variables =======================================

typedef struct {
   uint16_t        periodDIO;
   uint8_t         delayDIO;
   open_addr_t     all_routers_multicast;
   bool            busySending;
   uint16_t        seq;
   opentimer_id_t  timerId;
   uint8_t         DODAGIDFlagSet; ///< is DODAGID set?
   uint16_t        periodDAO;
   uint8_t         delayDAO;
} icmpv6rpl_vars_t;

icmpv6rpl_vars_t             icmpv6rpl_vars;

icmpv6rpl_dio_t              icmpv6rpl_dio;
icmpv6rpl_dao_t              icmpv6rpl_dao;
//icmpv6rpl_dao_rpl_target_t   icmpv6rpl_dao_rpl_target;
icmpv6rpl_dao_transit_info_t icmpv6rpl_dao_transit_info;
icmpv6rpl_dio_options_t      icmpv6rpl_dio_options;

//=========================== prototypes ======================================

void sendDIO();
void icmpv6rpl_timer_cb();
//==== added by Ahmad ====//
void sendDAO();
void icmpv6rpl_timer_DAO_cb();

//=========================== public ==========================================

void icmpv6rpl_init() {
   icmpv6rpl_vars.busySending     = FALSE;
   icmpv6rpl_vars.seq             = 0;
   
   icmpv6rpl_dio.reserved         = 0;
   icmpv6rpl_dio.flags            = 0;
   icmpv6rpl_dio.DTSN             = 0x33; //?? this values are not correct.
   icmpv6rpl_dio.verNumb          = 0x11; //?? this values are not correct.
   icmpv6rpl_dio.rplinstanceId    = 0x22; //?? this values are not correct.
   icmpv6rpl_dio.rplOptions       = 0x00| MOP_DIO_A | MOP_DIO_B | MOP_DIO_C | PRF_DIO_A | PRF_DIO_B | PRF_DIO_C | G_DIO ;
   
   //set flag to zero first
   icmpv6rpl_vars.DODAGIDFlagSet  = 0;
   // set the default DODAGID
   icmpv6rpl_dio.DODAGID[0]       = 0xaa;
   icmpv6rpl_dio.DODAGID[1]       = 0xaa;
   icmpv6rpl_dio.DODAGID[2]       = 0xbb;
   icmpv6rpl_dio.DODAGID[3]       = 0xbb;
   icmpv6rpl_dio.DODAGID[4]       = 0xcc;
   icmpv6rpl_dio.DODAGID[5]       = 0xcc;
   icmpv6rpl_dio.DODAGID[6]       = 0xdd;
   icmpv6rpl_dio.DODAGID[7]       = 0xdd;
   icmpv6rpl_dio.DODAGID[8]       = 0xaa;
   icmpv6rpl_dio.DODAGID[9]       = 0xaa;
   icmpv6rpl_dio.DODAGID[10]      = 0xbb;
   icmpv6rpl_dio.DODAGID[11]      = 0xbb;
   icmpv6rpl_dio.DODAGID[12]      = 0xcc;
   icmpv6rpl_dio.DODAGID[13]      = 0xcc;
   icmpv6rpl_dio.DODAGID[14]      = 0xdd;
   icmpv6rpl_dio.DODAGID[15]      = 0xdd;
   icmpv6rpl_dio.options          = 0x05;
   
   icmpv6rpl_dao.rplinstanceId    = 0x88;
   //K_D_flags
   icmpv6rpl_dao.K_D_flags        = 0x00| FLAG_DAO_A | FLAG_DAO_B | FLAG_DAO_C | FLAG_DAO_D | FLAG_DAO_E | PRF_DIO_C | FLAG_DAO_F | D_DAO | K_DAO;
   icmpv6rpl_dao.reserved         = 0x00;
   icmpv6rpl_dao.DAOSequance      = 0x99;
   icmpv6rpl_dao.DODAGID[0]       = 0xEE;
   icmpv6rpl_dao.DODAGID[1]       = 0xFF;
   icmpv6rpl_dao.DODAGID[2]       = 0xEE;
   icmpv6rpl_dao.DODAGID[3]       = 0xFF;
   icmpv6rpl_dao.DODAGID[4]       = 0xEE;
   icmpv6rpl_dao.DODAGID[5]       = 0xFF;
   
   icmpv6rpl_dao.DODAGID[6]       = 0xEE;
   icmpv6rpl_dao.DODAGID[7]       = 0xFF;
   icmpv6rpl_dao.DODAGID[8]       = 0xEE;
   icmpv6rpl_dao.DODAGID[9]       = 0xFF;
   icmpv6rpl_dao.DODAGID[10]      = 0xEE;
   icmpv6rpl_dao.DODAGID[11]      = 0xFF;
   icmpv6rpl_dao.DODAGID[12]      = 0xEE;
   icmpv6rpl_dao.DODAGID[13]      = 0xFF;
   icmpv6rpl_dao.DODAGID[14]      = 0xEE;
   icmpv6rpl_dao.DODAGID[15]      = 0xFF;
   icmpv6rpl_dao.options          = 0x07;
   
   //icmpv6rpl_dao_rpl_target.type                      = 0x05; 
   //icmpv6rpl_dao_rpl_target.optionLength              = 0x00; 
   //icmpv6rpl_dao_rpl_target.flags                     = 0x00;
   //icmpv6rpl_dao_rpl_target.prefixLength              = 0x08;
   
   icmpv6rpl_dao_transit_info.type                    = 0x06;
   icmpv6rpl_dao_transit_info.E_flags                 = 0x00 | E_DAO_Transit_Info;
   icmpv6rpl_dao_transit_info.PathControl             = 0x00 | PC1_A_DAO_Transit_Info | PC1_B_DAO_Transit_Info | PC2_A_DAO_Transit_Info | PC2_B_DAO_Transit_Info | PC3_A_DAO_Transit_Info | PC3_B_DAO_Transit_Info | PC4_A_DAO_Transit_Info | PC4_B_DAO_Transit_Info;  
   icmpv6rpl_dao_transit_info.PathSequence            = 0x00;
   icmpv6rpl_dao_transit_info.PathLifetime            = 0xAA;
   icmpv6rpl_dao_transit_info.optionLength            = 0x00;
   
   icmpv6rpl_dio_options.type                         = 0x03;
   icmpv6rpl_dio_options.optionLength                 = 0x08;
   icmpv6rpl_dio_options.prefixLength                 = 0x06;
   icmpv6rpl_dio_options.Resvd_Prf_Resvd              = 0x00 | Prf_A_dio_options | Prf_B_dio_options;
   icmpv6rpl_dio_options.routeLifeTime                = 0x00000011;
   
   icmpv6rpl_vars.all_routers_multicast.type = ADDR_128B;
   icmpv6rpl_vars.all_routers_multicast.addr_128b[0]  = 0xff;
   icmpv6rpl_vars.all_routers_multicast.addr_128b[1]  = 0x02;
   icmpv6rpl_vars.all_routers_multicast.addr_128b[2]  = 0x00;
   icmpv6rpl_vars.all_routers_multicast.addr_128b[3]  = 0x00;
   icmpv6rpl_vars.all_routers_multicast.addr_128b[4]  = 0x00;
   icmpv6rpl_vars.all_routers_multicast.addr_128b[5]  = 0x00;
   icmpv6rpl_vars.all_routers_multicast.addr_128b[6]  = 0x00;
   icmpv6rpl_vars.all_routers_multicast.addr_128b[7]  = 0x00;
   icmpv6rpl_vars.all_routers_multicast.addr_128b[8]  = 0x00;
   icmpv6rpl_vars.all_routers_multicast.addr_128b[9]  = 0x00;
   icmpv6rpl_vars.all_routers_multicast.addr_128b[10] = 0x00;
   icmpv6rpl_vars.all_routers_multicast.addr_128b[11] = 0x00;
   icmpv6rpl_vars.all_routers_multicast.addr_128b[12] = 0x00;
   icmpv6rpl_vars.all_routers_multicast.addr_128b[13] = 0x00;
   icmpv6rpl_vars.all_routers_multicast.addr_128b[14] = 0x00;
   icmpv6rpl_vars.all_routers_multicast.addr_128b[15] = 0x02;
   
   icmpv6rpl_vars.periodDIO  = 1700+(openrandom_get16b()&0xff);       // pseudo-random
   icmpv6rpl_vars.timerId    = opentimers_start(icmpv6rpl_vars.periodDIO,
                                                TIMER_PERIODIC,TIME_MS,
                                                icmpv6rpl_timer_cb);
   //====== RPL DAO TIMER =====//
   icmpv6rpl_vars.periodDAO  = 10000+(openrandom_get16b()&0xff);       // pseudo-random (2000 can be changed base on the network)
   icmpv6rpl_vars.timerId    = opentimers_start(icmpv6rpl_vars.periodDAO,
                                                TIMER_PERIODIC,TIME_MS,
                                                icmpv6rpl_timer_DAO_cb);
}

void icmpv6rpl_trigger() {
   uint8_t number_bytes_from_input_buffer;
   uint8_t input_buffer[16];
   
   //get command from OpenSerial (16B IPv6 destination address)
   number_bytes_from_input_buffer = openserial_getInputBuffer(&(input_buffer[0]),sizeof(input_buffer));
   if (number_bytes_from_input_buffer!=sizeof(input_buffer)) {
      openserial_printError(COMPONENT_ICMPv6ECHO,ERR_INPUTBUFFER_LENGTH,
                            (errorparameter_t)number_bytes_from_input_buffer,
                            (errorparameter_t)0);
      return;
   };
   // Before sending check if the rank is not the default one if not then send
   if(neighbors_getMyDAGrank() != DEFAULTDAGRANK) {
      //send
      sendDIO();
   }
}

void icmpv6rpl_sendDone(OpenQueueEntry_t* msg, error_t error) {
   msg->owner = COMPONENT_ICMPv6RPL;
   if (msg->creator!=COMPONENT_ICMPv6RPL) {//that was a packet I had not created
      openserial_printError(COMPONENT_ICMPv6RPL,ERR_UNEXPECTED_SENDDONE,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
   }
   openqueue_freePacketBuffer(msg);
   icmpv6rpl_vars.busySending = FALSE;
}

void icmpv6rpl_receive(OpenQueueEntry_t* msg) {
   open_addr_t* temp_prefix;
   uint8_t      j;
   uint8_t      psize;
   uint8_t      codeValue;
   
   codeValue=(((ICMPv6_ht*)(msg->payload))->code);
   
   msg->owner = COMPONENT_ICMPv6RPL;
   
   // toss ICMPv6 header
   packetfunctions_tossHeader(msg,sizeof(ICMPv6_ht));
   
   if (codeValue == IANA_ICMPv6_RPL_DIO) {
      if (idmanager_getIsBridge()==FALSE) { // check that I'm not a root
         //update neighbor table
         neighbors_indicateRxDIO(msg);//pass dio specific object
         // now check the DODAGID and copy it if yet it has not been updated.
         // if(icmpv6rpl_vars.DODAGIDFlagSet==0)
         // {
         icmpv6rpl_vars.DODAGIDFlagSet=1;
         // copy the DODAGID for DIO and DAO as well
         // for DIO
         memcpy(&(icmpv6rpl_dio.DODAGID[0]),
                &(((icmpv6rpl_dio_t*)(msg->payload))->DODAGID[0]),
                sizeof(icmpv6rpl_dio.DODAGID));
         //for DAO
         memcpy(&(icmpv6rpl_dao.DODAGID[0]),
                &(((icmpv6rpl_dio_t*)(msg->payload))->DODAGID[0]),
                sizeof(icmpv6rpl_dao.DODAGID));
         // now to set the prefix
         // idmanager_setMyID(&(((icmpv6rpl_dio_t*)(msg->payload))->DODAGID[0])); 
         temp_prefix=idmanager_getMyID(ADDR_PREFIX);
         
         psize=sizeof(temp_prefix->prefix);
         for(j=0;j<psize;j++) {
            //dodagid is big endian
            temp_prefix->prefix[j]=(((icmpv6rpl_dio_t*)(msg->payload))->DODAGID[j]);   
         }
         temp_prefix->type=ADDR_PREFIX;
         idmanager_setMyID(temp_prefix);
         
         /*
         // check if the DIO option is included.
         if(((icmpv6rpl_dio_t*)(msg->payload))->options==0x03) {
               packetfunctions_tossHeader(msg,sizeof(icmpv6rpl_dio_t));
               temp_prefix=&(((icmpv6rpl_dio_options_t*)(msg->payload))->prefix);
               temp_prefix->type=ADDR_PREFIX;
               idmanager_setMyID(temp_prefix);
         }
         */
      }
   } else if(codeValue== IANA_ICMPv6_RPL_DAO) {
      // IT shouldn't get DAO because it will be handled in the lower layer.
      openserial_printCritical(COMPONENT_ICMPv6RPL,ERR_UNEXPECTED_DAO,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
   }
   
   //free packet
   openqueue_freePacketBuffer(msg);
}

//======= timer

void timers_rpl_fired() {
   icmpv6rpl_vars.delayDIO = (icmpv6rpl_vars.delayDIO+1)%5; //send on average every 10s
   if (icmpv6rpl_vars.delayDIO==0) {
      sendDIO();
      //set a new random periodDIO
      icmpv6rpl_vars.periodDIO = 1700+(openrandom_get16b()&0xff);       // pseudo-random
      opentimers_setPeriod(icmpv6rpl_vars.timerId,
                           TIME_MS,
                           icmpv6rpl_vars.periodDIO);
   }
}

void timers_rpl_DAO_fired() {
   icmpv6rpl_vars.delayDAO = (icmpv6rpl_vars.delayDAO+1)%5; //send on average every 10s
   if (icmpv6rpl_vars.delayDAO==0) {
      sendDAO();
      //set a new random periodDIO
      icmpv6rpl_vars.delayDAO = 2000+(openrandom_get16b()&0xff);       // pseudo-random
      opentimers_setPeriod(icmpv6rpl_vars.timerId,
                           TIME_MS,
                           icmpv6rpl_vars.periodDAO);
   }
}

//=========================== private =========================================

void sendDIO() {
   
   open_addr_t*         temp_prefixID;
   OpenQueueEntry_t*    msg;
   
   // do not send DIO if I'm in in bridge mode
   if (idmanager_getIsBridge()==TRUE) {
      return;
   }
   
   // do not send DIO if I have the default DAG rank
   if (neighbors_getMyDAGrank()==DEFAULTDAGRANK) {
      return;
   }
   
   // do not send DIO if I'm already busy sending
   if (icmpv6rpl_vars.busySending==TRUE) {
      return;
   }
   
   // if you get here, all good to send a DIO
   
   // I'm now busy sending
   icmpv6rpl_vars.busySending = TRUE;
   
   // reserve a free packet buffer for DIO
   msg = openqueue_getFreePacketBuffer(COMPONENT_ICMPv6RPL);
   if (msg==NULL) {
      openserial_printError(COMPONENT_ICMPv6RPL,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      icmpv6rpl_vars.busySending = FALSE;
      return;
   }
   
   // admin
   msg->creator                               = COMPONENT_ICMPv6RPL;
   msg->owner                                 = COMPONENT_ICMPv6RPL;
   // l4
   msg->l4_protocol                           = IANA_ICMPv6;
   msg->l4_sourcePortORicmpv6Type             = IANA_ICMPv6_RPL;
   // l3
   memcpy(&(msg->l3_destinationAdd),&icmpv6rpl_vars.all_routers_multicast,sizeof(open_addr_t));
   
   //===== DIO option
    //pooipoi xv-- removing prefix checking as we want to form the topology even the network is not connected to the IPv6 Network.
  
      temp_prefixID = idmanager_getMyID(ADDR_PREFIX);
      memcpy(&(icmpv6rpl_dio_options.prefix),temp_prefixID,sizeof(open_addr_t));
      
      // lifetime
      packetfunctions_htons(0x00000011,(uint8_t*)&(icmpv6rpl_dio_options.routeLifeTime)); 
      
      packetfunctions_reserveHeaderSize(msg,sizeof(icmpv6rpl_dio_options_t));
      memcpy(((icmpv6rpl_dio_options_t*)(msg->payload)),&(icmpv6rpl_dio_options),sizeof(icmpv6rpl_dio_options));
      
      // change dio option field to distinguish between the DIO with/without option
      icmpv6rpl_dio.options      = 0x03;

   
   //===== DIO
   
   packetfunctions_reserveHeaderSize(msg,sizeof(icmpv6rpl_dio_t));
   icmpv6rpl_dio.rank=neighbors_getMyDAGrank();
   memcpy(((icmpv6rpl_dio_t*)(msg->payload)),&(icmpv6rpl_dio),sizeof(icmpv6rpl_dio));
   
   //=====================================================================//
   //ICMPv6 header
   
   packetfunctions_reserveHeaderSize(msg,sizeof(ICMPv6_ht));
   ((ICMPv6_ht*)(msg->payload))->type         = msg->l4_sourcePortORicmpv6Type;
   ((ICMPv6_ht*)(msg->payload))->code         = IANA_ICMPv6_RPL_DIO;
   // Below Identifier might need to be replaced by the identifier used by icmpv6rpl
   // packetfunctions_htons(0x1234,(uint8_t*)&((ICMPv6_ht*)(msg->payload))->identifier);
   // Below sequence_number might need to be removed
   // packetfunctions_htons(icmpv6rpl_vars.seq++ ,(uint8_t*)&((ICMPv6_ht*)(msg->payload))->sequence_number); 
   packetfunctions_calculateChecksum(msg,(uint8_t*)&(((ICMPv6_ht*)(msg->payload))->checksum));//call last
   //send
   if (icmpv6_send(msg)!=E_SUCCESS) {
      icmpv6rpl_vars.busySending = FALSE;
      openqueue_freePacketBuffer(msg);
   } else {
      icmpv6rpl_vars.busySending = FALSE; 
   }
}

void sendDAO() {
   uint8_t           i;
   uint8_t           j;
   OpenQueueEntry_t* msg;
   
   // dont' send a DAO if you're in bridge mode
   if (idmanager_getIsBridge()==TRUE) {
      return;
   }
   
   // dont' send a DAO if you did not acquire a DAGrank
   if (neighbors_getMyDAGrank()==DEFAULTDAGRANK) {
       return;
   }
   
   // dont' send a DAO if you're still busy sending the previous one
   if (icmpv6rpl_vars.busySending==TRUE) {
      return;
   }
   
   // if you get here, you will send a DAO
   
   //===== reserve packet
   msg = openqueue_getFreePacketBuffer(COMPONENT_ICMPv6RPL);
   if (msg==NULL) {
      openserial_printError(COMPONENT_ICMPv6RPL,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      return;
   }
   
   //===== meta information
   //admin
   msg->creator                               = COMPONENT_ICMPv6RPL;
   msg->owner                                 = COMPONENT_ICMPv6RPL;
   //l4
   msg->l4_protocol                           = IANA_ICMPv6;
   msg->l4_sourcePortORicmpv6Type             = IANA_ICMPv6_RPL;
   //l3 (send to DODAGID)
   (msg->l3_destinationAdd).type=ADDR_128B;
   for (i=0;i<sizeof(icmpv6rpl_dio.DODAGID);i++) {
      //big endian
      msg->l3_destinationAdd.addr_128b[i] =icmpv6rpl_dio.DODAGID[i];
   }
   
   //===== Transit option
   j=0;
   for (i=0;i<MAXNUMNEIGHBORS;i++) {
      if ((neighbors_isNeighborWithLowerDAGrank(i))== TRUE) {
         packetfunctions_reserveHeaderSize(msg,8);
         neighbors_writeAddrLowerDAGrank((msg->payload),ADDR_64B,i);    
         j++;
      }  
   }
   
   // It's only sent the transit if the node have at least a parents
   if (j>0) {
      icmpv6rpl_dao_transit_info.optionLength  =j;
      packetfunctions_reserveHeaderSize(msg,sizeof(icmpv6rpl_dao_transit_info_t));
      memcpy(((icmpv6rpl_dao_transit_info_t*)(msg->payload)),&(icmpv6rpl_dao_transit_info),sizeof(icmpv6rpl_dao_transit_info));
      
      // The path sequance has to be increased by one assuming each DAO sent is a new DAO frame
      icmpv6rpl_dao_transit_info.PathSequence++;  
      icmpv6rpl_dao.options      =0x06;    // indicate that in DAO the transit frame will be appended to the main DAO frame.
   }
   
   //===== Target option
   /*
   j=0;
   for (i=0;i<MAXNUMNEIGHBORS;i++) {
      if((neighbors_writeAddrHigherDAGrank(temp_prefix64btoWrite_parent,i))== TRUE) {
         packetfunctions_reserveHeaderSize(msg,sizeof(open_addr_t));
         memcpy(((open_addr_t*)(msg->payload)),temp_prefix64btoWrite_parent,sizeof(open_addr_t));
         j++;
      }        
   }
   
   icmpv6rpl_dao_rpl_target.optionLength  =j;
   packetfunctions_reserveHeaderSize(msg,sizeof(icmpv6rpl_dao_rpl_target_t));
   memcpy(((icmpv6rpl_dao_rpl_target_t*)(msg->payload)),&(icmpv6rpl_dao_rpl_target),sizeof(icmpv6rpl_dao_rpl_target));
   */
   
   //===== DAO
   packetfunctions_reserveHeaderSize(msg,sizeof(icmpv6rpl_dao_t));
   memcpy(((icmpv6rpl_dao_t*)(msg->payload)),&(icmpv6rpl_dao),sizeof(icmpv6rpl_dao));
   
   //===== ICMPv6
   packetfunctions_reserveHeaderSize(msg,sizeof(ICMPv6_ht));
   ((ICMPv6_ht*)(msg->payload))->type         = msg->l4_sourcePortORicmpv6Type;
   ((ICMPv6_ht*)(msg->payload))->code         = IANA_ICMPv6_RPL_DAO;
   packetfunctions_calculateChecksum(msg,(uint8_t*)&(((ICMPv6_ht*)(msg->payload))->checksum)); //call last
   
   //=====send
   if (icmpv6_send(msg)==E_SUCCESS) {
      icmpv6rpl_vars.busySending = TRUE;
   } else {
      openqueue_freePacketBuffer(msg);
   }
}

void icmpv6rpl_timer_cb() {
   scheduler_push_task(timers_rpl_fired,TASKPRIO_RPL);
}

void icmpv6rpl_timer_DAO_cb() {
   scheduler_push_task(timers_rpl_DAO_fired,TASKPRIO_RPL);
}

void icmpv6rpl_receiveDAO(OpenQueueEntry_t* msg){ 
   openserial_printCritical(COMPONENT_ICMPv6RPL,ERR_UNEXPECTED_DAO,
                            (errorparameter_t)1,
                            (errorparameter_t)0);
}