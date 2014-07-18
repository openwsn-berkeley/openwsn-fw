/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:02:02.548958.
*/
/**
\brief CoAP 6top application

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, February 2013.
\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, July 2014
*/

#ifndef __R6T_H
#define __R6T_H

/**
\addtogroup AppCoAP
\{
\addtogroup r6t
\{
*/

#include "openwsn_obj.h"
#include "opencoap_obj.h"
#include "schedule_obj.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t desc;
} r6t_vars_t;

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

void r6t_init(OpenMote* self);

/**
\}
\}
*/

#endif
