#include "opendefs.h"
#include "neighbors.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "idmanager.h"
#include "openserial.h"
#include "IEEE802154E.h"
#include "openrandom.h"
#include "msf.h"

//=========================== variables =======================================

static neighbors_vars_t neighbors_vars;

//=========================== prototypes ======================================

void registerNewNeighbor(
        open_addr_t* neighborID,
        int8_t       rssi,
        asn_t*       asnTimestamp,
        bool         joinPrioPresent,
        uint8_t      joinPrio,
        bool         insecure
     );
bool isNeighbor(open_addr_t* neighbor);
void removeNeighbor(uint8_t neighborIndex);
bool isThisRowMatching(
        open_addr_t* address,
        uint8_t      rowNumber
     );

//=========================== public ==========================================

/**
\brief Initializes this module.
*/
void neighbors_init(void) {

   // clear module variables
   memset(&neighbors_vars,0,sizeof(neighbors_vars_t));
   // The .used fields get reset to FALSE by this memset.

}

//===== getters

/**
\brief Retrieve the number of neighbors this mote's currently knows of.

\returns The number of neighbors this mote's currently knows of.
*/
uint8_t neighbors_getNumNeighbors(void) {
   uint8_t i;
   uint8_t returnVal;

   returnVal=0;
   for (i=0;i<MAXNUMNEIGHBORS;i++) {
      if (neighbors_vars.neighbors[i].used==TRUE) {
         returnVal++;
      }
   }
   return returnVal;
}

dagrank_t neighbors_getNeighborRank(uint8_t index) {
   return neighbors_vars.neighbors[index].DAGrank;
}

int8_t neighbors_getRssi(uint8_t index){
   return neighbors_vars.neighbors[index].rssi;
}

uint8_t neighbors_getNumTx(uint8_t index){
   return neighbors_vars.neighbors[index].numTx;
}
/**
\brief Find neighbor to which to send KA.

This function iterates through the neighbor table and identifies the neighbor
we need to send a KA to, if any. This neighbor satisfies the following
conditions:
- it is one of our preferred parents
- we haven't heard it for over kaPeriod

\param[in] kaPeriod The maximum number of slots I'm allowed not to have heard
   it.

\returns A pointer to the neighbor's address, or NULL if no KA is needed.
*/
open_addr_t* neighbors_getKANeighbor(uint16_t kaPeriod) {
   uint8_t         i;
   uint16_t        timeSinceHeard;

   // policy is not to KA to non-preferred parents so go strait to check if Preferred Parent is aging
   if (icmpv6rpl_getPreferredParentIndex(&i)) {      // we have a Parent
      if (neighbors_vars.neighbors[i].used==1) {     // that resolves to a neighbor in use (should always)
         timeSinceHeard = ieee154e_asnDiff(&neighbors_vars.neighbors[i].asn);
         if (timeSinceHeard>kaPeriod) {
            // this neighbor needs to be KA'ed to
            return &(neighbors_vars.neighbors[i].addr_64b);
         }
      }
   }
   return NULL;
}

/**
\brief Find neighbor which should act as a Join Proxy during the join process.

This function iterates through the neighbor table and identifies the neighbor
with lowest join priority metric to send join traffic through.

\returns A pointer to the neighbor's address, or NULL if no join proxy is found.
*/
open_addr_t* neighbors_getJoinProxy(void) {
    uint8_t i;
    uint8_t joinPrioMinimum;
    open_addr_t* joinProxy;

    uint16_t moteId;
    uint16_t slotoffset;
    uint8_t  channeloffset;
    uint16_t temp_slotoffset;

    joinPrioMinimum = 0xff;
    joinProxy = NULL;
    for (i=0;i<MAXNUMNEIGHBORS;i++) {
        if (
            neighbors_vars.neighbors[i].used==TRUE &&
            neighbors_vars.neighbors[i].stableNeighbor==TRUE &&
            neighbors_vars.neighbors[i].joinPrio <= joinPrioMinimum
        ) {
            joinProxy = &(neighbors_vars.neighbors[i].addr_64b);
            joinPrioMinimum = neighbors_vars.neighbors[i].joinPrio;
        }
    }

    if (joinProxy){
        // remove all previous installed autonomous cell
        neighbor_removeAllAutonomousTxRxCellUnicast();
        moteId = 256*joinProxy->addr_64b[6]+joinProxy->addr_64b[7];
        slotoffset          = msf_hashFunction_getSlotoffset(moteId);
        channeloffset       = msf_hashFunction_getChanneloffset(moteId);
        if (
            schedule_getAutonomousTxRxCellAnycast(&temp_slotoffset) &&
            temp_slotoffset == slotoffset
        ){
            msf_setHashCollisionFlag(TRUE);
        } else {
            // reserve the autonomous cell to joinproxy
            schedule_addActiveSlot(
                slotoffset,                                 // slot offset
                CELLTYPE_TXRX,                              // type of slot
                TRUE,                                       // shared?
                channeloffset,                              // channel offset
                joinProxy                                   // neighbor
            );
        }
    }

    return joinProxy;
}

