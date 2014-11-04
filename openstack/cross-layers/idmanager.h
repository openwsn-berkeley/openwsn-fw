#ifndef __IDMANAGER_H
#define __IDMANAGER_H

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

//=========================== typedef =========================================

BEGIN_PACK

typedef struct {
   bool          isDAGroot;
   uint8_t       myPANID[2];
   uint8_t       my16bID[2];
   uint8_t       my64bID[8];
   uint8_t       myPrefix[8];
} debugIDManagerEntry_t;

END_PACK

//=========================== module variables ================================

typedef struct {
   bool          isDAGroot;
   open_addr_t   myPANID;
   open_addr_t   my16bID;
   open_addr_t   my64bID;
   open_addr_t   myPrefix;
} idmanager_vars_t;

//=========================== prototypes ======================================

void         idmanager_init(void);
bool         idmanager_getIsDAGroot(void);
void         idmanager_setIsDAGroot(bool newRole);
open_addr_t* idmanager_getMyID(uint8_t type);
owerror_t    idmanager_setMyID(open_addr_t* newID);
bool         idmanager_isMyAddress(open_addr_t* addr);
void         idmanager_triggerAboutRoot(void);

bool         debugPrint_id(void);

/**
\}
\}
*/

#endif
