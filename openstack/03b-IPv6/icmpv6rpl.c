#include "opendefs.h"
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
#include "IEEE802154E.h"
#include "sixtop.h"



//#define _DEBUG_DAO_
//#define _DEBUG_DIO_

//=========================== variables =======================================

icmpv6rpl_vars_t             icmpv6rpl_vars;

//=========================== prototypes ======================================

// routing-related
void icmpv6rpl_updateMyDAGrankAndParentSelection(void);
// DIO-related
void icmpv6rpl_timer_DIO_cb(opentimer_id_t id);
void icmpv6rpl_timer_DIO_task(void);
void sendDIO(void);
// DAO-related
void icmpv6rpl_timer_DAO_cb(opentimer_id_t id);
void icmpv6rpl_timer_DAO_task(void);
void sendDAO(void);


//=========================== public ==========================================

/**
\brief Initialize this module.
*/
void icmpv6rpl_init() {
   uint8_t         dodagid[16];

   
   // retrieve my prefix and EUI64
   memcpy(&dodagid[0],idmanager_getMyID(ADDR_PREFIX)->prefix,8); // prefix
   memcpy(&dodagid[8],idmanager_getMyID(ADDR_64B)->addr_64b,8);  // eui64
   
   //===== reset local variables
   memset(&icmpv6rpl_vars,0,sizeof(icmpv6rpl_vars_t));
   
   //=== routing
   icmpv6rpl_vars.haveParent=FALSE;
   if (idmanager_getIsDAGroot()==TRUE) {
      icmpv6rpl_vars.myDAGrank=MINHOPRANKINCREASE;
   } else {
      icmpv6rpl_vars.myDAGrank=DEFAULTDAGRANK;
   }

   //=== admin
   
   icmpv6rpl_vars.lastDIO_tx                = NULL;
   icmpv6rpl_vars.lastDAO_tx                = NULL;
   icmpv6rpl_vars.fDodagidWritten           = 0;
   
   //=== DIO
   
   icmpv6rpl_vars.dio.rplinstanceId         = 0x00;        ///< TODO: put correct value
   icmpv6rpl_vars.dio.verNumb               = 0x00;        ///< TODO: put correct value
   // rank: to be populated upon TX
   icmpv6rpl_vars.dio.rplOptions            = MOP_DIO_A | \
                                              MOP_DIO_B | \
                                              MOP_DIO_C | \
                                              PRF_DIO_A | \
                                              PRF_DIO_B | \
                                              PRF_DIO_C | \
                                              G_DIO ;
   icmpv6rpl_vars.dio.DTSN                  = 0x33;        ///< TODO: put correct value
   icmpv6rpl_vars.dio.flags                 = 0x00;
   icmpv6rpl_vars.dio.reserved              = 0x00;
   memcpy(
      &(icmpv6rpl_vars.dio.DODAGID[0]),
      dodagid,
      sizeof(icmpv6rpl_vars.dio.DODAGID)
   ); // can be replaced later
   
   icmpv6rpl_vars.dioDestination.type = ADDR_128B;
   memcpy(&icmpv6rpl_vars.dioDestination.addr_128b[0],all_routers_multicast,sizeof(all_routers_multicast));
   
   icmpv6rpl_vars.dioPeriod                 = (TIMER_DIO_TIMEOUT+(openrandom_get16b()&0xff));
   icmpv6rpl_vars.timerIdDIO                = opentimers_start(
                                                icmpv6rpl_vars.dioPeriod,
                                                TIMER_PERIODIC,
                                                TIME_MS,
                                                icmpv6rpl_timer_DIO_cb
                                             );
   
   //=== DAO
   
   icmpv6rpl_vars.dao.rplinstanceId         = 0x00;        ///< TODO: put correct value
   icmpv6rpl_vars.dao.K_D_flags             = FLAG_DAO_A   | \
                                              FLAG_DAO_B   | \
                                              FLAG_DAO_C   | \
                                              FLAG_DAO_D   | \
                                              FLAG_DAO_E   | \
                                              PRF_DIO_C    | \
                                              FLAG_DAO_F   | \
                                              D_DAO        |
                                              K_DAO;
   icmpv6rpl_vars.dao.reserved              = 0x00;
   icmpv6rpl_vars.dao.DAOSequence           = 0x00;
   memcpy(
      &(icmpv6rpl_vars.dao.DODAGID[0]),
      dodagid,
      sizeof(icmpv6rpl_vars.dao.DODAGID)
   );  // can be replaced later
   
   icmpv6rpl_vars.dao_transit.type          = OPTION_TRANSIT_INFORMATION_TYPE;
   // optionLength: to be populated upon TX
   icmpv6rpl_vars.dao_transit.E_flags       = E_DAO_Transit_Info;
   icmpv6rpl_vars.dao_transit.PathControl   = PC1_A_DAO_Transit_Info | \
                                              PC1_B_DAO_Transit_Info | \
                                              PC2_A_DAO_Transit_Info | \
                                              PC2_B_DAO_Transit_Info | \
                                              PC3_A_DAO_Transit_Info | \
                                              PC3_B_DAO_Transit_Info | \
                                              PC4_A_DAO_Transit_Info | \
                                              PC4_B_DAO_Transit_Info;  
   icmpv6rpl_vars.dao_transit.PathSequence  = 0x00; // to be incremented at each TX
   icmpv6rpl_vars.dao_transit.PathLifetime  = 0xAA;
   //target information
   icmpv6rpl_vars.dao_target.type  = OPTION_TARGET_INFORMATION_TYPE;
   icmpv6rpl_vars.dao_target.optionLength  = 0;
   icmpv6rpl_vars.dao_target.flags  = 0;
   icmpv6rpl_vars.dao_target.prefixLength = 0;
   
   icmpv6rpl_vars.daoPeriod                 = (TIMER_DAO_TIMEOUT+(openrandom_get16b()&0xff));
   icmpv6rpl_vars.timerIdDAO                = opentimers_start(
                                                icmpv6rpl_vars.daoPeriod,
                                                TIMER_PERIODIC,
                                                TIME_MS,
                                                icmpv6rpl_timer_DAO_cb
                                             );
   
}

