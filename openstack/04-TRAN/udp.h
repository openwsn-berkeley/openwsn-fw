#ifndef OPENWSN_UDP_H
#define OPENWSN_UDP_H

/**
\addtogroup Transport
\{
\addtogroup Udp
\{
*/

#include "config.h"
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
   uint16_t length; // this should not be here. See RFC6282 section 4.3.3.
   uint16_t checksum;
} udp_ht;


typedef void (*udp_callbackReceive_cbt)(OpenQueueEntry_t* msg);
typedef void (*udp_callbackSendDone_cbt)(OpenQueueEntry_t* msg, owerror_t error);

typedef struct udp_resource_desc_t udp_resource_desc_t;

struct udp_resource_desc_t {
   uint16_t                 port;             ///< UDP port that is associated with the resource
   udp_callbackReceive_cbt  callbackReceive;  ///< receive callback,
                                              ///< if NULL, all message received for port will be discarded
   udp_callbackSendDone_cbt callbackSendDone; ///< send completion callback,
                                              ///< if NULL, the associated message will be release without notification
   udp_resource_desc_t*     next;
};

//=========================== variables =======================================

typedef struct {
   udp_resource_desc_t* resources;
} openudp_vars_t;

//=========================== prototypes ======================================

void    openudp_init(void);
void    openudp_register(udp_resource_desc_t* desc);
owerror_t openudp_send(OpenQueueEntry_t* msg);
void    openudp_sendDone(OpenQueueEntry_t* msg, owerror_t error);
void    openudp_receive(OpenQueueEntry_t* msg);
bool    openudp_debugPrint(void);

/**
\}
\}
*/

#endif /* OPENWSN_UDP_H */
