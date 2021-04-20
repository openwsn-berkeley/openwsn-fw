#ifndef OPENWSN_NEIGHBORS_H
#define OPENWSN_NEIGHBORS_H

/**
\addtogroup MAChigh
\{
\addtogroup Neighbors
\{
*/
#include "opendefs.h"
#include "icmpv6rpl.h"

//=========================== define ==========================================

#define MAXPREFERENCE             2
#ifndef BADNEIGHBORMAXRSSI
#define BADNEIGHBORMAXRSSI        (-70) //dBm
#endif
#ifndef GOODNEIGHBORMINRSSI
#define GOODNEIGHBORMINRSSI       (-80) //dBm
#endif
#ifndef SWITCHSTABILITYTHRESHOLD
#define SWITCHSTABILITYTHRESHOLD  3
#endif
#ifndef DEFAULTLINKCOST
#define DEFAULTLINKCOST           4
#endif
#define MINIMAL_NUM_TX            16

#define MAXDAGRANK                0xffff
#define DEFAULTDAGRANK            MAXDAGRANK
#define MINHOPRANKINCREASE        256  // default value in RPL and Minimal 6TiSCH draft
#define DAGMAXRANKINCREASE        768  // per RFC6550 https://tools.ietf.org/html/rfc6550#section-8.2.2.4 point 3

#define DEFAULTJOINPRIORITY       0xff

//=========================== typedef =========================================

typedef struct {
    neighborRow_t neighbors[MAXNUMNEIGHBORS];
    dagrank_t myDAGrank;
    uint8_t debugRow;
} neighborsVars_t;

//=========================== prototypes ======================================

void neighbors_init(void);

// getters
dagrank_t neighbors_getNeighborRank(uint8_t index);

uint8_t neighbors_getNumNeighbors(void);

uint16_t neighbors_getLinkMetric(uint8_t index);

open_addr_t* neighbors_getKANeighbor(uint16_t kaPeriod);

open_addr_t* neighbors_getJoinProxy(void);

bool neighbors_getNeighborNoResource(uint8_t index);

int8_t neighbors_getRssi(uint8_t index);

uint8_t neighbors_getNumTx(uint8_t index);

uint8_t neighbors_getSequenceNumber(open_addr_t *address);

// setters
void neighbors_setNeighborRank(uint8_t index, dagrank_t rank);

void neighbors_setNeighborNoResource(open_addr_t *address);

void neighbors_setPreferredParent(uint8_t index, bool isPreferred);

// interrogators
bool neighbors_isStableNeighbor(open_addr_t *address);

bool neighbors_isStableNeighborByIndex(uint8_t index);

bool neighbors_isInsecureNeighbor(open_addr_t *address);

bool neighbors_isNeighborWithHigherDAGrank(uint8_t index);

bool neighbors_reachedMinimalTransmission(uint8_t index);

// updating neighbor information
void neighbors_indicateRx(
        open_addr_t *l2_src,
        int8_t rssi,
        asn_t *asnTimestamp,
        bool joinPrioPresent,
        uint8_t joinPrio,
        bool insecure
);

void neighbors_indicateTx(
        open_addr_t *dest,
        uint8_t numTxAttempts,
        bool sentOnTxCell,
        bool wasAcked,
        asn_t *asnTimestamp
);

void neighbors_updateSequenceNumber(open_addr_t *address);

void neighbors_resetSequenceNumber(open_addr_t *address);

// get addresses
bool neighbors_getNeighborEui64(open_addr_t *address, uint8_t addr_type, uint8_t index);

// update backoff field
void neighbors_updateBackoff(open_addr_t *address);

void neighbors_decreaseBackoff(open_addr_t *address);

bool neighbors_backoffHitZero(open_addr_t *address);

void neighbors_resetBackoff(open_addr_t *address);

// maintenance
void neighbors_removeOld(void);

/**
\}
\}
*/

#endif /* OPENWSN_NEIGHBORS_H */
