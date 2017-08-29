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
// sfcontrol
#include "sf0.h"
#include "schedule.h"
// sfcontrol

//=========================== variables =======================================

icmpv6rpl_vars_t             icmpv6rpl_vars;

//=========================== prototypes ======================================

// DIO-related
void icmpv6rpl_timer_DIO_cb(opentimers_id_t id);
void icmpv6rpl_timer_DIO_task(void);
void sendDIO(void);
// DAO-related
void icmpv6rpl_timer_DAO_cb(opentimers_id_t id);
void icmpv6rpl_timer_DAO_task(void);
void sendDAO(void);
// parent update cb
void icmpv6rpl_timer_parent_update_cb(opentimers_id_t id);

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
   icmpv6rpl_vars.daoSent=FALSE;
   if (idmanager_getIsDAGroot()==TRUE) {
      icmpv6rpl_vars.myDAGrank=MINHOPRANKINCREASE;
   } else {
      icmpv6rpl_vars.myDAGrank=DEFAULTDAGRANK;
   }

   //=== admin
   
   icmpv6rpl_vars.busySendingDIO            = FALSE;
   icmpv6rpl_vars.busySendingDAO            = FALSE;
   icmpv6rpl_vars.fDodagidWritten           = 0;
   icmpv6rpl_vars.lowestRankInHistory       = DEFAULTDAGRANK;
   
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
   
   icmpv6rpl_vars.dioPeriod                 = TIMER_DIO_TIMEOUT;
   icmpv6rpl_vars.timerIdDIO                = opentimers_create();

   //initialize PIO -> move this to dagroot code
   icmpv6rpl_vars.pio.type                  = RPL_OPTION_PIO;
   icmpv6rpl_vars.pio.optLen                = 30;
   icmpv6rpl_vars.pio.prefLen               = 64;
   icmpv6rpl_vars.pio.flags                 = 96;
   icmpv6rpl_vars.pio.plifetime             = 0xFFFFFFFF;
   icmpv6rpl_vars.pio.vlifetime             = 0xFFFFFFFF;
   // if not dagroot then do not initialize, will receive PIO and update fields
   // later
   if (idmanager_getIsDAGroot()){
     memcpy(
        &(icmpv6rpl_vars.pio.prefix[0]),
        idmanager_getMyID(ADDR_PREFIX)->prefix,
        sizeof(idmanager_getMyID(ADDR_PREFIX)->prefix)
     );
      memcpy(
        &(icmpv6rpl_vars.pio.prefix[8]),
        idmanager_getMyID(ADDR_64B)->addr_64b,
        sizeof(idmanager_getMyID(ADDR_64B)->addr_64b)
     );
   }
   //configuration option 
   icmpv6rpl_vars.conf.type = RPL_OPTION_CONFIG;
   icmpv6rpl_vars.conf.optLen = 14;
   icmpv6rpl_vars.conf.flagsAPCS = DEFAULT_PATH_CONTROL_SIZE; //DEFAULT_PATH_CONTROL_SIZE = 0
   icmpv6rpl_vars.conf.DIOIntDoubl = 8; //8 -> trickle period - max times it will double ~20min
   icmpv6rpl_vars.conf.DIOIntMin = 12 ; // 12 ->  min trickle period -> 16s 
   icmpv6rpl_vars.conf.DIORedun = 0 ; // 0
   icmpv6rpl_vars.conf.maxRankIncrease = 2048; //  2048
   icmpv6rpl_vars.conf.minHopRankIncrease = 256 ; //256
   icmpv6rpl_vars.conf.OCP = 0; // 0 OF0
   icmpv6rpl_vars.conf.reserved = 0;
   icmpv6rpl_vars.conf.defLifetime = 0xff; //infinite - limit for DAO period  -> 0xff 
   icmpv6rpl_vars.conf.lifetimeUnit = 0xffff; // 0xffff
   
   opentimers_scheduleIn(
       icmpv6rpl_vars.timerIdDIO,
       872 +(openrandom_get16b()&0xff),
       TIME_MS,
       TIMER_ONESHOT,
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
   
   icmpv6rpl_vars.daoPeriod                 = TIMER_DAO_TIMEOUT;
   icmpv6rpl_vars.timerIdDAO                = opentimers_create();
   opentimers_scheduleIn(
       icmpv6rpl_vars.timerIdDAO,
       872 +(openrandom_get16b()&0xff),
       TIME_MS,
       TIMER_ONESHOT,
       icmpv6rpl_timer_DAO_cb
   );
   
   // parent update timer
   icmpv6rpl_vars.timerIdParentUpdate        = opentimers_create();
   opentimers_scheduleIn(
       icmpv6rpl_vars.timerIdParentUpdate,
       TIMER_PARENT_UPDATE_TIMEOUT,
       TIME_MS,
       TIMER_PERIODIC,
       icmpv6rpl_timer_parent_update_cb
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
                                                
owerror_t icmpv6rpl_getRPLDODAGid(uint8_t* address_128b){
   if (icmpv6rpl_vars.fDodagidWritten) {
       memcpy(address_128b,icmpv6rpl_vars.dao.DODAGID,16);
       return E_SUCCESS;
   }
   return E_FAIL;
}

/**
\brief Called when DIO/DAO was sent.

\param[in] msg   Pointer to the message just sent.
\param[in] error Outcome of the sending.
*/
void icmpv6rpl_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   
   // take ownership over that packet
   msg->owner = COMPONENT_ICMPv6RPL;
   
   // make sure I created it
   if (msg->creator!=COMPONENT_ICMPv6RPL) {
      openserial_printError(COMPONENT_ICMPv6RPL,ERR_UNEXPECTED_SENDDONE,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
   }
   
   // I'm not busy sending DIO/DAO anymore
   if (packetfunctions_isBroadcastMulticast(&(msg->l2_nextORpreviousHop))){
        icmpv6rpl_vars.busySendingDIO = FALSE;
   } else {
        icmpv6rpl_vars.busySendingDAO = FALSE;
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
         if (idmanager_getIsDAGroot()==TRUE) {
            // stop here if I'm in the DAG root
            break; // break, don't return
         }
         // update routing info for that neighbor
         icmpv6rpl_indicateRxDIO(msg);
         
         break;
      
      case IANA_ICMPv6_RPL_DAO:
         // this should never happen
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

\returns TRUE and index of my preferred parent if have one, FALSE if no parent
*/
bool icmpv6rpl_getPreferredParentIndex(uint8_t* indexptr) {
    uint8_t i;
    uint16_t lowestRankIncrease;
    uint16_t lowestRankIncreaseParent;
    uint16_t rankIncrease;
    
    lowestRankIncrease = MAXDAGRANK;
    for (i=0;i<icmpv6rpl_vars.numParent;i++){
        rankIncrease = neighbors_getLinkMetric(icmpv6rpl_vars.parentIndex[i]);
        if (rankIncrease < lowestRankIncrease){
            lowestRankIncrease       = rankIncrease;
            lowestRankIncreaseParent = icmpv6rpl_vars.parentIndex[i];
        }
    }
    
    // choose lowest rank increase as preferred parent
    if (lowestRankIncrease==(3*DEFAULTLINKCOST-2)*MINHOPRANKINCREASE){
        // no statistic yet on each parent, randomly choose one (lowest rank parent has highest possibility to be chosen)
        if (openrandom_get16b()>0xffff/2){
            lowestRankIncreaseParent = icmpv6rpl_vars.parentIndex[0];
        } else {
            lowestRankIncreaseParent = icmpv6rpl_vars.parentIndex[openrandom_get16b()%(icmpv6rpl_vars.numParent)];
        }
    }
    *indexptr = lowestRankIncreaseParent;
    return (icmpv6rpl_vars.numParent>0);
}

/**
\brief Retrieve my preferred parent's EUI64 address.
\param[out] addressToWrite Where to copy the preferred parent's address to.
*/
bool icmpv6rpl_getPreferredParentEui64(open_addr_t* addressToWrite) {
    
    uint16_t parentIndex;
    if (icmpv6rpl_vars.numParent==0){
        addressToWrite = NULL;
        return FALSE;
    }
    if (openrandom_get16b()>0xffff/2){
        parentIndex = icmpv6rpl_vars.parentIndex[0];
    } else {
        parentIndex = icmpv6rpl_vars.parentIndex[openrandom_get16b()%(icmpv6rpl_vars.numParent)];
    }
    
    return neighbors_getNeighborEui64(addressToWrite,ADDR_64B,parentIndex);
}

/**
\brief Indicate whether some neighbor is the routing parent.

\param[in] address The EUI64 address of the neighbor.

\returns TRUE if that neighbor is preferred parent, FALSE otherwise.
*/
bool icmpv6rpl_isPreferredParent(open_addr_t* address) {
    open_addr_t  temp;
    uint8_t      i;
   
    // do we currently have a parent?
    if (icmpv6rpl_vars.numParent==0) {
        return FALSE;
    }
    
    if (address->type!=ADDR_64B){
        openserial_printCritical(COMPONENT_NEIGHBORS,ERR_WRONG_ADDR_TYPE,
                               (errorparameter_t)address->type,
                               (errorparameter_t)3);
        return FALSE;
    }
    
    for (i=0;i<icmpv6rpl_vars.numParent;i++){
        neighbors_getNeighborEui64(&temp,ADDR_64B,icmpv6rpl_vars.parentIndex[i]);
        if (packetfunctions_sameAddress(address,&temp)==TRUE){
            return TRUE;
        }
    }
    
    return FALSE;
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
    // temporaries
    uint16_t  rankIncrease;
    dagrank_t neighborRank;
    uint32_t  tentativeDAGrank;
    
    uint8_t   neighborIndexWithLowestRank[NUM_MAINTAINED_NEIGHBOR];
    uint8_t   highestRankParentIndex;
    
    uint16_t  parentAddress_16b;
    open_addr_t parentAddress;
    
    uint8_t   j;
    uint16_t  tempSlotoffset;
    uint16_t  parentControlSlots[PARENTS_NUM];
    
    // sfcontrol
    // if I'm a DAGroot, my DAGrank is always MINHOPRANKINCREASE
    if ((idmanager_getIsDAGroot())==TRUE) {
        // the dagrank is not set through setting command, set rank to MINHOPRANKINCREASE here 
        if (icmpv6rpl_vars.myDAGrank!=MINHOPRANKINCREASE) { // test for change so as not to report unchanged value when root
            icmpv6rpl_vars.myDAGrank=MINHOPRANKINCREASE;
        }
        return;
    }
    
    // first round to get neighbors indexes sorted by their rank
    neighbors_sortRankAndHousekeeping(neighborIndexWithLowestRank,&highestRankParentIndex);
    
    if (highestRankParentIndex==0xff){
        // no parent yet
        return;
    }
    
    if (highestRankParentIndex<PARENTS_NUM){
        icmpv6rpl_vars.numParent = highestRankParentIndex+1;
    } else {
        icmpv6rpl_vars.numParent = PARENTS_NUM;
    }
    
    // second round to check whether the candidate neighbor can be parent
    do {
        rankIncrease     = neighbors_getLinkMetric(neighborIndexWithLowestRank[icmpv6rpl_vars.numParent-1]);
        neighborRank     = neighbors_getNeighborRank(neighborIndexWithLowestRank[icmpv6rpl_vars.numParent-1]);
        tentativeDAGrank = (uint32_t)neighborRank+rankIncrease;
        
        if (tentativeDAGrank > (uint32_t)(icmpv6rpl_vars.lowestRankInHistory + (3*DEFAULTLINKCOST-2)*MINHOPRANKINCREASE)){
            icmpv6rpl_vars.numParent--;
        } else {
            icmpv6rpl_vars.myDAGrank = tentativeDAGrank;
            if (tentativeDAGrank<icmpv6rpl_vars.lowestRankInHistory){
                icmpv6rpl_vars.lowestRankInHistory = icmpv6rpl_vars.myDAGrank;
            }
            break;
        }
    }while (icmpv6rpl_vars.numParent > 0);
    
    // reset preferred parent
    for (i=0;i<MAXNUMNEIGHBORS;i++) {
        neighbors_setPreferredParent(i,0);
    }

    if (icmpv6rpl_vars.numParent == 0){
        // stop if there is no available parent
        return;
    }
    
    // neighbor with lowest rank has preference 2
    neighbors_setPreferredParent(neighborIndexWithLowestRank[0],MAXPREFERENCE);
    for (i=1;i<icmpv6rpl_vars.numParent;i++) {
        // other parents have preference 1
        neighbors_setPreferredParent(neighborIndexWithLowestRank[i],MAXPREFERENCE-1);
    }
    
    memcpy(&icmpv6rpl_vars.parentIndex[0],&neighborIndexWithLowestRank[0],icmpv6rpl_vars.numParent);
    
    // update parent sf control cells
    j = 0;
    memset(&parentControlSlots[0], 0, PARENTS_NUM);
    for (i=0;i<icmpv6rpl_vars.numParent;i++){
        if (neighbors_getNeighborEui64(&parentAddress,ADDR_64B,icmpv6rpl_vars.parentIndex[i])){
            parentAddress_16b = 256*parentAddress.addr_64b[6]+parentAddress.addr_64b[7];
            tempSlotoffset = sf0_hashFunction(parentAddress_16b);
            if (schedule_isSlotOffsetAvailable(tempSlotoffset)==TRUE){
                schedule_addActiveSlot(
                    tempSlotoffset,
                    CELLTYPE_TXRX,
                    TRUE,
                    0,
                    &parentAddress
                );
            }
            parentControlSlots[j] = tempSlotoffset;
            j++;
        }
    }
    schedule_removeDedicateTxRxCellsExcept(parentControlSlots,j);
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
   open_addr_t      myPrefix;
   uint8_t*         current;
   uint8_t          optionsLen;
   // take ownership over the packet
   msg->owner = COMPONENT_NEIGHBORS;
   
   // update some fields of our DIO 
   memcpy(
      &(icmpv6rpl_vars.dio),
      (icmpv6rpl_dio_ht*)(msg->payload),
      sizeof(icmpv6rpl_dio_ht)
   );
   
   // write DODAGID in DIO and DAO
   icmpv6rpl_writeDODAGid(&(((icmpv6rpl_dio_ht*)(msg->payload))->DODAGID[0]));
   
   // save pointer to incoming DIO header in global structure for simplfying debug.
   icmpv6rpl_vars.incomingDio = (icmpv6rpl_dio_ht*)(msg->payload);
   current = msg->payload + sizeof(icmpv6rpl_dio_ht);
   optionsLen = msg->length - sizeof(icmpv6rpl_dio_ht);
   
   while (optionsLen>0){
     switch (current[0]){
     case RPL_OPTION_CONFIG:
       // configuration option
       icmpv6rpl_vars.incomingConf = (icmpv6rpl_config_ht*)(current);
       memcpy(&icmpv6rpl_vars.conf,icmpv6rpl_vars.incomingConf, sizeof(icmpv6rpl_config_ht));
       // do whatever needs to be done with the configuration option of RPL
       optionsLen = optionsLen - current[1] - 2;
       current = current + current[1] + 2;
       break;
     case RPL_OPTION_PIO:
       // pio
        icmpv6rpl_vars.incomingPio = (icmpv6rpl_pio_t*)(current);
        // update PIO with the received one.
        memcpy(&icmpv6rpl_vars.pio,icmpv6rpl_vars.incomingPio, sizeof(icmpv6rpl_pio_t));
        // update my prefix from PIO 
        // looks like we adopt the prefix from any PIO without a question about this node being our parent??
        myPrefix.type = ADDR_PREFIX;
        memcpy(
          myPrefix.prefix,
          icmpv6rpl_vars.incomingPio->prefix,
          sizeof(myPrefix.prefix)
        );
        idmanager_setMyID(&myPrefix);
        optionsLen = optionsLen - current[1] - 2;
        current = current + current[1] + 2;
       break;
     default:
       //option not supported, just jump the len;
       optionsLen = optionsLen - current[1] - 2;
       current = current + current[1] + 2;
       break;
     }
   }
   
   // quick fix: rank is two bytes in network order: need to swap bytes
   temp_8b            = *(msg->payload+2);
   icmpv6rpl_vars.incomingDio->rank = (temp_8b << 8) + *(msg->payload+3);
   
   //update rank in DIO as well (which will be overwritten with my rank when send).
   icmpv6rpl_vars.dio.rank = icmpv6rpl_vars.incomingDio->rank;

   // update rank of that neighbor in table
   for (i=0;i<MAXNUMNEIGHBORS;i++) {
      if (neighbors_getNeighborEui64(&NeighborAddress, ADDR_64B, i)) { // this neighbor entry is in use
         if (packetfunctions_sameAddress(&(msg->l2_nextORpreviousHop),&NeighborAddress)) { // matching address
            neighborRank=neighbors_getNeighborRank(i);
            if (
              (icmpv6rpl_vars.incomingDio->rank > neighborRank) &&
              (icmpv6rpl_vars.incomingDio->rank - neighborRank) > ((3*DEFAULTLINKCOST-2)*MINHOPRANKINCREASE)
            ) {
               // the new DAGrank looks suspiciously high, only increment a bit
               neighbors_setNeighborRank(i,neighborRank + (3*DEFAULTLINKCOST-2)*MINHOPRANKINCREASE);
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
    
    icmpv6rpl_vars.numParent=FALSE;

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
void icmpv6rpl_timer_DIO_cb(opentimers_id_t id) {
    scheduler_push_task(icmpv6rpl_timer_DIO_task,TASKPRIO_RPL);
    // update the period
    opentimers_scheduleIn(
        icmpv6rpl_vars.timerIdDIO,
        872 +(openrandom_get16b()&0xff),
        TIME_MS,
        TIMER_ONESHOT,
       icmpv6rpl_timer_DIO_cb
    );
}

/**
\brief Handler for DIO timer event.

\note This function is executed in task context, called by the scheduler.
*/
void icmpv6rpl_timer_DIO_task() {
    icmpv6rpl_vars.dioTimerCounter = (icmpv6rpl_vars.dioTimerCounter+1)%icmpv6rpl_vars.dioPeriod;
    switch (icmpv6rpl_vars.dioTimerCounter) {
    case 0:
        // called every TIMER_DIO_TIMEOUT seconds
        sendDIO();
        break;
    default:
        break;
    }
}

/**
\brief Prepare and a send a RPL DIO.
*/
void sendDIO() {
   OpenQueueEntry_t*    msg;
   
   // stop if I'm not sync'ed
   if (ieee154e_isSynch()==FALSE) {
      
      // remove packets genereted by this module (DIO and DAO) from openqueue
      openqueue_removeAllCreatedBy(COMPONENT_ICMPv6RPL);
      
      // I'm not busy sending a DIO/DAO
      icmpv6rpl_vars.busySendingDIO  = FALSE;
      icmpv6rpl_vars.busySendingDAO  = FALSE;
      
      // stop here
      return;
   }
   
   // do not send DIO if I have the default DAG rank
   if (icmpv6rpl_getMyDAGrank()==DEFAULTDAGRANK) {
      return;
   }
   
   // do not send DIO if I'm already busy sending
   if (icmpv6rpl_vars.busySendingDIO==TRUE) {
      return;
   }
   
   // sfcontrol
   if (schedule_hasSFControlCell()==FALSE){
      return;
   }
   // sfcontrol
   
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
   msg->l4_protocol_compressed              = FALSE;
   msg->l4_sourcePortORicmpv6Type           = IANA_ICMPv6_RPL;
   
   // set DIO destination
   memcpy(&(msg->l3_destinationAdd),&icmpv6rpl_vars.dioDestination,sizeof(open_addr_t));
   
   //===== Configuration option
   packetfunctions_reserveHeaderSize(msg,sizeof(icmpv6rpl_config_ht));
    
   //copy the PIO in the packet
   memcpy(
      ((icmpv6rpl_pio_t*)(msg->payload)),
      &(icmpv6rpl_vars.conf),
      sizeof(icmpv6rpl_config_ht)
   );
   
   //===== PIO payload

   packetfunctions_reserveHeaderSize(msg,sizeof(icmpv6rpl_pio_t));

   // copy my prefix into the PIO
   
   memcpy(
      &(icmpv6rpl_vars.pio.prefix[0]),
      idmanager_getMyID(ADDR_PREFIX)->prefix,
      sizeof(idmanager_getMyID(ADDR_PREFIX)->prefix)
   );
   // host address is not needed. Only prefix.
   /* memcpy(
      &(icmpv6rpl_vars.pio.prefix[8]),
      idmanager_getMyID(ADDR_64B)->addr_64b,
      sizeof(idmanager_getMyID(ADDR_64B)->addr_64b)
   );*/
   
   //copy the PIO in the packet
   memcpy(
      ((icmpv6rpl_pio_t*)(msg->payload)),
      &(icmpv6rpl_vars.pio),
      sizeof(icmpv6rpl_pio_t)
   );


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
    if (icmpv6_send(msg)==E_SUCCESS) {
        icmpv6rpl_vars.busySendingDIO = TRUE; 
    } else {
        openqueue_freePacketBuffer(msg);
    }
}

//===== DAO-related

/**
\brief DAO timer callback function.

\note This function is executed in interrupt context, and should only push a
   task.
*/
void icmpv6rpl_timer_DAO_cb(opentimers_id_t id) {
    scheduler_push_task(icmpv6rpl_timer_DAO_task,TASKPRIO_RPL);
    // update the period
    opentimers_scheduleIn(
        icmpv6rpl_vars.timerIdDAO,
        872 +(openrandom_get16b()&0xff),
        TIME_MS,
        TIMER_ONESHOT,
        icmpv6rpl_timer_DAO_cb
    );
}

/**
\brief Handler for DAO timer event.

\note This function is executed in task context, called by the scheduler.
*/
void icmpv6rpl_timer_DAO_task() {
    icmpv6rpl_vars.daoTimerCounter = (icmpv6rpl_vars.daoTimerCounter+1)%icmpv6rpl_vars.daoPeriod;
    switch (icmpv6rpl_vars.daoTimerCounter) {
    case 0:
        // called every TIMER_DAO_TIMEOUT seconds
        sendDAO();
        break;
    default:
        break;
    }
}

void icmpv6rpl_timer_parent_update_cb(opentimers_id_t id){
    open_addr_t       newNexthop;
    OpenQueueEntry_t* msg;
  
    // called every TIMER_PARENT_UPDATE_TIMEOUT miliseconds
    icmpv6rpl_updateMyDAGrankAndParentSelection();
    
    // get update to date parent as nexthop
    memset(&newNexthop,0,sizeof(open_addr_t));
    icmpv6rpl_getPreferredParentEui64(&newNexthop);
    msg = openqueue_rplGetSentToNonParentPackets(&icmpv6rpl_vars.parentIndex[0],icmpv6rpl_vars.numParent);
    
    if (msg!=NULL && newNexthop.type != ADDR_NONE){
        memcpy(&msg->l2_nextORpreviousHop, &newNexthop, sizeof(open_addr_t));
        memcpy(msg->l2_nextHopPayload, &(newNexthop.addr_64b[0]), 8);
    }
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
   
   if (ieee154e_isSynch()==FALSE) {
      // I'm not sync'ed 
      
      // delete packets genereted by this module (DIO and DAO) from openqueue
      openqueue_removeAllCreatedBy(COMPONENT_ICMPv6RPL);
      
      // I'm not busy sending a DIO/DAO
      icmpv6rpl_vars.busySendingDAO = FALSE;
      icmpv6rpl_vars.busySendingDIO = FALSE;
      
      // stop here
      return;
   }
   
   // dont' send a DAO if you're the DAG root
   if (idmanager_getIsDAGroot()==TRUE) {
      return;
   }
   
   // dont' send a DAO if you did not acquire a DAGrank
   if (icmpv6rpl_getMyDAGrank()==DEFAULTDAGRANK) {
       return;
   }
   
   // dont' send a DAO if you're still busy sending the previous one
   if (icmpv6rpl_vars.busySendingDAO==TRUE) {
      if (
          icmpv6rpl_getPreferredParentEui64(&address)==TRUE && \
          openqueue_macGetPacketCreatedBy(COMPONENT_ICMPv6RPL,&address) == NULL 
      ){
          // I have old DAO in buffer its destination is not reachable, remove them
          openqueue_removeAllCreatedBy(COMPONENT_ICMPv6RPL);
          
          // I'm not busy sending DAO/DIO
          icmpv6rpl_vars.busySendingDAO = FALSE;
          icmpv6rpl_vars.busySendingDIO = FALSE;
      } else {
          return;
      }
   }
   
   // if there is no cell to the parent, stop generating DAO
   if (schedule_getCellsCounts(SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_HANDLE,CELLTYPE_TX,&address)==0){
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
   
   // set DAO destination
   msg->l3_destinationAdd.type=ADDR_128B;
   memcpy(msg->l3_destinationAdd.addr_128b,icmpv6rpl_vars.dio.DODAGID,sizeof(icmpv6rpl_vars.dio.DODAGID));
   
   //===== fill in packet
   
   //NOTE: limit to preferrred parent only the number of DAO transit addresses to send
   
   //=== transit option -- from RFC 6550, page 55 - 1 transit information header per parent is required. 
   //getting only preferred parent as transit
   numTransitParents=0;
   neighbors_getNeighborEui64(&address,ADDR_64B,icmpv6rpl_vars.parentIndex[numTransitParents]);
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
   if (icmpv6_send(msg)==E_SUCCESS) {
      icmpv6rpl_vars.busySendingDAO = TRUE;
      icmpv6rpl_vars.daoSent = TRUE;
   } else {
      openqueue_freePacketBuffer(msg);
   }
}

void icmpv6rpl_setDIOPeriod(uint16_t dioPeriod){
    // convert to seconds
    icmpv6rpl_vars.dioPeriod = dioPeriod/1000;
}

void icmpv6rpl_setDAOPeriod(uint16_t daoPeriod){
    // convert to seconds
    icmpv6rpl_vars.daoPeriod = daoPeriod/1000;
}

bool icmpv6rpl_daoSent(void) {
    if (idmanager_getIsDAGroot()==TRUE) {
        return TRUE;
    }
    return icmpv6rpl_vars.daoSent;
}