void  icmpv6rpl_writeDODAGid(uint8_t* dodagid) {
   
   // write DODAGID to DIO/DAO
   memcpy(
      &(icmpv6rpl_vars.dio.DODAGID[0]),
      dodagid,
      sizeof(icmpv6rpl_vars.dio.DODAGID)
   );
   memcpy(
      &(icmpv6rpl_vars.dao.DODAGID[0]),
      dodagid,
      sizeof(icmpv6rpl_vars.dao.DODAGID)
   );
   
   // remember I got a DODAGID
   icmpv6rpl_vars.fDodagidWritten = 1;
}

uint8_t icmpv6rpl_getRPLIntanceID(){
   return icmpv6rpl_vars.dao.rplinstanceId;
}
                                                

void    icmpv6rpl_getRPLDODAGid(uint8_t* address_128b){
    memcpy(address_128b,icmpv6rpl_vars.dao.DODAGID,16);
}



/**
\brief Called when DIO/DAO was sent.

\param[in] msg   Pointer to the message just sent.
\param[in] error Outcome of the sending.
*/
void icmpv6rpl_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
#if defined(_DEBUG_DIO_) || defined(_DEBUG_DAO_)
   char str[150];
#endif

   
   // take ownership over that packet
   msg->owner = COMPONENT_ICMPv6RPL;
   
   // make sure I created it
   if (msg->creator!=COMPONENT_ICMPv6RPL) {
      openserial_printError(COMPONENT_ICMPv6RPL,ERR_UNEXPECTED_SENDDONE,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
   }


   // The DIO / DAO was pushed to the MAC layer
   if (msg == icmpv6rpl_vars.lastDIO_tx){
#ifdef _DEBUG_DIO_
      sprintf(str, "RPL - DIO transmitted");
      openserial_printf(COMPONENT_ICMPv6RPL, str, strlen(str));
#endif

      //for stats
      openserial_statDIOtx();

      icmpv6rpl_vars.lastDIO_tx = NULL;
   }
   if (msg == icmpv6rpl_vars.lastDAO_tx){

#ifdef _DEBUG_DAO_
      sprintf(str, "RPL - DAO transmitted via ");
      openserial_ncat_uint8_t_hex(str, (uint8_t)msg->l2_nextORpreviousHop.addr_64b[6], 150);
      openserial_ncat_uint8_t_hex(str, (uint8_t)msg->l2_nextORpreviousHop.addr_64b[7], 150);
      strncat(str, " to ", 150);
      openserial_ncat_uint8_t_hex(str, (uint8_t)msg->l3_destinationAdd.addr_128b[14], 150);
      openserial_ncat_uint8_t_hex(str, (uint8_t)msg->l3_destinationAdd.addr_128b[15], 150);
      openserial_printf(COMPONENT_ICMPv6RPL, str, strlen(str));
#endif

      //for stats
      openserial_statDAOtx(msg->l2_nextORpreviousHop.addr_64b);

      icmpv6rpl_vars.lastDAO_tx = NULL;
   }

   // free packet
   openqueue_freePacketBuffer(msg);

}