bool neighbors_getNeighborNoResource(uint8_t index){
    return neighbors_vars.neighbors[index].f6PNORES;
}

bool neighbors_getNeighborIsInBlacklist(uint8_t index){
    return neighbors_vars.neighbors[index].inBlacklist;
}

uint8_t neighbors_getSequenceNumber(open_addr_t* address){
    uint8_t i;
    for (i=0;i<MAXNUMNEIGHBORS;i++){
        if (packetfunctions_sameAddress(address, &neighbors_vars.neighbors[i].addr_64b)){
            break;
        }
    }
    return neighbors_vars.neighbors[i].sequenceNumber;

}

//===== interrogators

/**
\brief Indicate whether some neighbor is a stable neighbor

\param[in] address The address of the neighbor, a full 128-bit IPv6 addres.

\returns TRUE if that neighbor is stable, FALSE otherwise.
*/
bool neighbors_isStableNeighbor(open_addr_t* address) {
   uint8_t     i;
   open_addr_t temp_addr_64b;
   open_addr_t temp_prefix;
   bool        returnVal;

   // by default, not stable
   returnVal  = FALSE;

   // but neighbor's IPv6 address in prefix and EUI64
   switch (address->type) {
      case ADDR_128B:
         packetfunctions_ip128bToMac64b(address,&temp_prefix,&temp_addr_64b);
         break;
      default:
         openserial_printCritical(COMPONENT_NEIGHBORS,ERR_WRONG_ADDR_TYPE,
                               (errorparameter_t)address->type,
                               (errorparameter_t)0);
         return returnVal;
   }

   // iterate through neighbor table
   for (i=0;i<MAXNUMNEIGHBORS;i++) {
      if (isThisRowMatching(&temp_addr_64b,i) && neighbors_vars.neighbors[i].stableNeighbor==TRUE) {
         returnVal  = TRUE;
         break;
      }
   }

   return returnVal;
}

/**
\brief Indicate whether some neighbor is a stable neighbor

\param[in] index into the neighbor table.

\returns TRUE if that neighbor is in use and stable, FALSE otherwise.
*/
bool neighbors_isStableNeighborByIndex(uint8_t index) {
   return (neighbors_vars.neighbors[index].stableNeighbor &&
           neighbors_vars.neighbors[index].used);
}

/**
\brief Indicate whether some neighbor is an insecure neighbor

\param[in] address The address of the neighbor, a 64-bit address.

\returns TRUE if that neighbor is insecure, FALSE otherwise.
*/
bool neighbors_isInsecureNeighbor(open_addr_t* address) {
   uint8_t     i;
   bool        returnVal;

   // if not found, insecure
   returnVal  = TRUE;

   switch (address->type) {
      case ADDR_64B:
         break;
      default:
         openserial_printCritical(COMPONENT_NEIGHBORS,ERR_WRONG_ADDR_TYPE,
                               (errorparameter_t)address->type,
                               (errorparameter_t)0);
         return returnVal;
   }

   // iterate through neighbor table
   for (i=0;i<MAXNUMNEIGHBORS;i++) {
      if (isThisRowMatching(address,i)) {
         returnVal  = neighbors_vars.neighbors[i].insecure;
         break;
      }
   }

   return returnVal;
}

/**
\brief Indicate whether some neighbor has a lower DAG rank that me.

\param[in] index The index of that neighbor in the neighbor table.

\returns TRUE if that neighbor is in use and has a higher DAG rank than me, FALSE otherwise.
*/
bool neighbors_isNeighborWithHigherDAGrank(uint8_t index) {
   bool    returnVal;

   if (neighbors_vars.neighbors[index].used==TRUE &&
       neighbors_vars.neighbors[index].DAGrank >= icmpv6rpl_getMyDAGrank()) {
      returnVal = TRUE;
   } else {
      returnVal = FALSE;
   }

   return returnVal;
}

