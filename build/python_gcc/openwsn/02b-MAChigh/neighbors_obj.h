/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:01:56.613357.
*/
#ifndef __NEIGHBORS_H
#define __NEIGHBORS_H

/**
\addtogroup MAChigh
\{
\addtogroup Neighbors
\{
*/
#include "openwsn_obj.h"
#include "icmpv6rpl_obj.h"

//=========================== define ==========================================

#define MAXNUMNEIGHBORS           10
#define MAXPREFERENCE             2
#define BADNEIGHBORMAXRSSI        -80 //dBm
#define GOODNEIGHBORMINRSSI       -90 //dBm
#define SWITCHSTABILITYTHRESHOLD  3
#define DEFAULTLINKCOST           15

#define MAXDAGRANK                0xffff
#define DEFAULTDAGRANK            MAXDAGRANK
#define MINHOPRANKINCREASE        256  //default value in RPL and Minimal 6TiSCH draft

//=========================== typedef =========================================

BEGIN_PACK
typedef struct {
   bool             used;
   uint8_t          parentPreference;
   bool             stableNeighbor;
   uint8_t          switchStabilityCounter;
   open_addr_t      addr_64b;
   dagrank_t        DAGrank;
   int8_t           rssi;
   uint8_t          numRx;
   uint8_t          numTx;
   uint8_t          numTxACK;
   uint8_t          numWraps;//number of times the tx counter wraps. can be removed if memory is a restriction. also check openvisualizer then.
   asn_t            asn;
   uint8_t          joinPrio;
} neighborRow_t;
END_PACK

BEGIN_PACK
typedef struct {
   uint8_t         row;
   neighborRow_t   neighborEntry;
} debugNeighborEntry_t;
END_PACK

BEGIN_PACK
typedef struct {
   uint8_t         last_addr_byte;   // last byte of the neighbor's address
   int8_t          rssi;
   uint8_t         parentPreference;
   dagrank_t       DAGrank;
   uint16_t        asn; 
} netDebugNeigborEntry_t;
END_PACK

//=========================== module variables ================================
   
typedef struct {
   neighborRow_t        neighbors[MAXNUMNEIGHBORS];
   dagrank_t            myDAGrank;
   uint8_t              debugRow;
   icmpv6rpl_dio_ht*    dio; //keep it global to be able to debug correctly.
} neighbors_vars_t;

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

void neighbors_init(OpenMote* self);

// getters
dagrank_t neighbors_getMyDAGrank(OpenMote* self);
uint8_t neighbors_getNumNeighbors(OpenMote* self);
bool neighbors_getPreferredParentEui64(OpenMote* self, open_addr_t* addressToWrite);
open_addr_t* neighbors_getKANeighbor(OpenMote* self, uint16_t kaPeriod);

// interrogators
bool neighbors_isStableNeighbor(OpenMote* self, open_addr_t* address);
bool neighbors_isPreferredParent(OpenMote* self, open_addr_t* address);
bool neighbors_isNeighborWithLowerDAGrank(OpenMote* self, uint8_t index);
bool neighbors_isNeighborWithHigherDAGrank(OpenMote* self, uint8_t index);

// updating neighbor information
void neighbors_indicateRx(OpenMote* self, 
   open_addr_t*         l2_src,
   int8_t               rssi,
   asn_t*               asnTimestamp,
   bool                 joinPrioPresent,
   uint8_t              joinPrio
);
void neighbors_indicateTx(OpenMote* self, 
   open_addr_t*         dest,
   uint8_t              numTxAttempts,
   bool                 was_finally_acked,
   asn_t*               asnTimestamp
);
void neighbors_indicateRxDIO(OpenMote* self, OpenQueueEntry_t* msg);

// get addresses
void neighbors_getNeighbor(OpenMote* self, open_addr_t* address,uint8_t addr_type,uint8_t index);
// managing routing info
void neighbors_updateMyDAGrankAndNeighborPreference(OpenMote* self);
// maintenance
void neighbors_removeOld(OpenMote* self);
// debug
bool debugPrint_neighbors(OpenMote* self);
void debugNetPrint_neighbors(OpenMote* self, netDebugNeigborEntry_t* schlist);
          
/**
\}
\}
*/

#endif
