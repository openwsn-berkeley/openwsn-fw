#ifndef __NEIGHBORS_H
#define __NEIGHBORS_H

/**
\addtogroup MAChigh
\{
\addtogroup Neighbors
\{
*/
#include "openwsn.h"
#include "icmpv6rpl.h"

//=========================== define ==========================================

#define MAXNUMNEIGHBORS           10
#define MAXPREFERENCE             2
#define BADNEIGHBORMAXRSSI        -80 //dBm
#define GOODNEIGHBORMINRSSI       -90 //dBm
#define SWITCHSTABILITYTHRESHOLD  3

#define MAXDAGRANK                0xffff
#define DEFAULTDAGRANK            MAXDAGRANK

//=========================== typedef =========================================

PRAGMA(pack(1));
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
   asn_t            asn;
} neighborRow_t;
PRAGMA(pack());

PRAGMA(pack(1));
typedef struct {
   uint8_t         row;
   neighborRow_t   neighborEntry;
} debugNeighborEntry_t;
PRAGMA(pack());

PRAGMA(pack(1));
typedef struct {
   uint8_t         last_addr_byte;   // last byte of the neighbor's address
   int8_t          rssi;
   uint8_t         parentPreference;
   dagrank_t       DAGrank;
   uint16_t        asn; 
} netDebugNeigborEntry_t;
PRAGMA(pack());

//=========================== variables =======================================

//=========================== prototypes ======================================

void          neighbors_init();
// getters
dagrank_t     neighbors_getMyDAGrank();
uint8_t       neighbors_getNumNeighbors();
bool          neighbors_getPreferredParentEui64(open_addr_t* addressToWrite);
open_addr_t*  neighbors_getKANeighbor();
// interrogators
bool          neighbors_isStableNeighbor(open_addr_t* address);
bool          neighbors_isPreferredParent(open_addr_t* address);
bool          neighbors_isNeighborWithLowerDAGrank(uint8_t index);
// updating neighbor information
void          neighbors_indicateRx(
                   open_addr_t*        l2_src,
                   int8_t              rssi,
                   asn_t*              asnTimestamp
              );
void          neighbors_indicateTx(
                   open_addr_t*        dest,
                   uint8_t             numTxAttempts,
                   bool                was_finally_acked,
                   asn_t*              asnTimestamp
              );
void          neighbors_indicateRxDIO(OpenQueueEntry_t* msg);
// write addresses
void          neighbors_writeAddrLowerDAGrank(
                   uint8_t*            addressToWrite,
                   uint8_t             addr_type,
                   uint8_t             index
              );

bool          neighbors_writeAddrHigherDAGrank(
                   open_addr_t*        addressToWrite,
                   uint8_t             index
              );
// managing routing info
void          neighbors_updateMyDAGrankAndNeighborPreference();
// debug
bool          debugPrint_neighbors();
void          neighbors_getNetDebugInfo(netDebugNeigborEntry_t *schlist,uint8_t maxbytes);
          
/**
\}
\}
*/

#endif
