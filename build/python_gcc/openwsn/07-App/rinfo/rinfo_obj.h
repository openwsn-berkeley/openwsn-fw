/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:02:02.997058.
*/
#ifndef __RINFO_H
#define __RINFO_H

/**
\addtogroup AppCoAP
\{
\addtogroup rinfo
\{
*/

#include "opencoap_obj.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t desc;
} rinfo_vars_t;

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

void rinfo_init(OpenMote* self);

/**
\}
\}
*/

#endif
