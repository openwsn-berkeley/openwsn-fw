#ifndef __SFY_HASHTABLE_H
#define __SFY_HASHTABLE_H

/**
\addtogroup MAChigh
\{
\addtogroup sfy_hashtable
\{
*/

#include "opendefs.h"

//=========================== define ==========================================

//=========================== typedef =========================================

typedef struct {
  uint16_t    sharedSlot_me;
  uint16_t    sharedSlot_parent;
  open_addr_t previousParent;
}sfy_hashtable_vars_t;

//=========================== module variables ================================

//=========================== prototypes ======================================

// admin
void      sfy_hashtable_init();

bool      sfy_hashtable_getCellInfo(open_addr_t* address,uint16_t* slotOffset, uint8_t* channelOffset);
void      sfy_hashtable_updateSharedslotParent(open_addr_t* address);
// ==== getter
uint16_t  sfy_hashtable_getSharedSlotMe();
uint16_t  sfy_hashtable_getSharedSlotParent();
/**
\}
\}
*/

#endif
