/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:02:04.121211.
*/
#ifndef __RWELLKNOWN_H
#define __RWELLKNOWN_H

/**
\addtogroup AppCoAP
\{
\addtogroup rWellKnown
\{
*/

#include "opencoap_obj.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t desc;
} rwellknown_vars_t;

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

void rwellknown_init(OpenMote* self);

/**
\}
\}
*/

#endif
