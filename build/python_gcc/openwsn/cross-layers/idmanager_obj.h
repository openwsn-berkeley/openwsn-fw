/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:01:55.796119.
*/
#ifndef __IDMANAGER_H
#define __IDMANAGER_H

/**
\addtogroup cross-layers
\{
\addtogroup IDManager
\{
*/

#include "openwsn_obj.h"

//=========================== define ==========================================

#define ACTION_YES      'Y'
#define ACTION_NO       'N'
#define ACTION_TOGGLE   'T'

//=========================== typedef =========================================

typedef struct {
   bool          isDAGroot;
   bool          isBridge;
   open_addr_t   my16bID;
   open_addr_t   my64bID;
   open_addr_t   myPANID;
   open_addr_t   myPrefix;
} debugIDManagerEntry_t;

//=========================== module variables ================================

typedef struct {
   bool          isDAGroot;
   bool          isBridge;
   open_addr_t   my16bID;
   open_addr_t   my64bID;
   open_addr_t   myPANID;
   open_addr_t   myPrefix;
} idmanager_vars_t;

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

void idmanager_init(OpenMote* self);
bool idmanager_getIsDAGroot(OpenMote* self);
void idmanager_setIsDAGroot(OpenMote* self, bool newRole);
bool idmanager_getIsBridge(OpenMote* self);
void idmanager_setIsBridge(OpenMote* self, bool newRole);
open_addr_t* idmanager_getMyID(OpenMote* self, uint8_t type);
owerror_t idmanager_setMyID(OpenMote* self, open_addr_t* newID);
bool idmanager_isMyAddress(OpenMote* self, open_addr_t* addr);
void idmanager_triggerAboutRoot(OpenMote* self);
void idmanager_triggerAboutBridge(OpenMote* self);

bool debugPrint_id(OpenMote* self);


/**
\}
\}
*/

#endif