bool neighbors_reachedMinimalTransmission(uint8_t index){
    bool    returnVal;

    if (
        neighbors_vars.neighbors[index].used  == TRUE &&
        neighbors_vars.neighbors[index].numTx >  MINIMAL_NUM_TX
    ) {
        returnVal = TRUE;
    } else {
        returnVal = FALSE;
    }

    return returnVal;
}

//===== updating neighbor information

/**
\brief Indicate some (non-ACK) packet was received from a neighbor.

This function should be called for each received (non-ACK) packet so neighbor
statistics in the neighbor table can be updated.

The fields which are updated are:
- numRx
- rssi
- asn
- stableNeighbor
- switchStabilityCounter

\param[in] l2_src MAC source address of the packet, i.e. the neighbor who sent
   the packet just received.
\param[in] rssi   RSSI with which this packet was received.
\param[in] asnTs  ASN at which this packet was received.
\param[in] joinPrioPresent Whether a join priority was present in the received
   packet.
\param[in] joinPrio The join priority present in the packet, if any.
*/
void neighbors_indicateRx(open_addr_t* l2_src,
                          int8_t       rssi,
                          asn_t*       asnTs,
                          bool         joinPrioPresent,
                          uint8_t      joinPrio,
                          bool         insecure) {
   uint8_t i;
   bool    newNeighbor;

   // update existing neighbor
   newNeighbor = TRUE;
   for (i=0;i<MAXNUMNEIGHBORS;i++) {
      if (isThisRowMatching(l2_src,i)) {

         // this is not a new neighbor
         newNeighbor = FALSE;

         // whether the neighbor is considered as secure or not
         neighbors_vars.neighbors[i].insecure = insecure;

         // update numRx, rssi, asn
         neighbors_vars.neighbors[i].numRx++;
         neighbors_vars.neighbors[i].rssi=rssi;
         memcpy(&neighbors_vars.neighbors[i].asn,asnTs,sizeof(asn_t));
         //update jp
         if (joinPrioPresent==TRUE){
            neighbors_vars.neighbors[i].joinPrio=joinPrio;
         }

         // update stableNeighbor, switchStabilityCounter
         if (neighbors_vars.neighbors[i].stableNeighbor==FALSE) {
            if (neighbors_vars.neighbors[i].rssi>BADNEIGHBORMAXRSSI) {
               neighbors_vars.neighbors[i].switchStabilityCounter++;
               if (neighbors_vars.neighbors[i].switchStabilityCounter>=SWITCHSTABILITYTHRESHOLD) {
                  neighbors_vars.neighbors[i].switchStabilityCounter=0;
                  neighbors_vars.neighbors[i].stableNeighbor=TRUE;
               }
            } else {
               neighbors_vars.neighbors[i].switchStabilityCounter=0;
            }
         } else if (neighbors_vars.neighbors[i].stableNeighbor==TRUE) {
            if (neighbors_vars.neighbors[i].rssi<GOODNEIGHBORMINRSSI) {
               neighbors_vars.neighbors[i].switchStabilityCounter++;
               if (neighbors_vars.neighbors[i].switchStabilityCounter>=SWITCHSTABILITYTHRESHOLD) {
                  neighbors_vars.neighbors[i].switchStabilityCounter=0;
                   neighbors_vars.neighbors[i].stableNeighbor=FALSE;
               }
            } else {
               neighbors_vars.neighbors[i].switchStabilityCounter=0;
            }
         }

         // stop looping
         break;
      }
   }

   // register new neighbor
   if (newNeighbor==TRUE) {
      registerNewNeighbor(l2_src, rssi, asnTs, joinPrioPresent, joinPrio, insecure);
   }
}

