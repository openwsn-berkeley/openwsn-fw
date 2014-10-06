#ifndef __NEIGHBORS_H
#define __NEIGHBORS_H

/**
\addtogroup MAChigh
\{
\addtogroup Neighbors
\{
*/
#include "opendefs.h"
#include "icmpv6rpl.h"

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

//=========================== prototypes ======================================

void          neighbors_init(void);

// getters
dagrank_t     neighbors_getMyDAGrank(void);
uint8_t       neighbors_getNumNeighbors(void);
bool          neighbors_getPreferredParentEui64(open_addr_t* addressToWrite);
open_addr_t*  neighbors_getKANeighbor(uint16_t kaPeriod);

// interrogators
bool          neighbors_isStableNeighbor(open_addr_t* address);
bool          neighbors_isPreferredParent(open_addr_t* address);
bool          neighbors_isNeighborWithLowerDAGrank(uint8_t index);
bool          neighbors_isNeighborWithHigherDAGrank(uint8_t index);

// updating neighbor information
void          neighbors_indicateRx(
   open_addr_t*         l2_src,
   int8_t               rssi,
   asn_t*               asnTimestamp,
   bool                 joinPrioPresent,
   uint8_t              joinPrio
);
void          neighbors_indicateTx(
   open_addr_t*         dest,
   uint8_t              numTxAttempts,
   bool                 was_finally_acked,
   asn_t*               asnTimestamp
);
void          neighbors_indicateRxDIO(OpenQueueEntry_t* msg);

// get addresses
void          neighbors_getNeighbor(open_addr_t* address,uint8_t addr_type,uint8_t index);
// managing routing info
void          neighbors_updateMyDAGrankAndNeighborPreference(void);
// maintenance
void          neighbors_removeOld(void);
// debug
bool          debugPrint_neighbors(void);

/**
\}
\}
*/

#endif