/**
\brief Called when RPL message received.

\param[in] msg   Pointer to the received message.
*/
void icmpv6rpl_receive(OpenQueueEntry_t* msg) {
   uint8_t      icmpv6code;
   open_addr_t  myPrefix;
   
   // take ownership
   msg->owner      = COMPONENT_ICMPv6RPL;
   
   // retrieve ICMPv6 code
   icmpv6code      = (((ICMPv6_ht*)(msg->payload))->code);
   
   // toss ICMPv6 header
   packetfunctions_tossHeader(msg,sizeof(ICMPv6_ht));
   
   // handle message
   switch (icmpv6code) {
      case IANA_ICMPv6_RPL_DIS:
         icmpv6rpl_timer_DIO_task();
         break;
      case IANA_ICMPv6_RPL_DIO:
         // update neighbor table
         //icmpv6rpl_indicateRxDIO(msg);

         if (idmanager_getIsDAGroot()==TRUE) {
            // stop here if I'm in the DAG root
            break; // break, don't return
         }
                  
         memcpy(
            &(icmpv6rpl_vars.dio),
            (icmpv6rpl_dio_ht*)(msg->payload),
            sizeof(icmpv6rpl_dio_ht)
         );
         
         // write DODAGID in DIO and DAO
         icmpv6rpl_writeDODAGid(&(((icmpv6rpl_dio_ht*)(msg->payload))->DODAGID[0]));
         
         // update my prefix // looks like we adopt the prefix from any DIO without a question about this node being our parent??
         myPrefix.type = ADDR_PREFIX;
         memcpy(
            myPrefix.prefix,
            &((icmpv6rpl_dio_ht*)(msg->payload))->DODAGID[0],
            sizeof(myPrefix.prefix)
         );
         idmanager_setMyID(&myPrefix);
         
         // update routing info for that neighbor
         icmpv6rpl_indicateRxDIO(msg);
         
         break;
      
      case IANA_ICMPv6_RPL_DAO:

#ifdef _DEBUG_DAO_
         ;
         char str[150];
         sprintf(str, "DAO received from the source ");
         openserial_ncat_uint8_t_hex(str, (uint8_t)msg->l3_sourceAdd.addr_128b[14], 150);
         strncat(str, "-", 150);
         openserial_ncat_uint8_t_hex(str, (uint8_t)msg->l3_sourceAdd.addr_128b[15], 150);
         openserial_printf(COMPONENT_ICMPv6RPL, str, strlen(str));
#endif

         // this should never happen
         if (!idmanager_getIsDAGroot())
            openserial_printError(COMPONENT_ICMPv6RPL,ERR_UNEXPECTED_DAO,
               (errorparameter_t)0,
               (errorparameter_t)0);


         break;
      default:
         // this should never happen
         openserial_printError(COMPONENT_ICMPv6RPL,ERR_MSG_UNKNOWN_TYPE,
                               (errorparameter_t)icmpv6code,
                               (errorparameter_t)0);
         break;
      
   }
   
   // free message
   openqueue_freePacketBuffer(msg);
}

/**
\brief Retrieve this mote's parent index in neighbor table.

\returns TRUE and index of parent if have one, FALSE if no parent
*/
bool icmpv6rpl_getPreferredParentIndex(uint8_t* indexptr) {
   *indexptr = icmpv6rpl_vars.ParentIndex;
   return icmpv6rpl_vars.haveParent;
}

/**
\brief Retrieve my preferred parent's EUI64 address.
\param[out] addressToWrite Where to copy the preferred parent's address to.
*/
bool icmpv6rpl_getPreferredParentEui64(open_addr_t* addressToWrite) {
   if (icmpv6rpl_vars.haveParent){
       return neighbors_getNeighborEui64(addressToWrite,ADDR_64B,icmpv6rpl_vars.ParentIndex);
   }
   else return FALSE;
}

/**
\brief Indicate whether some neighbor is the routing parent.

\param[in] address The EUI64 address of the neighbor.

\returns TRUE if that neighbor is preferred parent, FALSE otherwise.
*/
bool icmpv6rpl_isPreferredParent(open_addr_t* address) {
   open_addr_t  temp;
   // do we currently have a parent?
   if (icmpv6rpl_vars.haveParent==FALSE) {
      return FALSE;
   }
   
   //compare parent address to the one presented.
   switch (address->type) {
      case ADDR_64B:
         neighbors_getNeighborEui64(&temp,ADDR_64B,icmpv6rpl_vars.ParentIndex);
         return packetfunctions_sameAddress_debug(address,&temp,COMPONENT_ICMPv6RPL);
      default:
         openserial_printCritical(COMPONENT_ICMPv6RPL,ERR_WRONG_ADDR_TYPE,
                               (errorparameter_t)address->type,
                               (errorparameter_t)3);
         return FALSE;
   }
}

/**
\brief Retrieve this mote's current DAG rank.

\returns This mote's current DAG rank.
*/
dagrank_t icmpv6rpl_getMyDAGrank() {
   return icmpv6rpl_vars.myDAGrank;
}

/**
\brief Direct intervention to set the value of DAG rank in the data structure

Meant for direct control from command on serial port or from test application,
bypassing the routing protocol!
*/
void icmpv6rpl_setMyDAGrank(dagrank_t rank){
    icmpv6rpl_vars.myDAGrank = rank;
}

