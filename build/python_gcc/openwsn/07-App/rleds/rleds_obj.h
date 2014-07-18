/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:02:03.375414.
*/
#ifndef __RLEDS_H
#define __RLEDS_H

/**
\addtogroup AppCoAP
\{
\addtogroup netLeds
\{
*/

#include "opencoap_obj.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t desc;
} rleds_vars_t;

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

void rleds__init(OpenMote* self);

/**
\}
\}
*/

#endif
