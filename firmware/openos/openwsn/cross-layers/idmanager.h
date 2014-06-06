#ifndef __IDMANAGER_H
#define __IDMANAGER_H

/**
\addtogroup cross-layers
\{
\addtogroup IDManager
\{
*/

#include "openwsn.h"

//=========================== define ==========================================

#define ACTION_YES      'Y'
#define ACTION_NO       'N'
#define ACTION_TOGGLE   'T'

//=========================== typedef =========================================

typedef struct {
   BOOL          isDAGroot;
   BOOL          isBridge;
   open_addr_t   my16bID;
   open_addr_t   my64bID;
   open_addr_t   myPANID;
   open_addr_t   myPrefix;
} debugIDManagerEntry_t;

//=========================== module variables ================================

typedef struct {
   BOOL          isDAGroot;
   BOOL          isBridge;
   open_addr_t   my16bID;
   open_addr_t   my64bID;
   open_addr_t   myPANID;
   open_addr_t   myPrefix;
} idmanager_vars_t;

//=========================== prototypes ======================================

void         idmanager_init();
BOOL         idmanager_getIsDAGroot();
void         idmanager_setIsDAGroot(BOOL newRole);
BOOL         idmanager_getIsBridge();
void         idmanager_setIsBridge(BOOL newRole);
open_addr_t* idmanager_getMyID(uint8_t type);
owerror_t      idmanager_setMyID(open_addr_t* newID);
BOOL         idmanager_isMyAddress(open_addr_t* addr);
void         idmanager_triggerAboutRoot();
void         idmanager_triggerAboutBridge();

BOOL         debugPrint_id();


/**
\}
\}
*/

#endif