/**
\brief Routing algorithm
*/
void icmpv6rpl_updateMyDAGrankAndParentSelection() {
   uint8_t   i;
   uint16_t  previousDAGrank;
   uint16_t  prevRankIncrease;
   uint8_t   prevParentIndex;
   bool      prevHadParent;
   bool      foundBetterParent;
   // temporaries
   uint16_t  rankIncrease;
   dagrank_t neighborRank;
   uint32_t  tentativeDAGrank;
   
   // if I'm a DAGroot, my DAGrank is always MINHOPRANKINCREASE
   if ((idmanager_getIsDAGroot())==TRUE) {
       // the dagrank is not set through setting command, set rank to MINHOPRANKINCREASE here 
       if (icmpv6rpl_vars.myDAGrank!=MINHOPRANKINCREASE) { // test for change so as not to report unchanged value when root
           icmpv6rpl_vars.myDAGrank=MINHOPRANKINCREASE;
           return;
       }
   }

   // prep for loop, remember state before neighbor table scanning
   previousDAGrank      = icmpv6rpl_vars.myDAGrank;
   prevParentIndex      = icmpv6rpl_vars.ParentIndex;
   prevHadParent        = icmpv6rpl_vars.haveParent;
   prevRankIncrease     = icmpv6rpl_vars.rankIncrease;
   foundBetterParent    = FALSE;
   icmpv6rpl_vars.haveParent = FALSE;
   icmpv6rpl_vars.myDAGrank = DEFAULTDAGRANK;
   
   //debug
   char str[150];
 /*  sprintf(str, "My dagrank=");
   openserial_ncat_uint32_t(str, (uint32_t)icmpv6rpl_vars.myDAGrank, 150);
   openserial_printf(COMPONENT_ICMPv6RPL, str, strlen(str));
*/
   // loop through neighbor table, update myDAGrank
   for (i=0;i<MAXNUMNEIGHBORS;i++) {

      open_addr_t      NeighborAddress;
      neighbors_getNeighborEui64(&NeighborAddress, ADDR_64B, i); // this neighbor entry is in use
     /*
      sprintf(str, "RANK neigh ");
      openserial_ncat_uint8_t_hex(str, NeighborAddress.addr_64b[6], 150);
      openserial_ncat_uint8_t_hex(str, NeighborAddress.addr_64b[7], 150);
      strncat(str, ", stable= ", 150);
      openserial_ncat_uint32_t(str, (uint32_t)neighbors_isStableNeighborByIndex(i), 150);
*/

      if (neighbors_isStableNeighborByIndex(i)) { // in use and link is stable
         // get link cost to this neighbor
         rankIncrease=neighbors_getLinkMetric(i);
         // if this link cost is too high, pass on this neighbor
         // TODO
         // get this neighbor's advertized rank
         neighborRank=neighbors_getNeighborRank(i);

         /*
         strncat(str, ", rank=", 150);
         openserial_ncat_uint32_t(str, (uint32_t)neighborRank, 150);
         strncat(str, ", metric= ", 150);
         openserial_ncat_uint32_t(str, (uint32_t)neighbors_getLinkMetric(i), 150);
*/

         // if this neighbor has unknown/infinite rank, pass on it
         if (neighborRank==DEFAULTDAGRANK) continue;
         // compute tentative cost of full path to root through this neighbor
         tentativeDAGrank = (uint32_t)neighborRank+rankIncrease;
         if (tentativeDAGrank > 65535) {tentativeDAGrank = 65535;}


         // if not low enough to justify switch, pass (i.e. hysterisis)
         //if ((previousDAGrank<tentativeDAGrank) ||
         // next line is wrong, difference can be negative
         //    (tentativeDAGrank-previousDAGrank < 2*MINHOPRANKINCREASE)) continue;
         // remember that we have at least one valid candidate parent
         foundBetterParent=TRUE;
         // select best candidate so far
         if (icmpv6rpl_vars.myDAGrank>tentativeDAGrank) {
            strncat(str, ", better", 150);

            icmpv6rpl_vars.myDAGrank    = (uint16_t)tentativeDAGrank;
            icmpv6rpl_vars.ParentIndex  = i;
            icmpv6rpl_vars.rankIncrease = rankIncrease;
         }
         //openserial_printf(COMPONENT_ICMPv6RPL, str, strlen(str));
    }
   } 
   
   if (foundBetterParent) {
      icmpv6rpl_vars.haveParent=TRUE;
      if (!prevHadParent) {
         // only report on link creation
      } else {
         if (icmpv6rpl_vars.ParentIndex==prevParentIndex) {

            // report on the rank change if any, not on the deletion/creation of parent
               if (icmpv6rpl_vars.myDAGrank!=previousDAGrank) {
               } else ;// same parent, same rank, nothing to report about
         } else {
            //update the next hops for my enqueued packets
            open_addr_t      prevParent, newParent;
            neighbors_getNeighborEui64(&prevParent, ADDR_64B, prevParentIndex); // this neighbor entry is in use
            neighbors_getNeighborEui64(&newParent, ADDR_64B, icmpv6rpl_vars.ParentIndex); // this neighbor entry is in use
            openqueue_replace_nexthop(&prevParent, &newParent);

            char str[150];
            sprintf(str, "RPL (parent update) replaces in the queue the nexthops ");
            openserial_ncat_uint8_t_hex(str, prevParent.addr_64b[6], 150);
            openserial_ncat_uint8_t_hex(str, prevParent.addr_64b[7], 150);
            strncat(str, " by ", 150);
            openserial_ncat_uint8_t_hex(str, newParent.addr_64b[6], 150);
            openserial_ncat_uint8_t_hex(str, newParent.addr_64b[7], 150);
            openserial_printf(COMPONENT_ICMPv6RPL, str, strlen(str));

            //back to the idle state!
            sixtop_setIdle();

            // report on deletion of parent
            // report on creation of new parent
         }
      }
   } else {
      // restore routing table as we found it on entry
      icmpv6rpl_vars.myDAGrank   = previousDAGrank;
      icmpv6rpl_vars.ParentIndex = prevParentIndex;
      icmpv6rpl_vars.haveParent  = prevHadParent;
      icmpv6rpl_vars.rankIncrease= prevRankIncrease;
      // no change to report on
   }
}

