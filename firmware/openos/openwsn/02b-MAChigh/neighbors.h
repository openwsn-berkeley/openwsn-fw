#ifndef __NEIGHBORS_H
#define __NEIGHBORS_H

/**
\addtogroup MAChigh
\{
\addtogroup Neighbors
\{
*/

//=========================== define ==========================================

#define MAXNUMNEIGHBORS            10
#define MAXPREFERENCE               2
#define BADNEIGHBORMAXRSSI        -80 //dBm
#define GOODNEIGHBORMINRSSI       -90 //dBm
#define SWITCHSTABILITYTHRESHOLD    3

//=========================== typedef =========================================

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

typedef struct {
   uint8_t         row;
   neighborRow_t   neighborEntry;
} debugNeighborEntry_t;

//=========================== variables =======================================

//=========================== prototypes ======================================

          void          neighbors_init();
          void          neighbors_receiveDIO(OpenQueueEntry_t* msg);
          void          neighbors_updateMyDAGrankAndNeighborPreference();
          void          neighbors_indicateRx(open_addr_t* l2_src,
                                             int8_t       rssi,
                                             asn_t*       asnTimestamp);
          void          neighbors_indicateTx(open_addr_t* dest,
                                             uint8_t      numTxAttempts,
                                             bool         was_finally_acked,
                                             asn_t*       asnTimestamp);
          open_addr_t*  neighbors_KaNeighbor();
          bool          neighbors_isStableNeighbor(open_addr_t* address);
__monitor bool          neighbors_isPreferredParent(open_addr_t* address);
          dagrank_t     neighbors_getMyDAGrank();
          uint8_t       neighbors_getNumNeighbors();
          void          neighbors_getPreferredParent(open_addr_t* addressToWrite,
                                                     uint8_t addr_type);
          bool          debugPrint_neighbors();

/**
\}
\}
*/

#endif