/**
\brief Indicate some packet was sent to some neighbor.

This function should be called for each transmitted (non-ACK) packet so
neighbor statistics in the neighbor table can be updated.

The fields which are updated are:
- numTx
- numTxACK
- asn

\param[in] l2_dest MAC destination address of the packet, i.e. the neighbor
   who I just sent the packet to.
\param[in] numTxAttempts Number of transmission attempts to this neighbor.
\param[in] was_finally_acked TRUE iff the packet was ACK'ed by the neighbor
   on final transmission attempt.
\param[in] asnTs ASN of the last transmission attempt.
*/
void neighbors_indicateTx(
    open_addr_t* l2_dest,
    uint8_t      numTxAttempts,
    bool         was_finally_acked,
    asn_t*       asnTs
) {
    uint8_t i;
    // don't run through this function if packet was sent to broadcast address
    if (packetfunctions_isBroadcastMulticast(l2_dest)==TRUE) {
        return;
    }

    // loop through neighbor table
    for (i=0;i<MAXNUMNEIGHBORS;i++) {
        if (isThisRowMatching(l2_dest,i)) {
            // handle roll-over case

            if (neighbors_vars.neighbors[i].numTx>(0xff-numTxAttempts)) {
                neighbors_vars.neighbors[i].numWraps++; //counting the number of times that tx wraps.
                neighbors_vars.neighbors[i].numTx/=2;
                neighbors_vars.neighbors[i].numTxACK/=2;
            }
            // update statistics
            neighbors_vars.neighbors[i].numTx += numTxAttempts;

            if (was_finally_acked==TRUE) {
                neighbors_vars.neighbors[i].numTxACK++;
                memcpy(&neighbors_vars.neighbors[i].asn,asnTs,sizeof(asn_t));
            }

            // numTx and numTxAck changed,, update my rank
            icmpv6rpl_updateMyDAGrankAndParentSelection();
            break;
        }
    }
}

void neighbors_updateSequenceNumber(open_addr_t* address){
    uint8_t i;
    for (i=0;i<MAXNUMNEIGHBORS;i++){
        if (packetfunctions_sameAddress(address, &neighbors_vars.neighbors[i].addr_64b)){
            neighbors_vars.neighbors[i].sequenceNumber = (neighbors_vars.neighbors[i].sequenceNumber+1) & 0xFF;
            // rollover from 0xff to 0x01
            if (neighbors_vars.neighbors[i].sequenceNumber == 0){
                neighbors_vars.neighbors[i].sequenceNumber = 1;
            }
            break;
        }
    }
}

void neighbors_resetSequenceNumber(open_addr_t* address){
    uint8_t i;
    for (i=0;i<MAXNUMNEIGHBORS;i++){
        if (packetfunctions_sameAddress(address, &neighbors_vars.neighbors[i].addr_64b)){
            neighbors_vars.neighbors[i].sequenceNumber = 0;
            break;
        }
    }
}

//===== write addresses

/**
\brief Write the 64-bit address of some neighbor to some location.
// Returns false if neighbor not in use or address type is not 64bits
*/
bool  neighbors_getNeighborEui64(open_addr_t* address, uint8_t addr_type, uint8_t index){
   bool ReturnVal = FALSE;
   switch(addr_type) {
      case ADDR_64B:
         memcpy(&(address->addr_64b),&(neighbors_vars.neighbors[index].addr_64b.addr_64b),LENGTH_ADDR64b);
         address->type=ADDR_64B;
         ReturnVal=neighbors_vars.neighbors[index].used;
         break;
      default:
         openserial_printCritical(COMPONENT_NEIGHBORS,ERR_WRONG_ADDR_TYPE,
                               (errorparameter_t)addr_type,
                               (errorparameter_t)1);
         break;
   }
   return ReturnVal;
}
// ==== update backoff
void neighbors_updateBackoff(open_addr_t* address){
    uint8_t i;
    for (i=0;i<MAXNUMNEIGHBORS;i++){
        if (packetfunctions_sameAddress(address, &neighbors_vars.neighbors[i].addr_64b)){
            // increase the backoffExponent
            if (neighbors_vars.neighbors[i].backoffExponenton<MAXBE) {
                neighbors_vars.neighbors[i].backoffExponenton++;
            }
            // set the backoff to a random value in [0..2^BE]
            neighbors_vars.neighbors[i].backoff = openrandom_get16b()%(1<<neighbors_vars.neighbors[i].backoffExponenton);
            break;
        }
    }
}
void neighbors_decreaseBackoff(open_addr_t* address){
    uint8_t i;
    for (i=0;i<MAXNUMNEIGHBORS;i++){
        if (packetfunctions_sameAddress(address, &neighbors_vars.neighbors[i].addr_64b)){
            if (neighbors_vars.neighbors[i].backoff>0) {
                neighbors_vars.neighbors[i].backoff--;
            }
            break;
        }
    }
}
bool neighbors_backoffHitZero(open_addr_t* address){
    uint8_t i;
    bool returnVal;

    returnVal = FALSE;
    for (i=0;i<MAXNUMNEIGHBORS;i++){
        if (packetfunctions_sameAddress(address, &neighbors_vars.neighbors[i].addr_64b)){
            returnVal = (neighbors_vars.neighbors[i].backoff==0);
            break;
        }
    }
    return returnVal;
}

