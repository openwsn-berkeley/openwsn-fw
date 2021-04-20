#ifndef OPENWSN_IDMANAGER_H
#define OPENWSN_IDMANAGER_H

/**
\addtogroup cross-layers
\{
\addtogroup IDManager
\{
*/

#include "opendefs.h"

//=========================== define ==========================================

#define ACTION_YES      'Y'
#define ACTION_NO       'N'
#define ACTION_TOGGLE   'T'

static const uint8_t linklocalprefix[] = {0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

//=========================== typedef =========================================

//=========================== module variables ================================

typedef struct {
    bool isDAGroot;
    open_addr_t myPANID;
    open_addr_t my16bID;
    open_addr_t my64bID;
    open_addr_t myPrefix;
    bool slotSkip;
    uint8_t joinKey[16];
    asn_t joinAsn;
} idManagerVars_t;

//=========================== prototypes ======================================

void idmanager_init(void);

bool idmanager_getIsDAGroot(void);

void idmanager_setIsDAGroot(bool newRole);

bool idmanager_getIsSlotSkip(void);

open_addr_t* idmanager_getMyID(uint8_t type);

owerror_t idmanager_setMyID(open_addr_t *newID);

bool idmanager_isMyAddress(open_addr_t *addr);

void idmanager_triggerAboutRoot(void);

void idmanager_setJoinKey(uint8_t *key);

void idmanager_setJoinAsn(asn_t *asn);

void idmanager_getJoinKey(uint8_t **pKey);

/**
\}
\}
*/

#endif /* OPENWSN_IDMANAGER_H */
