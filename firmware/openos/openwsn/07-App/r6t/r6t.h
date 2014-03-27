/**
\brief CoAP schedule manager application.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, Feb. 2013.

*/


#ifndef __RSCHED_H
#define __RSCHED_H

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

#define R6T_MAXRESPONSES 20 ///< maximum number of elements to be processed by a command

//=========================== typedef =========================================

// CRUD operations for cells
typedef enum {
   CREATE_LINK                           = 0,
   READ_LINK                             = 1,
   UPDATE_LINK                           = 2,
   DELETE_LINK                           = 3,
} link_command_t;

// header
PRAGMA(pack(1)); //elements for slot info 
typedef struct {
  link_command_t type; 
  uint8_t numelem;//number of elements 
}r6t_command_t;
PRAGMA(pack());

//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t desc;
} r6t_vars_t;

//=========================== prototypes ======================================

void r6t_init();

/**
\}
\}
*/

#endif