/**
\brief Indicate I just received a RPL DIO from a neighbor.

This function should be called for each received a DIO is received so neighbor
routing information in the neighbor table can be updated.

The fields which are updated are:
- DAGrank

\param[in] msg The received message with msg->payload pointing to the DIO
   header.
*/
void icmpv6rpl_indicateRxDIO(OpenQueueEntry_t* msg) {
   uint8_t          i;
   uint8_t          temp_8b;
   dagrank_t        neighborRank;
   open_addr_t      NeighborAddress;
  
   // take ownership over the packet
   msg->owner = COMPONENT_NEIGHBORS;
   
   // save pointer to incoming DIO header in global structure for simplifying debug.
   icmpv6rpl_vars.incomingDio = (icmpv6rpl_dio_ht*)(msg->payload);
   // quick fix: rank is two bytes in network order: need to swap bytes
   temp_8b            = *(msg->payload+2);
   icmpv6rpl_vars.incomingDio->rank = (temp_8b << 8) + *(msg->payload+3);

   // update rank of that neighbor in table
   for (i=0;i<MAXNUMNEIGHBORS;i++) {
      if (neighbors_getNeighborEui64(&NeighborAddress, ADDR_64B, i)) { // this neighbor entry is in use
         if (packetfunctions_sameAddress_debug(&(msg->l2_nextORpreviousHop),&NeighborAddress, COMPONENT_ICMPv6RPL)) { // matching address
            neighborRank=neighbors_getNeighborRank(i);

            if (
              (icmpv6rpl_vars.incomingDio->rank > neighborRank) &&
              (icmpv6rpl_vars.incomingDio->rank - neighborRank) > (DEFAULTLINKCOST*2*MINHOPRANKINCREASE)
            ) {
               // the new DAGrank looks suspiciously high, only increment a bit
               neighbors_setNeighborRank(i,neighborRank + (DEFAULTLINKCOST*2*MINHOPRANKINCREASE));
               openserial_printError(COMPONENT_NEIGHBORS,ERR_LARGE_DAGRANK,
                               (errorparameter_t)icmpv6rpl_vars.incomingDio->rank,
                               (errorparameter_t)neighborRank);
            } else {
               neighbors_setNeighborRank(i,icmpv6rpl_vars.incomingDio->rank);
            }
            // since changes were made to neighbors DAG rank, run the routing algorithm again
            icmpv6rpl_updateMyDAGrankAndParentSelection(); 
            break; // there should be only one matching entry, no need to loop further
         }
      }
   } 
}

void icmpv6rpl_killPreferredParent() {
    icmpv6rpl_vars.haveParent=FALSE;
    if (idmanager_getIsDAGroot()==TRUE) {
       icmpv6rpl_vars.myDAGrank=MINHOPRANKINCREASE;
    } else {
       icmpv6rpl_vars.myDAGrank=DEFAULTDAGRANK;
    }
}

//=========================== private =========================================

//===== DIO-related

/**
\brief DIO timer callback function.

\note This function is executed in interrupt context, and should only push a 
   task.
*/
void icmpv6rpl_timer_DIO_cb(opentimer_id_t id) {
   scheduler_push_task(icmpv6rpl_timer_DIO_task,TASKPRIO_RPL);
}

/**
\brief Handler for DIO timer event.

\note This function is executed in task context, called by the scheduler.
*/
void icmpv6rpl_timer_DIO_task() {

   // send DIO
   sendDIO();

   // pick a new pseudo-random dioPeriod
   uint16_t   jitter = openrandom_get16b();
   uint16_t   bool = openrandom_get16b() & 0x0001;

   while(jitter > TIMER_DIO_TIMEOUT * TIMER_DIO_JITTER)
      jitter -= TIMER_DIO_TIMEOUT * TIMER_DIO_JITTER;
   if (bool > 0)
      icmpv6rpl_vars.dioPeriod = (TIMER_DIO_TIMEOUT - jitter);
   else
      icmpv6rpl_vars.dioPeriod = (TIMER_DIO_TIMEOUT + jitter);


   // arm the DIO timer with this new value
   opentimers_setPeriod(
         icmpv6rpl_vars.timerIdDIO,
         TIME_MS,
         icmpv6rpl_vars.dioPeriod
   );
}

