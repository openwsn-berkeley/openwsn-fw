/**
\brief CoAP schedule manager application.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, Feb. 2013.

*/


#ifndef __RSCHED_H
#define __RSCHED_H

/**
\addtogroup App
\{
\addtogroup rsched
\{
*/

#include "openwsn.h"
#include "schedule.h"

//=========================== define ==========================================

#define R6TUS_MAXRESPONSES 20 //max number of elements to be processed by a command.

//=========================== typedef =========================================

//CRUD OPERATIONS FOR LINKS.
typedef enum {
   CREATE_LINK                           = 0,          
   READ_LINK                             = 1,
   UPDATE_LINK                           = 2,
   DELETE_LINK                           = 3,
}link_command_t;

//header
PRAGMA(pack(1)); //elements for slot info 
typedef struct {
  link_command_t type; 
  uint8_t numelem;//number of elements 
}r6tus_command_t;
PRAGMA(pack());

//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t desc;
} r6tus_vars_t;

//=========================== prototypes ======================================

void r6tus_init();

/**
\}
\}
*/

#endif