void neighbors_resetBackoff(open_addr_t* address){
    uint8_t i;

    for (i=0;i<MAXNUMNEIGHBORS;i++){
        if (packetfunctions_sameAddress(address, &neighbors_vars.neighbors[i].addr_64b)){
            neighbors_vars.neighbors[i].backoffExponenton     = MINBE-1;
            neighbors_vars.neighbors[i].backoff               = 0;
            break;
        }
    }
}

//===== setters

void neighbors_setNeighborRank(uint8_t index, dagrank_t rank) {
   neighbors_vars.neighbors[index].DAGrank=rank;

}

void neighbors_setNeighborNoResource(open_addr_t* address){
   uint8_t i;

   // loop through neighbor table
   for (i=0;i<MAXNUMNEIGHBORS;i++) {
      if (isThisRowMatching(address,i)) {
          neighbors_vars.neighbors[i].f6PNORES = TRUE;
          icmpv6rpl_updateMyDAGrankAndParentSelection();
          break;
      }
   }
}

void neighbors_setPreferredParent(uint8_t index, bool isPreferred){

    neighbors_vars.neighbors[index].parentPreference = isPreferred;
}

void neighbor_removeAutonomousTxRxCellUnicast(open_addr_t* address){

    uint16_t moteId;
    uint16_t slotoffset;
    uint16_t temp_slotoffset;

    moteId = 256*address->addr_64b[6]+address->addr_64b[7];
    slotoffset          = msf_hashFunction_getSlotoffset(moteId);

    if (
        schedule_getAutonomousTxRxCellAnycast(&temp_slotoffset) &&
        temp_slotoffset == slotoffset
    ){
        msf_setHashCollisionFlag(FALSE);
    } else {
        schedule_removeActiveSlot(
            slotoffset,             // slotoffset
            address                 // neighbor
        );
    }
}

void neighbor_removeAllAutonomousTxRxCellUnicast(void){

    schedule_removeAllAutonomousTxRxCellUnicast();
}

//===== managing routing info

/**
\brief return the link cost to a neighbor, expressed as a rank increase from this neighbor to this node

This really belongs to icmpv6rpl but it would require a much more complex interface to the neighbor table
*/

uint16_t neighbors_getLinkMetric(uint8_t index) {
    uint16_t  rankIncrease;
    uint32_t  rankIncreaseIntermediary; // stores intermediary results of rankIncrease calculation

    // we assume that this neighbor has already been checked for being in use
    // calculate link cost to this neighbor
    if (neighbors_vars.neighbors[index].numTxACK==0) {
        if (neighbors_vars.neighbors[index].numTx > DEFAULTLINKCOST){
            if (neighbors_vars.neighbors[index].numTx < MINIMAL_NUM_TX){
                rankIncrease = (3*neighbors_vars.neighbors[index].numTx-2)*MINHOPRANKINCREASE;
            } else {
                rankIncrease = 65535;
            }
        } else {
            rankIncrease = (3*DEFAULTLINKCOST-2)*MINHOPRANKINCREASE;
        }
    } else {
        //6TiSCH minimal draft using OF0 for rank computation: ((3*numTx/numTxAck)-2)*minHopRankIncrease
        // numTx is on 8 bits, so scaling up 10 bits won't lead to saturation
        // but this <<10 followed by >>10 does not provide any benefit either. Result is the same.
        rankIncreaseIntermediary = (((uint32_t)neighbors_vars.neighbors[index].numTx) << 10);
        rankIncreaseIntermediary = (3*rankIncreaseIntermediary * MINHOPRANKINCREASE) / ((uint32_t)neighbors_vars.neighbors[index].numTxACK);
        rankIncreaseIntermediary = rankIncreaseIntermediary - ((uint32_t)(2 * MINHOPRANKINCREASE)<<10);
        // this could still overflow for numTx large and numTxAck small, Casting to 16 bits will yiel the least significant bits
        if (rankIncreaseIntermediary >= (65536<<10)) {
            rankIncrease = 65535;
        } else {
            rankIncrease = (uint16_t)(rankIncreaseIntermediary >> 10);
        }

        if (
            rankIncrease>(3*DEFAULTLINKCOST-2)*MINHOPRANKINCREASE &&
            neighbors_vars.neighbors[index].numTx > MINIMAL_NUM_TX
        ){
            // PDR too low, put the neighbor in blacklist
            neighbors_vars.neighbors[index].inBlacklist = TRUE;
        }
    }
    return rankIncrease;
}

