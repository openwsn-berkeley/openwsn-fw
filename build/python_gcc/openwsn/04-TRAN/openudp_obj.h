/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:02:09.910574.
*/
#ifndef __OPENUDP_H
#define __OPENUDP_H

/**
\addtogroup Transport
\{
\addtogroup OpenUdp
\{
*/

//=========================== define ==========================================

enum UDP_enums {
   UDP_ID        = 3,
   UDP_CHECKSUM  = 2,
   UDP_PORTS     = 0,
};

enum UDP_ID_enums {
   UDP_ID_DEFAULT = 0x1E,
};

enum UDP_CHECKSUM_enums {
   UDP_CHECKSUM_INLINE  = 0,
   UDP_CHECKSUM_ELIDED  = 1,
};

enum UDP_PORTS_enums {
   UDP_PORTS_16b_SRC_16b_DEST_INLINE = 0,
   UDP_PORTS_16b_SRC_8b_DEST_INLINE  = 1,
   UDP_PORTS_8b_SRC_16b_DEST_INLINE  = 2,
   UDP_PORTS_4b_SRC_4b_DEST_INLINE   = 3,
};

//=========================== typedef =========================================

typedef struct {
   uint16_t port_src;
   uint16_t port_dest;
   uint16_t length;
   uint16_t checksum;
} udp_ht;

//=========================== variables =======================================

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

void openudp_init(OpenMote* self);
owerror_t openudp_send(OpenMote* self, OpenQueueEntry_t* msg);
void openudp_sendDone(OpenMote* self, OpenQueueEntry_t* msg, owerror_t error);
void openudp_receive(OpenMote* self, OpenQueueEntry_t* msg);
bool openudp_debugPrint(OpenMote* self);

/**
\}
\}
*/

#endif