/**
\brief Prepare and a send a RPL DIO.
*/
void sendDIO() {
   OpenQueueEntry_t*    msg;

#ifdef _DEBUG_DIO_
   char str[150];
#endif
   
   // stop if I'm not sync'ed
   if (ieee154e_isSynch()==FALSE) {
      
#ifdef _DEBUG_DIO_
     sprintf(str, "RPL - DIO failed (!synchro) ");
      openserial_printf(COMPONENT_ICMPv6RPL, str, strlen(str));
#endif

      if (icmpv6rpl_vars.lastDIO_tx != NULL)
         openqueue_removeEntry(icmpv6rpl_vars.lastDIO_tx);
      icmpv6rpl_vars.lastDIO_tx = NULL;
      
      // stop here
      return;
   }

   // do not send DIO if I have the default DAG rank
   if (icmpv6rpl_getMyDAGrank()==DEFAULTDAGRANK) {
#ifdef _DEBUG_DIO_
      sprintf(str, "RPL - DIO failed (no rank)");
      openserial_printf(COMPONENT_ICMPv6RPL, str, strlen(str));
#endif

      return;
   }
   
   // replace the previous if one exists in the queue
   if (icmpv6rpl_vars.lastDIO_tx != NULL){
#ifdef _DEBUG_DIO_
      sprintf(str, "RPL - DIO: previous DIO replaced");
      openserial_printf(COMPONENT_ICMPv6RPL, str, strlen(str));
#endif
      openqueue_removeEntry(icmpv6rpl_vars.lastDIO_tx);
      icmpv6rpl_vars.lastDIO_tx = NULL;

      return;
   }
   

   // if you get here, all good to send a DIO
   
   // reserve a free packet buffer for DIO
   msg = openqueue_getFreePacketBuffer(COMPONENT_ICMPv6RPL);
   if (msg==NULL) {
      openserial_printError(COMPONENT_ICMPv6RPL,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      
      return;
   }
   
   // take ownership
   msg->creator                             = COMPONENT_ICMPv6RPL;
   msg->owner                               = COMPONENT_ICMPv6RPL;
   
   // set transport information
   msg->l4_protocol                         = IANA_ICMPv6;
   msg->l4_sourcePortORicmpv6Type           = IANA_ICMPv6_RPL;
   
   // set DIO destination
   memcpy(&(msg->l3_destinationAdd),&icmpv6rpl_vars.dioDestination,sizeof(open_addr_t));
   
   //===== DIO payload
   // note: DIO is already mostly populated
   icmpv6rpl_vars.dio.rank                  = icmpv6rpl_getMyDAGrank();
   packetfunctions_reserveHeaderSize(msg,sizeof(icmpv6rpl_dio_ht));
   memcpy(
      ((icmpv6rpl_dio_ht*)(msg->payload)),
      &(icmpv6rpl_vars.dio),
      sizeof(icmpv6rpl_dio_ht)
   );

   // reverse the rank bytes order in Big Endian
   *(msg->payload+2) = (icmpv6rpl_vars.dio.rank >> 8) & 0xFF;
   *(msg->payload+3) = icmpv6rpl_vars.dio.rank        & 0xFF;
   
   //===== ICMPv6 header
   packetfunctions_reserveHeaderSize(msg,sizeof(ICMPv6_ht));
   ((ICMPv6_ht*)(msg->payload))->type       = msg->l4_sourcePortORicmpv6Type;
   ((ICMPv6_ht*)(msg->payload))->code       = IANA_ICMPv6_RPL_DIO;
   packetfunctions_calculateChecksum(msg,(uint8_t*)&(((ICMPv6_ht*)(msg->payload))->checksum));//call last
   

   //send
    if (icmpv6_send(msg)!=E_SUCCESS) {
       openqueue_freePacketBuffer(msg);
 #ifdef _DEBUG_DIO_
       sprintf(str, "RPL - DIO: tx failed");
       openserial_printf(COMPONENT_ICMPv6RPL, str, strlen(str));
 #endif

    } else {
       icmpv6rpl_vars.lastDIO_tx = msg;

 #ifdef _DEBUG_DIO_
       sprintf(str, "RPL - DIO pushed in the queue");
       openserial_printf(COMPONENT_ICMPv6RPL, str, strlen(str));
 #endif

    }
}

//===== DAO-related

/**
\brief DAO timer callback function.

\note This function is executed in interrupt context, and should only push a
   task.
*/
void icmpv6rpl_timer_DAO_cb(opentimer_id_t id) {
   scheduler_push_task(icmpv6rpl_timer_DAO_task,TASKPRIO_RPL);
}

/**
\brief Handler for DAO timer event.

\note This function is executed in task context, called by the scheduler.
*/
void icmpv6rpl_timer_DAO_task() {

   // pick a new pseudo-random daoPeriod
   uint16_t   jitter = openrandom_get16b();
   uint16_t   bool = openrandom_get16b() & 0x0001;
   while(jitter > TIMER_DAO_TIMEOUT * TIMER_DAO_JITTER)
      jitter -= TIMER_DAO_TIMEOUT * TIMER_DAO_JITTER;
   if (bool > 0)
      icmpv6rpl_vars.daoPeriod = (TIMER_DAO_TIMEOUT - jitter);
   else
      icmpv6rpl_vars.daoPeriod = (TIMER_DAO_TIMEOUT + jitter);

   // send DAO
   sendDAO();

   // arm the DAO timer with this new value
   opentimers_setPeriod(
         icmpv6rpl_vars.timerIdDAO,
         TIME_MS,
         icmpv6rpl_vars.daoPeriod
   );
}

/**
\brief Prepare and a send a RPL DAO.
*/
void sendDAO() {
   OpenQueueEntry_t*    msg;                // pointer to DAO messages
   uint8_t              nbrIdx;             // running neighbor index
   uint8_t              numTransitParents,numTargetParents;  // the number of parents indicated in transit option
   open_addr_t         address;
   open_addr_t*        prefix;
   

#ifdef _DEBUG_DAO_
   char str[150];
#endif


   // dont' send a DAO if you're the DAG root
   if (idmanager_getIsDAGroot()==TRUE) {
      return;
   }

   if (ieee154e_isSynch()==FALSE) {
      // I'm not sync'ed 
#ifdef _DEBUG_DAO_
      sprintf(str, "RPL - DAO failed (not synchronized)");
      openserial_printf(COMPONENT_ICMPv6RPL, str, strlen(str));
#endif


      // delete the DAO already enqueued
      if (icmpv6rpl_vars.lastDAO_tx != NULL)
         openqueue_removeEntry(icmpv6rpl_vars.lastDAO_tx);
      icmpv6rpl_vars.lastDAO_tx = NULL;

      // stop here
      return;
   }

   //TODO: we will have a warning if we are currently transmitting this packet (154E layer)
   // dont' send a DAO if you're still busy sending the previous one
   if (icmpv6rpl_vars.lastDAO_tx != NULL) {

#ifdef _DEBUG_DAO_
      sprintf(str, "RPL - DAO: we replace the last one (in the queue)");
      openserial_printf(COMPONENT_ICMPv6RPL, str, strlen(str));
#endif

      openqueue_removeEntry(icmpv6rpl_vars.lastDAO_tx);
      icmpv6rpl_vars.lastDAO_tx = NULL;
   }
   
   // dont' send a DAO if you did not acquire a DAGrank
   if (icmpv6rpl_getMyDAGrank()==DEFAULTDAGRANK) {
#ifdef _DEBUG_DAO_
       sprintf(str, "RPL - DAO failed (no dag rank)");
       openserial_printf(COMPONENT_ICMPv6RPL, str, strlen(str));
#endif

       return;
   }
   

   
   // if you get here, you start construct DAO
   
   // reserve a free packet buffer for DAO
   msg = openqueue_getFreePacketBuffer(COMPONENT_ICMPv6RPL);
   if (msg==NULL) {
      openserial_printError(COMPONENT_ICMPv6RPL,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      return;
   }
   
   // take ownership
   msg->creator                             = COMPONENT_ICMPv6RPL;
   msg->owner                               = COMPONENT_ICMPv6RPL;
   
   // set transport information
   msg->l4_protocol                         = IANA_ICMPv6;
   msg->l4_sourcePortORicmpv6Type           = IANA_ICMPv6_RPL;

   
   // set track for DAO
#if (TRACK_MGMT == TRACK_MGMT_NO)
   msg->l2_track = sixtop_get_trackbesteffort();
#endif

#if (TRACK_MGMT == TRACK_MGMT_SHARED)
   msg->l2_track = sixtop_get_trackbesteffort();
#endif

#if (TRACK_MGMT == TRACK_MGMT_ISOLATION)
   memcpy(msg->l2_track.owner.addr_64b, &(icmpv6rpl_vars.dio.DODAGID[8]), 8);
   msg->l2_track.owner.type = ADDR_64B;
   msg->l2_track.instance   = (uint16_t)TRACK_IMCPv6RPL;
#endif

#if (TRACK_MGMT == TRACK_MGMT_6P_ISOLATION)
   sixtop_get_trackcontrol(&(msg->l2_track));
#endif

#if (TRACK_MGMT != TRACK_MGMT_NO)  && (TRACK_MGMT != TRACK_MGMT_SHARED)  && (TRACK_MGMT != TRACK_MGMT_ISOLATION)  && (TRACK_MGMT != TRACK_MGMT_6P_ISOLATION)
   THIS IS AN ERROR
#endif

   // set DAO destination
   msg->l3_destinationAdd.type=ADDR_128B;
   memcpy(msg->l3_destinationAdd.addr_128b,icmpv6rpl_vars.dio.DODAGID,sizeof(icmpv6rpl_vars.dio.DODAGID));
   
   //===== fill in packet
   
   //NOTE: limit to preferrred parent only the number of DAO transit addresses to send
   
   //=== transit option -- from RFC 6550, page 55 - 1 transit information header per parent is required. 
   //getting only preferred parent as transit
   numTransitParents=0;
   icmpv6rpl_getPreferredParentEui64(&address);
   packetfunctions_writeAddress(msg,&address,OW_BIG_ENDIAN);
   prefix=idmanager_getMyID(ADDR_PREFIX);
   packetfunctions_writeAddress(msg,prefix,OW_BIG_ENDIAN);
   // update transit info fields
   // from rfc6550 p.55 -- Variable, depending on whether or not the DODAG ParentAddress subfield is present.
   // poipoi xv: it is not very clear if this includes all fields in the header. or as target info 2 bytes are removed.
   // using the same pattern as in target information.
   icmpv6rpl_vars.dao_transit.optionLength  = LENGTH_ADDR128b + sizeof(icmpv6rpl_dao_transit_ht)-2;
   icmpv6rpl_vars.dao_transit.PathControl=0; //todo. this is to set the preference of this parent.      
   icmpv6rpl_vars.dao_transit.type=OPTION_TRANSIT_INFORMATION_TYPE;
           
   // write transit info in packet
   packetfunctions_reserveHeaderSize(msg,sizeof(icmpv6rpl_dao_transit_ht));
   memcpy(
          ((icmpv6rpl_dao_transit_ht*)(msg->payload)),
          &(icmpv6rpl_vars.dao_transit),
          sizeof(icmpv6rpl_dao_transit_ht)
   );
   numTransitParents++;
   
   //target information is required. RFC 6550 page 55.
   /*
   One or more Transit Information options MUST be preceded by one or
   more RPL Target options.   
   */
    numTargetParents                        = 0;
    for (nbrIdx=0;nbrIdx<MAXNUMNEIGHBORS;nbrIdx++) {
      if ((neighbors_isNeighborWithHigherDAGrank(nbrIdx))==TRUE) {
         // this neighbor is of higher DAGrank as I am. so it is my child
         
         // write it's address in DAO RFC6550 page 80 check point 1.
         neighbors_getNeighborEui64(&address,ADDR_64B,nbrIdx); 
         packetfunctions_writeAddress(msg,&address,OW_BIG_ENDIAN);
         prefix=idmanager_getMyID(ADDR_PREFIX);
         packetfunctions_writeAddress(msg,prefix,OW_BIG_ENDIAN);
        
         // update target info fields 
         // from rfc6550 p.55 -- Variable, length of the option in octets excluding the Type and Length fields.
         // poipoi xv: assuming that type and length fields refer to the 2 first bytes of the header
         icmpv6rpl_vars.dao_target.optionLength  = LENGTH_ADDR128b +sizeof(icmpv6rpl_dao_target_ht) - 2; //no header type and length
         icmpv6rpl_vars.dao_target.type  = OPTION_TARGET_INFORMATION_TYPE;
         icmpv6rpl_vars.dao_target.flags  = 0;       //must be 0
         icmpv6rpl_vars.dao_target.prefixLength = 128; //128 leading bits  -- full address.
         
         // write transit info in packet
         packetfunctions_reserveHeaderSize(msg,sizeof(icmpv6rpl_dao_target_ht));
         memcpy(
               ((icmpv6rpl_dao_target_ht*)(msg->payload)),
               &(icmpv6rpl_vars.dao_target),
               sizeof(icmpv6rpl_dao_target_ht)
         );
         
         // remember I found it
         numTargetParents++;
      }  
      //limit to MAX_TARGET_PARENTS the number of DAO target addresses to send
      //section 8.2.1 pag 67 RFC6550 -- using a subset
      // poipoi TODO base selection on ETX rather than first X.
      if (numTargetParents>=MAX_TARGET_PARENTS) break;
   }
   
   
   // stop here if no parents found
   if (numTransitParents==0) {
#ifdef _DEBUG_DAO_
      sprintf(str, "RPL - DAO stopped (no transit parent)");
      openserial_printf(COMPONENT_ICMPv6RPL, str, strlen(str));
#endif

      openqueue_freePacketBuffer(msg);
      return;
   }
   
   icmpv6rpl_vars.dao_transit.PathSequence++; //increment path sequence.
   // if you get here, you will send a DAO
   
   
   //=== DAO header
   packetfunctions_reserveHeaderSize(msg,sizeof(icmpv6rpl_dao_ht));
   memcpy(
      ((icmpv6rpl_dao_ht*)(msg->payload)),
      &(icmpv6rpl_vars.dao),
      sizeof(icmpv6rpl_dao_ht)
   );
   
   //=== ICMPv6 header
   packetfunctions_reserveHeaderSize(msg,sizeof(ICMPv6_ht));
   ((ICMPv6_ht*)(msg->payload))->type       = msg->l4_sourcePortORicmpv6Type;
   ((ICMPv6_ht*)(msg->payload))->code       = IANA_ICMPv6_RPL_DAO;
   packetfunctions_calculateChecksum(msg,(uint8_t*)&(((ICMPv6_ht*)(msg->payload))->checksum)); //call last
   

   //===== send
   if (icmpv6_send(msg) == E_SUCCESS) {
      icmpv6rpl_vars.lastDAO_tx = msg;

#ifdef _DEBUG_DAO_
      sprintf(str, "RPL - DAO pushed in the queue");
      openserial_printf(COMPONENT_ICMPv6RPL, str, strlen(str));
#endif

   } else {
      openqueue_freePacketBuffer(msg);

#ifdef _DEBUG_DAO_
      sprintf(str, "RPL - DAO tx failed = icmpv6_send() error");
      openserial_printf(COMPONENT_ICMPv6RPL, str, strlen(str));
#endif
   }


}

void icmpv6rpl_setDIOPeriod(uint16_t dioPeriod){
   uint32_t        dioPeriodRandom;
   
   icmpv6rpl_vars.dioPeriod = dioPeriod;
   dioPeriodRandom = icmpv6rpl_vars.dioPeriod - 0x80 + (openrandom_get16b()&0xff);
   opentimers_setPeriod(
       icmpv6rpl_vars.timerIdDIO,
       TIME_MS,
       dioPeriodRandom
   );
}

void icmpv6rpl_setDAOPeriod(uint16_t daoPeriod){
   uint32_t        daoPeriodRandom;
   
   icmpv6rpl_vars.daoPeriod = daoPeriod;
   daoPeriodRandom = icmpv6rpl_vars.daoPeriod - 0x80 + (openrandom_get16b()&0xff);
   opentimers_setPeriod(
       icmpv6rpl_vars.timerIdDAO,
       TIME_MS,
       daoPeriodRandom
   );
}