//===== maintenance

void  neighbors_removeOld(void) {
    uint8_t    i, j;
    bool       haveParent;
    PORT_TIMER_WIDTH timeSinceHeard;
    open_addr_t addressToWrite;

    if (
        icmpv6rpl_getPreferredParentEui64(&addressToWrite) == FALSE      ||
        (
            icmpv6rpl_getPreferredParentEui64(&addressToWrite)           &&
            schedule_hasAutonomousTxRxCellUnicast(&addressToWrite)== FALSE
        )
    ) {
        return;
    }

    // remove old neighbor
    for (i=0;i<MAXNUMNEIGHBORS;i++) {
        if (neighbors_vars.neighbors[i].used==1) {
            timeSinceHeard = ieee154e_asnDiff(&neighbors_vars.neighbors[i].asn);
            if (timeSinceHeard>(2*DESYNCTIMEOUT)) {
                if (
                    neighbors_vars.neighbors[i].f6PNORES    == FALSE &&
                    neighbors_vars.neighbors[i].inBlacklist == FALSE
                ){
                    removeNeighbor(i);
                }
                haveParent = icmpv6rpl_getPreferredParentIndex(&j);
                if (haveParent && (i==j)) { // this is our preferred parent, carefully!
                    icmpv6rpl_killPreferredParent();
                    icmpv6rpl_updateMyDAGrankAndParentSelection();
                }
            }
        }
    }
}

//===== debug

/**
\brief Triggers this module to print status information, over serial.

debugPrint_* functions are used by the openserial module to continuously print
status information about several modules in the OpenWSN stack.

\returns TRUE if this function printed something, FALSE otherwise.
*/
bool debugPrint_neighbors(void) {
    debugNeighborEntry_t temp;
    neighbors_vars.debugRow=(neighbors_vars.debugRow+1)%MAXNUMNEIGHBORS;
    temp.row=neighbors_vars.debugRow;
    temp.neighborEntry=neighbors_vars.neighbors[neighbors_vars.debugRow];
    openserial_printStatus(STATUS_NEIGHBORS,(uint8_t*)&temp,sizeof(debugNeighborEntry_t));
    return TRUE;
}

//=========================== private =========================================

void registerNewNeighbor(open_addr_t* address,
                         int8_t       rssi,
                         asn_t*       asnTimestamp,
                         bool         joinPrioPresent,
                         uint8_t      joinPrio,
                         bool         insecure) {
    uint8_t  i;

    // filter errors
    if (address->type!=ADDR_64B) {
        openserial_printCritical(COMPONENT_NEIGHBORS,ERR_WRONG_ADDR_TYPE,
                            (errorparameter_t)address->type,
                            (errorparameter_t)2);
        return;
    }
    // add this neighbor
    if (isNeighbor(address)==FALSE) {
        i=0;
        while(i<MAXNUMNEIGHBORS) {
            if (neighbors_vars.neighbors[i].used==FALSE) {
                if (rssi < GOODNEIGHBORMINRSSI){
                    break;
                }
                // add this neighbor
                neighbors_vars.neighbors[i].used                   = TRUE;
                neighbors_vars.neighbors[i].insecure               = insecure;
                // neighbors_vars.neighbors[i].stableNeighbor         = FALSE;
                // Note: all new neighbors are consider stable
                neighbors_vars.neighbors[i].stableNeighbor         = TRUE;
                neighbors_vars.neighbors[i].switchStabilityCounter = 0;
                memcpy(&neighbors_vars.neighbors[i].addr_64b,address,sizeof(open_addr_t));
                neighbors_vars.neighbors[i].DAGrank                = DEFAULTDAGRANK;
                // since we don't have a DAG rank at this point, no need to call for routing table update
                neighbors_vars.neighbors[i].rssi                   = rssi;
                neighbors_vars.neighbors[i].numRx                  = 1;
                neighbors_vars.neighbors[i].numTx                  = 0;
                neighbors_vars.neighbors[i].numTxACK               = 0;
                memcpy(&neighbors_vars.neighbors[i].asn,asnTimestamp,sizeof(asn_t));
                neighbors_vars.neighbors[i].backoffExponenton      = MINBE-1;;
                neighbors_vars.neighbors[i].backoff                = 0;
                //update jp
                if (joinPrioPresent==TRUE){
                    neighbors_vars.neighbors[i].joinPrio=joinPrio;
                } else {
                    neighbors_vars.neighbors[i].joinPrio=DEFAULTJOINPRIORITY;
                }
                break;
            }
            i++;
        }
        if (i==MAXNUMNEIGHBORS) {
            openserial_printError(COMPONENT_NEIGHBORS,ERR_NEIGHBORS_FULL,
                               (errorparameter_t)MAXNUMNEIGHBORS,
                               (errorparameter_t)0);
            return;
        }
    }
}

