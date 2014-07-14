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

#include "openwsn.h"
#include "opencoap.h"
#include "schedule.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t desc;
} r6t_vars_t;

//=========================== prototypes ======================================

void r6t_init(void);

/**
\}
\}
*/

#endif