bool isNeighbor(open_addr_t* neighbor) {
   uint8_t i=0;
   for (i=0;i<MAXNUMNEIGHBORS;i++) {
      if (isThisRowMatching(neighbor,i)) {
         return TRUE;
      }
   }
   return FALSE;
}

void removeNeighbor(uint8_t neighborIndex) {

    uint16_t moteId;
    uint16_t slotoffset;
    uint16_t temp_slotoffset;

    neighbors_vars.neighbors[neighborIndex].used                      = FALSE;
    neighbors_vars.neighbors[neighborIndex].parentPreference          = 0;
    neighbors_vars.neighbors[neighborIndex].stableNeighbor            = FALSE;
    neighbors_vars.neighbors[neighborIndex].switchStabilityCounter    = 0;
    neighbors_vars.neighbors[neighborIndex].DAGrank                   = DEFAULTDAGRANK;
    neighbors_vars.neighbors[neighborIndex].rssi                      = 0;
    neighbors_vars.neighbors[neighborIndex].numRx                     = 0;
    neighbors_vars.neighbors[neighborIndex].numTx                     = 0;
    neighbors_vars.neighbors[neighborIndex].numTxACK                  = 0;
    neighbors_vars.neighbors[neighborIndex].asn.bytes0and1            = 0;
    neighbors_vars.neighbors[neighborIndex].asn.bytes2and3            = 0;
    neighbors_vars.neighbors[neighborIndex].asn.byte4                 = 0;
    neighbors_vars.neighbors[neighborIndex].f6PNORES                  = FALSE;
    neighbors_vars.neighbors[neighborIndex].sequenceNumber            = 0;
    neighbors_vars.neighbors[neighborIndex].backoffExponenton         = MINBE-1;;
    neighbors_vars.neighbors[neighborIndex].backoff                   = 0;

    if (
        schedule_hasAutonomousTxRxCellUnicast(&(neighbors_vars.neighbors[neighborIndex].addr_64b))
    ){
        moteId = 256*neighbors_vars.neighbors[neighborIndex].addr_64b.addr_64b[6]+\
                     neighbors_vars.neighbors[neighborIndex].addr_64b.addr_64b[7];
        slotoffset = msf_hashFunction_getSlotoffset(moteId);
        if (
            schedule_getAutonomousTxRxCellAnycast(&temp_slotoffset) &&
            slotoffset == temp_slotoffset
        ) {
            msf_setHashCollisionFlag(FALSE);
        } else {
            schedule_removeActiveSlot(
                slotoffset,                                         // slot offset
                &(neighbors_vars.neighbors[neighborIndex].addr_64b) // neighbor
            );
        }
    }

    neighbors_vars.neighbors[neighborIndex].addr_64b.type             = ADDR_NONE;
}

//=========================== helpers =========================================

bool isThisRowMatching(open_addr_t* address, uint8_t rowNumber) {
   switch (address->type) {
      case ADDR_64B:
         return neighbors_vars.neighbors[rowNumber].used &&
                packetfunctions_sameAddress(address,&neighbors_vars.neighbors[rowNumber].addr_64b);
      default:
         openserial_printCritical(COMPONENT_NEIGHBORS,ERR_WRONG_ADDR_TYPE,
                               (errorparameter_t)address->type,
                               (errorparameter_t)3);
         return FALSE;
   }
}
